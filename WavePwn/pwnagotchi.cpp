#include <Arduino.h>
#include <WiFi.h>
#include <SD.h>
#include <esp_wifi.h>
#include <esp_sleep.h>

#include "pwnagotchi.h"
#include "ui.h"
#include "config.h"
#include "storage.h"
#include "wifi_sniffer.h"
#include "sensors.h"
#include "audio.h"
#include "capture.h"
#include "ai/neura9_inference.h"
#include "src/webserver.h"
#include "ble_grid/pwn_grid.h"

// -----------------------------------------------------------------------------
// Implementação mínima do wrapper LGFX (por enquanto, apenas loga no Serial).
// Nas próximas etapas vamos conectar isso ao driver real do display QSPI.
// -----------------------------------------------------------------------------
void LGFX::begin() {
    // Placeholder: aqui entra a inicialização real do display.
    Serial.println("[LGFX] Inicializando display (stub inicial)...");
}

void LGFX::fillScreen(uint32_t color) {
    (void)color;
    // No stub não fazemos nada; o objetivo aqui é garantir compilação estável.
}

void LGFX::setTextColor(uint32_t color) {
    (void)color;
}

void LGFX::setTextSize(uint8_t size) {
    (void)size;
}

void LGFX::setCursor(int16_t x, int16_t y) {
    (void)x;
    (void)y;
}

void LGFX::println(const char *text) {
    // Enquanto o driver real não estiver plugado, usamos apenas o Serial.
    Serial.println(text);
}

void LGFX::setBrightness(uint8_t value) {
    // Stub: apenas loga. No futuro será mapeado para o PWM do backlight.
    Serial.printf("[LGFX] setBrightness(%u)\\n", value);
}

// -----------------------------------------------------------------------------
// Pwnagotchi - ciclo de vida principal
// -----------------------------------------------------------------------------
void Pwnagotchi::begin() {
    initDisplay();
    showBootAnimation();

    initSD();
    capture_init();

    initSensors();
    initWiFiMonitor();

    // Callback promíscuo -> motor de captura de handshakes/PMKID
    esp_wifi_set_promiscuous_rx_cb([](void* buf, wifi_promiscuous_pkt_type_t type) {
        (void)type;
        wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
        capture_packet_handler((uint8_t*)pkt, pkt->rx_ctrl.sig_len, pkt->rx_ctrl.channel);
    });

    lv_init();
    ui_init();

    // Inicializa IA defensiva local (NEURA9)
    if (!neura9.begin()) {
        Serial.println("[NEURA9] Falha ao inicializar IA defensiva (modo stub)");
    }

    // BLE PwnGrid cooperativo
    pwnGrid.begin();

    // Webserver + WebSocket + OTA seguro
    webserver_start();

    Serial.println("WAVE PWN PRONTO PARA DOMINAR");
}

void Pwnagotchi::update() {
    // Uptimes simples em segundos (aprox.)
    uptime = millis() / 1000;

    // Placeholders de ambiente (até integrar sensores reais reais).
    // Estes valores podem ser sobrescritos por sensores.cpp quando existirem.
    if (battery_percent <= 0.0f || battery_percent > 100.0f) {
        battery_percent = 100.0f;
    }

    // Modo ZUMBI: se a bateria entrar em estado crítico, reduz ao mínimo
    if (!in_zombie_mode && battery_percent <= 1.0f) {
        enterZombieMode();
        return;
    }

    ui_update_stats(
        aps_seen,
        handshakes,
        pmkids,
        deauths,
        current_channel,
        battery_percent,
        is_moving
    );

    // NEURA9 avalia o ambiente periodicamente (HUD + PwnGrid)
    static uint32_t last_ai = 0;
    uint32_t now = millis();
    if (now - last_ai > 800) {
        uint8_t cls = neura9.predict();
        float conf  = neura9.get_confidence();
        threat_level = cls;
        threat_confidence = conf;

        Serial.printf("[NEURA9] classe=%u (%s) conf=%.2f\n",
                      cls,
                      NEURA9_THREAT_LABELS[cls],
                      conf);

        last_ai = now;

        // Compartilha nível de ameaça com a PwnGrid cooperativa
        pwnGrid.share_threat_level(cls);
    }

    // Web dashboard em tempo real
    webserver_send_stats();

    // Deixa o LVGL rodar a UI
    lv_timer_handler();
    delay(5);
}

// -----------------------------------------------------------------------------
// Inicializações privadas
// -----------------------------------------------------------------------------
void Pwnagotchi::initDisplay() {
    lcd.begin();
    // Futuro: integrar LVGL + LovyanGFX aqui (flush_cb, draw buffers, touch, etc.)
}

void Pwnagotchi::initSD() {
    Serial.println("[WavePwn] initSD() - inicializando microSD...");

    if (!SD.begin()) {
        Serial.println("[WavePwn] Falha ao inicializar o microSD");
        return;
    }

    // Estrutura definitiva de pastas
    SD.mkdir("/sd");
    SD.mkdir("/sd/wavepwn");
    SD.mkdir("/sd/wavepwn/handshakes");
    SD.mkdir("/sd/wavepwn/pmkid");
    SD.mkdir("/sd/wavepwn/sae");
    SD.mkdir("/sd/wavepwn/logs");
    SD.mkdir("/sd/wavepwn/session");

    Serial.println("[WavePwn] microSD pronto para captura de handshakes");
}

void Pwnagotchi::initWiFiMonitor() {
    Serial.println("[WavePwn] initWiFiMonitor() - configurando Wi-Fi em modo promíscuo + AP");

    // Modo AP+STA permite sniffar e, ao mesmo tempo, expor o dashboard web.
    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.disconnect(true, true);

    // SoftAP local para o dashboard / OTA (sem acesso à Internet por padrão).
#ifdef PET_NAME
    const char* ap_ssid = PET_NAME;
#else
    const char* ap_ssid = "WavePwn";
#endif
    const char* ap_pass = "wavepwn";

    bool ap_ok = WiFi.softAP(ap_ssid, ap_pass);
    if (ap_ok) {
        IPAddress ip = WiFi.softAPIP();
        Serial.printf("[WavePwn] AP '%s' ativo em %s (senha: %s)\n",
                      ap_ssid,
                      ip.toString().c_str(),
                      ap_pass);
    } else {
        Serial.println("[WavePwn] Falha ao iniciar SoftAP");
    }

    // Filtro: apenas management + data (onde estão beacons, probe resp, EAPOL, etc.)
    wifi_promiscuous_filter_t filter = {};
    filter.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT | WIFI_PROMIS_FILTER_MASK_DATA;
    esp_wifi_set_promiscuous_filter(&filter);

    // Canal inicial (primeiro da lista configurada)
    esp_wifi_set_channel(WIFI_CHANNELS[0], WIFI_SECOND_CHAN_NONE);

    // Ativa modo promíscuo
    esp_wifi_set_promiscuous(true);
}

void Pwnagotchi::initSensors() {
    Serial.println("[WavePwn] initSensors() - inicializando sensores básicos");
    init_motion_wakeup();
}

// -----------------------------------------------------------------------------
// Boot animation inicial (usando apenas o wrapper LGFX por enquanto)
// -----------------------------------------------------------------------------
void Pwnagotchi::showBootAnimation() {
    lcd.fillScreen(TFT_BLACK);
    lcd.setTextColor(TFT_CYAN);
    lcd.setTextSize(3);
    lcd.setCursor(40, 100);
    lcd.println("WAVE");
    lcd.setCursor(40, 140);
    lcd.println("PWN");
    delay(1500);
    lcd.fillScreen(TFT_BLACK);
    lcd.setTextSize(2);
    lcd.setCursor(20, 120);
    lcd.println("O Pwnagotchi Renascido");
    delay(2000);
}

// -----------------------------------------------------------------------------
// Modo ZUMBI — funciona até com 1% de bateria
// -----------------------------------------------------------------------------
void Pwnagotchi::enterZombieMode() {
    if (in_zombie_mode) return;
    in_zombie_mode = true;

    Serial.println("[ZUMBI] Entrando em modo ZUMBI (ultra low-power)");

    // Brilho mínimo no display físico
    lcd.setBrightness(5);

    // Esconde a UI LVGL, mas mantém a estrutura para futuros wake-ups.
    lv_obj_add_flag(lv_scr_act(), LV_OBJ_FLAG_HIDDEN);

    // Acorda a cada 30s apenas para checar ameaças com a NEURA9.
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    esp_sleep_enable_timer_wakeup(30000000ULL); // 30 segundos em micros

    while (true) {
        // Ainda protege mesmo dormindo: a IA defensiva roda e atualiza o nível.
        uint8_t cls = neura9.predict();
        threat_level = cls;
        threat_confidence = neura9.get_confidence();

        Serial.printf("[ZUMBI] Tick - classe=%u (%s) conf=%.2f\n",
                      cls,
                      NEURA9_THREAT_LABELS[cls],
                      threat_confidence);

        // Dorme profundamente até o próximo tick; o resto do sistema fica em paz.
        esp_light_sleep_start();
    }
}