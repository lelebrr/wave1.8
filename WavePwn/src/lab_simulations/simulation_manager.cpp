/*
  simulation_manager.cpp - Implementação das simulações acadêmicas

  AVISO IMPORTANTE:
  - Nenhuma função aqui envia frames 802.11, comandos Bluetooth reais ou NFC.
  - Tudo é tratado como "cenário sintético" para fins didáticos.
  - O modo laboratório só é considerado ativo quando:
    * o arquivo /sd/.enable_lab_attacks existe no microSD; e
    * o PIN de laboratório da sessão atual foi desbloqueado.
*/

#include <Arduino.h>
#include "simulation_manager.h"
#include "ui.h"

// Estado de desbloqueio da sessão atual (PIN válido já fornecido).
static bool s_lab_unlocked = false;

static void lab_ensure_log_dir() {
  if (!SD.exists("/sd/lab_logs")) {
    SD.mkdir("/sd/lab_logs");
  }
}

static void lab_log_event(const char* tag, const char* details) {
  lab_ensure_log_dir();

  char path[64];
  snprintf(path, sizeof(path), "/sd/lab_logs/%s.log", tag ? tag : "lab_sim");

  File f = SD.open(path, FILE_APPEND);
  if (!f) {
    Serial.printf("[LAB][SIM] Falha ao abrir log %s\n", path);
    return;
  }

  unsigned long t = millis() / 1000;
  f.printf("%lu;%s;%s\n",
           (unsigned long)t,
           tag ? tag : "event",
           details ? details : "");
  f.close();
}

static lv_obj_t* s_sim_banner = nullptr;

static void lab_show_banner(const char* text) {
  lv_obj_t* scr = lv_scr_act();
  if (!scr) return;

  if (!s_sim_banner) {
    s_sim_banner = lv_label_create(scr);
    lv_obj_set_width(s_sim_banner, lv_pct(100));
    lv_obj_set_style_text_align(s_sim_banner, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(s_sim_banner, lv_color_hex(0xFF0000), 0);
    lv_obj_align(s_sim_banner, LV_ALIGN_TOP_MID, 0, 8);
  }

  if (text) {
    lv_label_set_text(s_sim_banner, text);
  } else {
    lv_label_set_text(s_sim_banner, "SIMULACAO LABORATORIAL");
  }
}

static void lab_warn_mode_off(const char* feature) {
  Serial.printf(
      "[LAB][SIM] Modo laboratorio DESATIVADO - simulacao '%s' bloqueada. "
      "Crie /sd/.enable_lab_attacks e desbloqueie o PIN de 6 digitos "
      "apos ler ATTACK_SIMULATION.md\n",
      feature ? feature : "?");
  ui_set_mood(MOOD_SAD);
  lab_show_banner("LAB MODE OFF - LEIA ATTACK_SIMULATION.MD");
  lab_log_event("lab_mode_off", feature ? feature : "?");
}

// ---------------------------------------------------------------------------
// API de modo laboratório
// ---------------------------------------------------------------------------

bool SimulationManager::is_lab_mode_enabled() {
  if (!SD.exists("/sd/.enable_lab_attacks")) {
    return false;
  }
  return s_lab_unlocked;
}

void SimulationManager::set_lab_unlocked(bool unlocked) {
  s_lab_unlocked = unlocked;
  lab_log_event(unlocked ? "lab_unlocked" : "lab_locked",
                unlocked ? "sessao_atual_pin_ok" : "sessao_atual_pin_reset");
}

bool SimulationManager::is_lab_unlocked() {
  return s_lab_unlocked;
}

// ---------------------------------------------------------------------------
// Implementações públicas de SimulationManager (simulações acadêmicas)
// ---------------------------------------------------------------------------

void SimulationManager::deauth_burst_sim(uint8_t channel, int packets) {
  if (!is_lab_mode_enabled()) {
    lab_warn_mode_off("deauth_burst_sim");
    return;
  }

  Serial.printf(
      "[LAB][SIM] Deauth burst (SIMULACAO) no canal %u, pacotes=%d "
      "(nenhuma transmissao real)\n",
      channel,
      packets);

  ui_set_mood(MOOD_ANGRY);
  lab_show_banner("SIMULACAO DE DEAUTH (SEM TRANSMISSAO)");

  char details[96];
  snprintf(details, sizeof(details), "channel=%u;packets=%d", channel, packets);
  lab_log_event("deauth_burst_sim", details);
}

void SimulationManager::evil_twin_sim(const char* ssid, uint8_t channel) {
  if (!is_lab_mode_enabled()) {
    lab_warn_mode_off("evil_twin_sim");
    return;
  }

  const char* safe_ssid = ssid ? ssid : "(null)";

  Serial.printf(
      "[LAB][SIM] Evil Twin (SIMULACAO) SSID='%s' canal=%u "
      "(nenhum AP falso real criado)\n",
      safe_ssid,
      channel);

  ui_set_mood(MOOD_SNEAKY);
  lab_show_banner("SIMULACAO EVIL TWIN (SEM AP REAL)");

  char details[128];
  snprintf(details, sizeof(details), "ssid=%s;channel=%u", safe_ssid, channel);
  lab_log_event("evil_twin_sim", details);
}

void SimulationManager::beacon_spam_sim(int count) {
  if (!is_lab_mode_enabled()) {
    lab_warn_mode_off("beacon_spam_sim");
    return;
  }

  Serial.printf(
      "[LAB][SIM] Beacon spam (SIMULACAO) count=%d "
      "(nenhum beacon falso enviado)\n",
      count);

  ui_set_mood(MOOD_SCANNING);
  lab_show_banner("SIMULACAO BEACON SPAM (SEM FRAMES)");

  char details[64];
  snprintf(details, sizeof(details), "count=%d", count);
  lab_log_event("beacon_spam_sim", details);
}

void SimulationManager::rogue_ap_sim() {
  if (!is_lab_mode_enabled()) {
    lab_warn_mode_off("rogue_ap_sim");
    return;
  }

  Serial.println(
      "[LAB][SIM] Rogue AP (SIMULACAO) - apenas logs e UI, nenhum AP malicioso real");

  ui_set_mood(MOOD_WIFI_OFF);
  lab_show_banner("SIMULACAO ROGUE AP (SEM AP REAL)");

  lab_log_event("rogue_ap_sim", "simulacao_rogue_ap");
}

void SimulationManager::bluetooth_spam_sim(const char* profile_name) {
  if (!is_lab_mode_enabled()) {
    lab_warn_mode_off("bluetooth_spam_sim");
    return;
  }

  const char* safe_profile = profile_name ? profile_name : "(null)";

  Serial.printf(
      "[LAB][SIM] Bluetooth spam (SIMULACAO) perfil='%s' "
      "(nenhuma conexao BLE real criada)\n",
      safe_profile);

  ui_set_mood(MOOD_EXCITED);
  lab_show_banner("SIMULACAO BLUETOOTH (SEM SPAM REAL)");

  char details[128];
  snprintf(details, sizeof(details), "profile=%s", safe_profile);
  lab_log_event("bluetooth_spam_sim", details);
}

void SimulationManager::pmkid_flood_sim(const char* ap_label, int frames) {
  if (!is_lab_mode_enabled()) {
    lab_warn_mode_off("pmkid_flood_sim");
    return;
  }

  const char* safe_label = ap_label ? ap_label : "(null)";

  Serial.printf(
      "[LAB][SIM] PMKID flood (SIMULACAO) alvo='%s' frames=%d "
      "(nenhum frame PMKID real enviado)\n",
      safe_label,
      frames);

  ui_set_mood(MOOD_PMKID);
  lab_show_banner("SIMULACAO PMKID (SEM FRAMES REAIS)");

  char details[160];
  snprintf(details, sizeof(details), "target=%s;frames=%d", safe_label, frames);
  lab_log_event("pmkid_flood_sim", details);
}

void SimulationManager::bluetooth_jammer_sim(int duration_seconds) {
  if (!is_lab_mode_enabled()) {
    lab_warn_mode_off("bluetooth_jammer_sim");
    return;
  }

  if (duration_seconds < 0) {
    duration_seconds = 0;
  }

  Serial.printf(
      "[LAB][SIM] Bluetooth jammer (SIMULACAO) duracao=%ds "
      "(nenhuma interferencia real gerada)\n",
      duration_seconds);

  ui_set_mood(MOOD_WIFI_OFF);
  lab_show_banner("SIMULACAO BT JAMMER (SEM INTERFERENCIA)");

  char details[64];
  snprintf(details, sizeof(details), "duration=%d", duration_seconds);
  lab_log_event("bluetooth_jammer_sim", details);
}

void SimulationManager::bluetooth_inquiry_flood_sim(int duration_seconds) {
  if (!is_lab_mode_enabled()) {
    lab_warn_mode_off("bluetooth_inquiry_flood_sim");
    return;
  }

  if (duration_seconds < 0) {
    duration_seconds = 0;
  }

  Serial.printf(
      "[LAB][SIM] Bluetooth inquiry flood (SIMULACAO) duracao=%ds "
      "(nenhuma inquiry real enviada)\n",
      duration_seconds);

  ui_set_mood(MOOD_SCANNING);
  lab_show_banner("SIMULACAO INQUIRY (SEM SCAN REAL)");

  char details[64];
  snprintf(details, sizeof(details), "duration=%d", duration_seconds);
  lab_log_event("bluetooth_inquiry_flood_sim", details);
}

// ---------------------------------------------------------------------------
// Simulações Wi-Fi avançadas (sempre acadêmicas)
// ---------------------------------------------------------------------------

void SimulationManager::wps_attack_sim(const uint8_t* ap_mac, uint8_t channel) {
  if (!is_lab_mode_enabled()) {
    lab_warn_mode_off("wps_attack_sim");
    return;
  }

  // Não há brute-force real aqui. Apenas registramos tentativas sintéticas
  // para visualização em laboratório.
  Serial.printf(
      "[LAB][SIM] WPS PIN brute-force (SIMULACAO) canal=%u "
      "(nenhum frame WPS real enviado)\n",
      channel);

  ui_set_mood(MOOD_ANGRY);
  lab_show_banner("SIMULACAO WPS PIN (SEM FRAMES)");

  // Representa 5 tentativas genéricas de PIN em logs didáticos.
  for (int i = 0; i < 5; ++i) {
    char details[160];
    char mac_suffix[12] = "";
    if (ap_mac) {
      snprintf(mac_suffix,
               sizeof(mac_suffix),
               "%02X:%02X",
               ap_mac[4],
               ap_mac[5]);
    }
    snprintf(details,
             sizeof(details),
             "attempt=%d;channel=%u;target_suffix=%s",
             i + 1,
             channel,
             ap_mac ? mac_suffix : "NA");
    lab_log_event("wps_attack_sim", details);
    delay(50);
  }
}

void SimulationManager::karma_attack_sim(const char* ssid_pattern,
                                         uint8_t channel) {
  if (!is_lab_mode_enabled()) {
    lab_warn_mode_off("karma_attack_sim");
    return;
  }

  const char* safe_pattern = ssid_pattern ? ssid_pattern : "(null)";

  Serial.printf(
      "[LAB][SIM] Karma/MANA (SIMULACAO) pattern='%s' canal=%u "
      "(nenhuma resposta a probe real enviada)\n",
      safe_pattern,
      channel);

  ui_set_mood(MOOD_SNEAKY);
  lab_show_banner("SIMULACAO KARMA/MANA (SEM FRAMES)");

  char details[192];
  snprintf(details,
           sizeof(details),
           "pattern=%s;channel=%u",
           safe_pattern,
           channel);
  lab_log_event("karma_attack_sim", details);
}

void SimulationManager::handshake_downgrade_sim(const char* ap_label,
                                                bool from_wpa3_to_wpa2) {
  if (!is_lab_mode_enabled()) {
    lab_warn_mode_off("handshake_downgrade_sim");
    return;
  }

  const char* safe_label = ap_label ? ap_label : "(null)";

  Serial.printf(
      "[LAB][SIM] Handshake downgrade (SIMULACAO) alvo='%s' direcao=%s "
      "(nenhum downgrade real realizado)\n",
      safe_label,
      from_wpa3_to_wpa2 ? "WPA3->WPA2" : "GENERICA");

  ui_set_mood(MOOD_PMKID);
  lab_show_banner("SIMULACAO DOWNGRADE WPA (SEM MUDAR CIFRA)");

  char details[192];
  snprintf(details,
           sizeof(details),
           "target=%s;direction=%s",
           safe_label,
           from_wpa3_to_wpa2 ? "wpa3_to_wpa2" : "generic");
  lab_log_event("handshake_downgrade_sim", details);
}

// ---------------------------------------------------------------------------
// Simulações Bluetooth avançadas
// ---------------------------------------------------------------------------

void SimulationManager::bluetooth_hid_injection_sim(const char* payload) {
  if (!is_lab_mode_enabled()) {
    lab_warn_mode_off("bluetooth_hid_injection_sim");
    return;
  }

  const char* safe_payload = payload ? payload : "(null)";

  Serial.printf(
      "[LAB][SIM] Bluetooth HID injection (SIMULACAO) payload=\"%s\" "
      "(nenhum teclado ou comando real enviado)\n",
      safe_payload);

  ui_set_mood(MOOD_COOL);
  lab_show_banner("SIMULACAO HID (SEM TECLADO REAL)");

  char details[192];
  snprintf(details, sizeof(details), "payload=%s", safe_payload);
  lab_log_event("bluetooth_hid_injection_sim", details);
}