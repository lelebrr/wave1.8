#include <Arduino.h>
#include "pwnagotchi.h"
#include "ui.h"
#include "config.h"
#include "storage.h"
#include "wifi_sniffer.h"
#include "sensors.h"
#include "audio.h"

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

// -----------------------------------------------------------------------------
// Pwnagotchi - ciclo de vida principal
// -----------------------------------------------------------------------------
void Pwnagotchi::begin() {
    initDisplay();
    showBootAnimation();
    initSD();
    initSensors();
    initWiFiMonitor();
    lv_init();
    ui_init();
    Serial.println("WAVE PWN PRONTO PARA DOMINAR");
}

void Pwnagotchi::update() {
    // Uptimes simples em segundos (aprox.)
    uptime = millis() / 1000;

    // Futuro: atualizar stats de redes, handshakes, PMKID etc.

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
    // Futuro: montar microSD, criar pastas de PCAPs, logs, configs, etc.
    Serial.println("[WavePwn] initSD() - stub inicial");
}

void Pwnagotchi::initWiFiMonitor() {
    // Futuro: inicializar esp_wifi em modo promíscuo + ESP32Marauder helpers.
    Serial.println("[WavePwn] initWiFiMonitor() - stub inicial");
}

void Pwnagotchi::initSensors() {
    // Futuro: inicializar QMI8658, AXP2101, PCF85063 e mapear para humor/estados.
    Serial.println("[WavePwn] initSensors() - stub inicial");
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