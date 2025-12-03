#include <Arduino.h>
#include <WiFi.h>
#include <SD.h>
#include <esp_wifi.h>

#include "pwnagotchi.h"
#include "ui.h"
#include "config.h"
#include "storage.h"
#include "wifi_sniffer.h"
#include "sensors.h"
#include "audio.h"
#include "capture.h"
#include "ai/neura9_inference.h"

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

    Serial.println("WAVE PWN PRONTO PARA DOMINAR");
}

void Pwnagotchi::update() {
    // Uptimes simples em segundos (aprox.)
    uptime = millis() / 1000;

    // Placeholders de ambiente (até integrar sensores reais)
    current_channel  = 0;
    battery_percent  = 100.0f;
    is_charging      = false;
    is_moving        = false;

    ui_update_stats(
        aps_seen,
        handshakes,
        pmkids,
        deauths,
        current_channel,
        battery_percent,
        is_moving
    );

    // NEURA9 avalia o ambiente periodicamente
    static uint32_t last_ai = 0;
    uint32_t now = millis();
    if (now - last_ai > 800) {
        uint8_t cls = neura9.predict();
        float conf  = neura9.get_confidence();
        (void)cls;
        (void)conf;
        // Futuro: exibir classe/confiança na HUD; por enquanto apenas log.
        Serial.printf("[NEURA9] classe=%u conf=%.2f\n", cls, conf);
        last_ai = now;
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

    Serial.println("[WavePwn] microSD pronto para captura de handshakes");
}

void Pwnagotchi::initWiFiMonitor() {
    Serial.println("[WavePwn] initWiFiMonitor() - configurando Wi-Fi em modo promíscuo");

    WiFi.mode(WIFI_MODE_STA);
    WiFi.disconnect(true, true);

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