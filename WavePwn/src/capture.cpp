#include "capture.h"
#include "pwnagotchi.h"
#include "ui.h"

#include <Arduino.h>
#include <esp_wifi_types.h>
#include <time.h>
#include <set>
#include <map>

extern Pwnagotchi pwn;

// -----------------------------------------------------------------------------
// Estado global da captura
// -----------------------------------------------------------------------------

File pcap_file;
String current_pcap_path = "";
uint64_t pcap_size = 0;
const uint64_t MAX_PCAP_SIZE = 150ULL * 1024 * 1024; // 150 MB por arquivo

// Deduplicação em RAM (AP+STA)
static std::set<String> seen_handshakes;

// Mapa BSSID -> SSID (para enriquecer logs / futuros exports 22000)
static std::map<String, String> bssid_to_ssid;

// -----------------------------------------------------------------------------
// Helpers internos
// -----------------------------------------------------------------------------

static void capture_write_global_header();
static bool locate_eapol(const uint8_t* frame,
                         uint16_t len,
                         uint16_t& eapol_offset,
                         uint16_t& eapol_len);
static bool is_eapol_frame(const uint8_t* frame, uint16_t len);

static void parse_pmkid(const uint8_t* frame, uint16_t len, uint8_t channel);
static void parse_handshake(const uint8_t* frame, uint16_t len, uint8_t channel);

static String mac_to_string(const uint8_t* mac);
static String mac_to_string_nosep(const uint8_t* mac);
static String bytes_to_hex(const uint8_t* data, size_t len);

// -----------------------------------------------------------------------------
// PCAP
// -----------------------------------------------------------------------------

static void capture_write_global_header() {
    // Cabeçalho PCAP clássico little-endian, linktype = IEEE802_11 (105)
    uint8_t header[24] = {
        0xd4, 0xc3, 0xb2, 0xa1, // magic number (little endian)
        0x02, 0x00,             // version major
        0x04, 0x00,             // version minor
        0x00, 0x00, 0x00, 0x00, // thiszone
        0x00, 0x00, 0x00, 0x00, // sigfigs
        0xff, 0xff, 0x00, 0x00, // snaplen
        0x69, 0x00, 0x00, 0x00  // network = 105 (LINKTYPE_IEEE802_11)
    };

    pcap_file.write(header, sizeof(header));
    pcap_size = sizeof(header);
}

void capture_rotate_files() {
    if (pcap_file) {
        pcap_file.flush();
        pcap_file.close();
    }

    time_t t = time(nullptr);
    struct tm* tm_info = localtime(&t);

    char path[96];
    if (tm_info) {
        snprintf(path,
                 sizeof(path),
                 "/sd/wavepwn/handshakes/%04d%02d%02d_%02d%02d%02d.pcap",
                 tm_info->tm_year + 1900,
                 tm_info->tm_mon + 1,
                 tm_info->tm_mday,
                 tm_info->tm_hour,
                 tm_info->tm_min,
                 tm_info->tm_sec);
    } else {
        // Fallback se RTC não estiver configurado
        unsigned long ms = millis();
        snprintf(path,
                 sizeof(path),
                 "/sd/wavepwn/handshakes/%010lu.pcap",
                 ms / 1000UL);
    }

    current_pcap_path = path;

    pcap_file = SD.open(path, FILE_WRITE);
    if (!pcap_file) {
        Serial.printf("[CAPTURE] ERRO ao abrir PCAP '%s' para escrita\n", path);
        return;
    }

    capture_write_global_header();
    Serial.printf("[CAPTURE] Novo PCAP: %s\n", path);
}

void capture_init() {
    // Assume que o SD já foi inicializado em Pwnagotchi::initSD()
    capture_rotate_files();
}

String capture_get_current_pcap() {
    return current_pcap_path;
}

void capture_write_pcap(const uint8_t* data, uint32_t len, uint32_t channel) {
    if (!pcap_file) return;

    // Timestamp em microssegundos baseado no millis()
    uint32_t ms = millis();
    uint32_t ts_sec = ms / 1000;
    uint32_t ts_usec = (ms % 1000) * 1000;

    // Cabeçalho por pacote
    uint8_t header[16];
    memcpy(header + 0, &ts_sec, 4);
    memcpy(header + 4, &ts_usec, 4);
    memcpy(header + 8, &len, 4);
    memcpy(header + 12, &len, 4);

    pcap_file.write(header, sizeof(header));
    pcap_file.write(data, len);
    pcap_size += sizeof(header) + len;

    if (pcap_size > MAX_PCAP_SIZE) {
        capture_rotate_files();
    }
}

// -----------------------------------------------------------------------------
// Deduplicação
// -----------------------------------------------------------------------------

static String make_handshake_key(const uint8_t* ap, const uint8_t* sta) {
    return mac_to_string_nosep(ap) + String("_") + mac_to_string_nosep(sta);
}

bool capture_is_duplicate(const uint8_t* ap, const uint8_t* sta) {
    if (!ap || !sta) return false;
    String key = make_handshake_key(ap, sta);
    return seen_handshakes.find(key) != seen_handshakes.end();
}

// -----------------------------------------------------------------------------
// Salvar estruturas parseadas
// -----------------------------------------------------------------------------

bool capture_save_pmkid(const PMKID* pmkid) {
    if (!pmkid) return false;
    if (!pcap_file) {
        // Não exige PCAP aberto, mas assume SD OK
    }

    String ap_hex = mac_to_string_nosep(pmkid->ap);
    String sta_hex = mac_to_string_nosep(pmkid->sta);
    String pmkid_hex = bytes_to_hex(pmkid->pmkid, sizeof(pmkid->pmkid));
    String ssid_str = String(pmkid->ssid);

    // Formato hashcat -m 16800: WPA*01*PMKID*MAC_AP*MAC_STA*SSID_HEX
    String ssid_hex = bytes_to_hex((const uint8_t*)pmkid->ssid, strlen(pmkid->ssid));
    String hash_line = "WPA*01*";
    hash_line += pmkid_hex;
    hash_line += "*";
    hash_line += ap_hex;
    hash_line += "*";
    hash_line += sta_hex;
    hash_line += "*";
    hash_line += ssid_hex;

    String file_path = "/sd/wavepwn/pmkid/";
    file_path += pmkid_hex;
    file_path += "_";
    file_path += ap_hex;
    file_path += "_";
    file_path += sta_hex;
    file_path += ".16800";

    File f = SD.open(file_path, FILE_WRITE);
    if (!f) {
        Serial.printf("[CAPTURE] Falha ao abrir arquivo PMKID: %s\n", file_path.c_str());
        return false;
    }
    f.println(hash_line);
    f.close();

    // Atualiza mapa BSSID -> SSID
    bssid_to_ssid[ap_hex] = ssid_str;

    // Log JSON simples
    File log = SD.open("/sd/wavepwn/logs/pmkid.jsonl", FILE_APPEND);
    if (log) {
        log.printf(
            "{\"type\":\"pmkid\",\"ap\":\"%s\",\"sta\":\"%s\",\"ssid\":\"%s\","
            "\"file\":\"%s\",\"timestamp\":%llu}\n",
            ap_hex.c_str(),
            sta_hex.c_str(),
            ssid_str.c_str(),
            file_path.c_str(),
            (unsigned long long)pmkid->timestamp);
        log.close();
    }

    Serial.printf("[CAPTURE] PMKID salvo (%s -> %s) SSID=\"%s\"\n",
                  ap_hex.c_str(),
                  sta_hex.c_str(),
                  ssid_str.c_str());
    return true;
}

bool capture_save_handshake(const Handshake* hs) {
    if (!hs) return false;

    String ap_hex = mac_to_string_nosep(hs->ap);
    String sta_hex = mac_to_string_nosep(hs->sta);

    char ts_buf[32];
    snprintf(ts_buf, sizeof(ts_buf), "%llu",
             (unsigned long long)hs->timestamp);

    // Salvamos a estrutura binária para futura conversão (22000/2500) em PC.
    String file_path = "/sd/wavepwn/session/hs_";
    file_path += ap_hex;
    file_path += "_";
    file_path += sta_hex;
    file_path += "_";
    file_path += ts_buf;
    file_path += ".bin";

    File f = SD.open(file_path, FILE_WRITE);
    if (!f) {
        Serial.printf("[CAPTURE] Falha ao abrir arquivo Handshake: %s\n", file_path.c_str());
        return false;
    }
    f.write((const uint8_t*)hs, sizeof(Handshake));
    f.close();

    // Log JSONL com metadados
    File log = SD.open("/sd/wavepwn/logs/handshakes.jsonl", FILE_APPEND);
    if (log) {
        String ssid = "";
        auto it = bssid_to_ssid.find(ap_hex);
        if (it != bssid_to_ssid.end()) {
            ssid = it->second;
        }
        log.printf(
            "{\"type\":\"handshake\",\"ap\":\"%s\",\"sta\":\"%s\",\"ssid\":\"%s\","
            "\"file\":\"%s\",\"keyver\":%u,\"msg\":%u,\"timestamp\":%llu}\n",
            ap_hex.c_str(),
            sta_hex.c_str(),
            ssid.c_str(),
            file_path.c_str(),
            hs->key_version,
            hs->message_num,
            (unsigned long long)hs->timestamp);
        log.close();
    }

    Serial.printf("[CAPTURE] Handshake salvo (%s <-> %s)\n",
                  ap_hex.c_str(),
                  sta_hex.c_str());
    return true;
}

// -----------------------------------------------------------------------------
// Parsing de frames
// -----------------------------------------------------------------------------

static bool locate_eapol(const uint8_t* frame,
                         uint16_t len,
                         uint16_t& eapol_offset,
                         uint16_t& eapol_len) {
    if (len < 40) return false;

    uint8_t fc = frame[0];
    uint8_t type = (fc >> 2) & 0x3; // bits Type
    if (type != 2) {
        // Apenas Data frames carregam EAPOL
        return false;
    }

    uint16_t hdr_len = 24; // 3 endereços
    uint8_t flags = frame[1];
    bool to_ds = flags & 0x01;
    bool from_ds = flags & 0x02;

    if (to_ds && from_ds) {
        // 4-addr frame
        hdr_len += 6;
    }

    // QOS? (subtype bit 7 ligado em QoS Data)
    bool qos = (fc & 0x80) != 0;
    if (qos) {
        hdr_len += 2;
    }

    if (len < hdr_len + 8) return false;

    const uint8_t* llc = frame + hdr_len;
    // LLC: AA AA 03 00 00 00 88 8E
    if (llc[0] != 0xAA || llc[1] != 0xAA) return false;
    if (llc[6] != 0x88 || llc[7] != 0x8E) return false;

    eapol_offset = hdr_len + 8;
    if (len <= eapol_offset) return false;
    eapol_len = len - eapol_offset;
    return true;
}

static bool is_eapol_frame(const uint8_t* frame, uint16_t len) {
    uint16_t off = 0, l = 0;
    return locate_eapol(frame, len, off, l);
}

// Parser de PMKID a partir de IEs RSN (Beacon / Probe Response)
static void parse_pmkid(const uint8_t* frame, uint16_t len, uint8_t channel) {
    (void)channel;

    if (len < 36) return;

    uint8_t fc = frame[0];
    uint8_t type = (fc >> 2) & 0x3;
    uint8_t subtype = (fc >> 4) & 0xF;

    if (type != 0) return; // só management
    if (!(subtype == 8 || subtype == 5)) {
        // Beacon (1000) ou Probe Response (0101)
        return;
    }

    const uint8_t* addr1 = frame + 4;
    const uint8_t* addr2 = frame + 10;
    const uint8_t* addr3 = frame + 16;

    const uint8_t* body = frame + 24;
    uint16_t body_len = len - 24;

    if (body_len < 12) return;
    const uint8_t* ie = body + 12;
    uint16_t ie_len = body_len - 12;

    const uint8_t* rsn = nullptr;
    uint8_t rsn_len = 0;
    const uint8_t* ssid = nullptr;
    uint8_t ssid_len = 0;

    // Navega pelos IEs
    while (ie_len >= 2) {
        uint8_t id = ie[0];
        uint8_t elen = ie[1];
        if (elen + 2 > ie_len) break;

        if (id == 0) { // SSID
            ssid = ie + 2;
            ssid_len = elen > 32 ? 32 : elen;
        } else if (id == 48) { // RSN
            rsn = ie + 2;
            rsn_len = elen;
        }

        ie += 2 + elen;
        ie_len -= 2 + elen;
    }

    if (!rsn || rsn_len < 20) return;

    const uint8_t* ptr = rsn;
    uint16_t rem = rsn_len;

    if (rem < 2) return;
    uint16_t version = ptr[0] | (ptr[1] << 8);
    (void)version;
    ptr += 2;
    rem -= 2;

    if (rem < 4) return; // group cipher
    ptr += 4;
    rem -= 4;

    if (rem < 2) return;
    uint16_t pairwise_count = ptr[0] | (ptr[1] << 8);
    ptr += 2;
    rem -= 2;

    if (rem < 4 * pairwise_count) return;
    ptr += 4 * pairwise_count;
    rem -= 4 * pairwise_count;

    if (rem < 2) return;
    uint16_t akm_count = ptr[0] | (ptr[1] << 8);
    ptr += 2;
    rem -= 2;

    if (rem < 4 * akm_count) return;
    ptr += 4 * akm_count;
    rem -= 4 * akm_count;

    // RSN Capabilities (opcional, mas geralmente presente)
    if (rem < 2) return;
    ptr += 2;
    rem -= 2;

    if (rem < 2) return;
    uint16_t pmkid_count = ptr[0] | (ptr[1] << 8);
    ptr += 2;
    rem -= 2;

    if (pmkid_count == 0 || rem < 16) return;

    PMKID pm = {};
    memcpy(pm.pmkid, ptr, 16);
    pm.timestamp = millis();

    // BSSID é addr3
    memcpy(pm.ap, addr3, 6);
    // Para Beacons/Probe Resp não há STA específica, usamos broadcast
    memset(pm.sta, 0xFF, 6);

    if (ssid && ssid_len > 0) {
        memcpy(pm.ssid, ssid, ssid_len);
        pm.ssid[ssid_len] = '\0';
    } else {
        strncpy(pm.ssid, "<hidden>", sizeof(pm.ssid));
        pm.ssid[sizeof(pm.ssid) - 1] = '\0';
    }

    if (capture_save_pmkid(&pm)) {
        pwn.pmkids++;
        ui_set_mood(MOOD_PMKID);
        ui_celebrate_pmkid();
    }
}

// Parser simplificado do 4-way handshake WPA2
static void parse_handshake(const uint8_t* frame, uint16_t len, uint8_t channel) {
    (void)channel;

    uint16_t eapol_offset = 0;
    uint16_t eapol_len = 0;

    if (!locate_eapol(frame, len, eapol_offset, eapol_len)) {
        return;
    }

    if (eapol_len < 4 + 95) {
        // Cabeçalho EAPOL (4) + estrutura mínima de EAPOL-Key
        return;
    }

    const uint8_t* eapol = frame + eapol_offset;

    uint8_t eapol_version = eapol[0];
    uint8_t eapol_type = eapol[1];
    uint16_t eapol_body_len = (eapol[2] << 8) | eapol[3];

    (void)eapol_version;
    if (eapol_type != 3) {
        // Só EAPOL-Key nos interessa
        return;
    }

    if (eapol_body_len + 4 > eapol_len) {
        eapol_body_len = eapol_len - 4;
    }

    const uint8_t* key_frame = eapol + 4;
    if (eapol_body_len < 95) return;

    uint8_t descriptor_type = key_frame[0];
    (void)descriptor_type;

    uint16_t key_info = (key_frame[1] << 8) | key_frame[2];
    uint16_t key_len = (key_frame[3] << 8) | key_frame[4];
    (void)key_len;

    const uint8_t* replay_counter = key_frame + 5;
    const uint8_t* key_nonce = key_frame + 13;
    const uint8_t* key_mic = key_frame + 77;
    uint16_t key_data_len = (key_frame[93] << 8) | key_frame[94];
    (void)replay_counter;
    (void)key_data_len;

    // Bits de controle
    const uint16_t WPA_KEY_INFO_INSTALL = 0x0040;
    const uint16_t WPA_KEY_INFO_ACK     = 0x0080;
    const uint16_t WPA_KEY_INFO_MIC     = 0x0100;
    const uint16_t WPA_KEY_INFO_SECURE  = 0x0200;
    const uint16_t WPA_KEY_INFO_REQUEST = 0x0800;

    bool mic_set   = (key_info & WPA_KEY_INFO_MIC) != 0;
    bool ack_set   = (key_info & WPA_KEY_INFO_ACK) != 0;
    bool install   = (key_info & WPA_KEY_INFO_INSTALL) != 0;
    bool secure    = (key_info & WPA_KEY_INFO_SECURE) != 0;
    bool request   = (key_info & WPA_KEY_INFO_REQUEST) != 0;

    uint8_t message_num = 0;
    if (!mic_set && ack_set && !install && !secure) {
        message_num = 1;
    } else if (mic_set && !ack_set && !install && !secure && !request) {
        message_num = 2;
    } else if (mic_set && ack_set && install && secure) {
        message_num = 3;
    } else if (mic_set && !ack_set && !install && secure) {
        message_num = 4;
    }

    // Extrair MACs do header 802.11
    const uint8_t* addr1 = frame + 4;
    const uint8_t* addr2 = frame + 10;
    const uint8_t* addr3 = frame + 16;

    uint8_t flags = frame[1];
    bool to_ds = flags & 0x01;
    bool from_ds = flags & 0x02;

    const uint8_t* bssid = nullptr;
    if (!to_ds && !from_ds) {
        bssid = addr3;
    } else if (!to_ds && from_ds) {
        bssid = addr2;
    } else if (to_ds && !from_ds) {
        bssid = addr1;
    } else {
        // WDS, escolhemos addr1 por convenção
        bssid = addr1;
    }

    const uint8_t* ap_mac = nullptr;
    const uint8_t* sta_mac = nullptr;

    if (memcmp(addr1, bssid, 6) == 0) {
        ap_mac = addr1;
        sta_mac = addr2;
    } else if (memcmp(addr2, bssid, 6) == 0) {
        ap_mac = addr2;
        sta_mac = addr1;
    } else {
        ap_mac = bssid;
        sta_mac = addr2;
    }

    if (!ap_mac || !sta_mac) return;

    // Deduplicação: um handshake por par AP/STA
    if (capture_is_duplicate(ap_mac, sta_mac)) {
        return;
    }

    Handshake hs = {};
    memcpy(hs.ap, ap_mac, 6);
    memcpy(hs.sta, sta_mac, 6);

    bool from_ap = (memcmp(addr2, ap_mac, 6) == 0);
    if (from_ap) {
        memcpy(hs.anonce, key_nonce, 32);
    } else {
        memcpy(hs.snonce, key_nonce, 32);
    }

    memcpy(hs.mic, key_mic, 16);

    uint16_t copy_len = eapol_len;
    if (copy_len > sizeof(hs.eapol)) copy_len = sizeof(hs.eapol);
    memcpy(hs.eapol, eapol, copy_len);
    hs.eapol_size = copy_len;

    // Zera o MIC no buffer EAPOL para facilitar conversões futuras (-m 22000)
    size_t mic_offset = (size_t)(key_mic - eapol);
    if (mic_offset + 16 <= hs.eapol_size) {
        memset(hs.eapol + mic_offset, 0, 16);
    }

    hs.key_version = key_info & 0x7;
    hs.message_num = message_num;
    hs.timestamp = millis();

    String key = make_handshake_key(hs.ap, hs.sta);
    if (capture_save_handshake(&hs)) {
        seen_handshakes.insert(key);
        pwn.handshakes++;
        ui_set_mood(MOOD_HANDSHAKE);
        ui_celebrate_handshake();
    }
}

// -----------------------------------------------------------------------------
// Callback principal chamado pelo driver Wi-Fi em modo promíscuo
// -----------------------------------------------------------------------------

void IRAM_ATTR capture_packet_handler(uint8_t* buf, uint16_t len, uint8_t channel) {
    (void)len;

    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    const uint8_t* frame = pkt->payload;
    uint16_t frame_len = pkt->rx_ctrl.sig_len;
    uint8_t ch = pkt->rx_ctrl.channel;
    (void)channel; // usamos o do rx_ctrl para garantir consistência

    // Escreve TODOS os pacotes no PCAP
    capture_write_pcap(frame, frame_len, ch);

    uint8_t fc = frame[0];

    // Handshake WPA/WPA2 (EAPOL dentro de Data frame)
    if (is_eapol_frame(frame, frame_len)) {
        parse_handshake(frame, frame_len, ch);
    }

    // PMKID a partir de beacons / probe responses
    uint8_t type = (fc >> 2) & 0x3;
    uint8_t subtype = (fc >> 4) & 0xF;
    if (type == 0 && (subtype == 8 || subtype == 5)) {
        parse_pmkid(frame, frame_len, ch);
    }
}

// -----------------------------------------------------------------------------
// Helpers de formatação
// -----------------------------------------------------------------------------

static String mac_to_string(const uint8_t* mac) {
    char buf[18];
    snprintf(buf,
             sizeof(buf),
             "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buf);
}

static String mac_to_string_nosep(const uint8_t* mac) {
    char buf[13];
    snprintf(buf,
             sizeof(buf),
             "%02X%02X%02X%02X%02X%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buf);
}

static String bytes_to_hex(const uint8_t* data, size_t len) {
    static const char* hex = "0123456789abcdef";
    String out;
    out.reserve(len * 2);
    for (size_t i = 0; i < len; ++i) {
        uint8_t b = data[i];
        out += hex[b >> 4];
        out += hex[b & 0x0F];
    }
    return out;
}