#include <Arduino.h>
#include <WiFi.h>
#include <SD.h>
#include <esp_wifi.h>
#include <esp_sleep.h>
#include <ArduinoJson.h>

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
#include "src/home_assistant.h"
#include "ble_grid/pwn_grid.h"
#include "anti_tamper/secure_boot.h"
#include "reports/tiny_pdf.h"
#include "assistants/assistant_manager.h"

uint32_t threat_count = 0;

// -----------------------------------------------------------------------------
// Tema dark/light - aplica tema global na tela ativa do LVGL.
// -----------------------------------------------------------------------------
void switch_theme(bool dark) {
    lv_obj_t *scr = lv_scr_act();
    if (!scr) return;

    if (dark) {
        lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
        lv_obj_set_style_text_color(scr, lv_color_hex(0x00FFFF), 0);
    } else {
        lv_obj_set_style_bg_color(scr, lv_color_hex(0xF0F0F0), 0);
        lv_obj_set_style_text_color(scr, lv_color_hex(0x000000), 0);
    }
}

// -----------------------------------------------------------------------------
// Carregamento de idioma via JSON no microSD com fallback.
// Espera arquivos em /sd/lang/<lang>.json (ex: /sd/lang/pt-BR.json).
// -----------------------------------------------------------------------------
void load_language(const char* lang) {
    if (!lang) return;

    String path = "/sd/lang/";
    path += lang;
    path += ".json";

    File f = SD.open(path.c_str());
    if (!f) {
        Serial.printf("[LANG] Falha ao abrir %s\n", path.c_str());
        if (strcmp(lang, "en-US") != 0) {
            load_language("en-US");
        }
        return;
    }

    StaticJsonDocument<1024> doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();

    if (err) {
        Serial.printf("[LANG] Erro ao parsear %s: %s\n",
                      path.c_str(),
                      err.c_str());
        if (strcmp(lang, "en-US") != 0) {
            load_language("en-US");
        }
        return;
    }

    const char* code = doc["language"] | lang;
    Serial.printf("[LANG] Idioma carregado: %s (%s)\n", lang, code);
    // Nesta etapa mantemos o parser como stub; os textos localizados
    // podem ser ligados aos labels da UI nas próximas evoluções.
}

// -----------------------------------------------------------------------------
// Boot premium animado usando LVGL (15 frames ~ 2s).
// -----------------------------------------------------------------------------
void show_premium_boot() {
    lv_obj_t *scr = lv_scr_act();
    if (!scr || !ui.face) return;

    for (int i = 0; i < 15; i++) {
        uint8_t v = (uint8_t)((i + 1) * 255 / 15);
        lv_color_t c = lv_color_make(0, v, 255 - v);
        lv_obj_set_style_bg_color(ui.face, c, 0);
        lv_timer_handler();
        delay(133);  // ~15 FPS → 2s
    }
}

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
    // Proteção anti-tamper / secure boot
    anti_tamper_check();

    initDisplay();
    showBootAnimation();

    initSD();
    capture_init();

    initSensors();
    initWiFiMonitor();

    // Nome do dispositivo + escolha de assistente (Alexa / Google / ambos / nenhum)
    assistantManager.begin();

    // Callback promíscuo -> motor de captura de handshakes/PMKID
    esp_wifi_set_promiscuous_rx_cb([](void* buf, wifi_promiscuous_pkt_type_t type) {
        (void)type;
        wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
        capture_packet_handler((uint8_t*)pkt, pkt->rx_ctrl.sig_len, pkt->rx_ctrl.channel);
    });

    lv_init();
    ui_init();

    // Tema inicial e idioma padrão
    switch_theme(true);          // dark por padrão
    load_language("pt-BR");
    show_premium_boot();

    // Inicializa IA defensiva local (NEURA9)
    if (!neura9.begin()) {
        Serial.println("[NEURA9] Falha ao inicializar IA defensiva (modo stub)");
    }

    // BLE PwnGrid cooperativo
    pwnGrid.begin();

    // Webserver + WebSocket + OTA seguro
    webserver_start();

    // Integração opcional com Home Assistant / Google Home
    ha_init();

    Serial.println("WAVE PWN PRONTO PARA DOMINAR");
}

void Pwnagotchi::update() {
    // Uptimes simples em segundos (aprox.)
    uptime = millis() / 1000;

    // Atualiza timers relacionados ao Home Assistant (NTP etc.).
    ha_loop();

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

    // NEURA9 avalia o ambiente periodicamente (HUD + PwnGrid + Home Assistant)
    static uint32_t last_ai = 0;
    uint32_t now = millis();
    if (now - last_ai > 800) {
        uint8_t cls = neura9.predict();
        float conf  = neura9.get_confidence();
        threat_level = cls;
        threat_confidence = conf;

        if (cls > 0) {
            threat_count++;
        }

        Serial.printf("[NEURA9] classe=%u (%s) conf=%.2f\n",
                      cls,
                      NEURA9_THREAT_LABELS[cls],
                      conf);

        last_ai = now;

        // Compartilha nível de ameaça com a PwnGrid cooperativa
        pwnGrid.share_threat_level(cls);

        // Publica o nível de ameaça atual no Home Assistant / Google Home.
        ha_send_threat(NEURA9_THREAT_LABELS[cls]);
    }

    // Tema dark/light automatico simples baseado em tempo de execução
    static uint32_t last_theme_toggle = 0;
    static bool theme_dark = true;
    if (now - last_theme_toggle > 600000) { // a cada 10 minutos
        theme_dark = !theme_dark;
        switch_theme(theme_dark);
        last_theme_toggle = now;
    }

    // Web dashboard em tempo real
    webserver_send_stats();

    // Envia periodicamente um resumo de status para os assistentes de voz
    static uint32_t last_voice_status = 0;
    if (now - last_voice_status > 60000UL) { // a cada 60s
        assistantManager.send_status();
        last_voice_status = now;
    }

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
    SD.mkdir("/sd/lang");
    SD.mkdir("/sd/reports");
    SD.mkdir("/sd/lab_logs");

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