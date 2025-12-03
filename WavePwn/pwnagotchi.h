#pragma once
#include <lvgl.h>
#include <stdint.h>

// Cores básicas usadas nas telas iniciais (RGB565)
#ifndef TFT_BLACK
#define TFT_BLACK 0x0000
#endif

#ifndef TFT_CYAN
#define TFT_CYAN 0x07FF
#endif

// Implementação mínima de um wrapper LGFX para o boot inicial.
// Nas próximas etapas podemos trocar por um mapeamento real do display/QSPI.
class LGFX {
public:
    void begin();
    void fillScreen(uint32_t color);
    void setTextColor(uint32_t color);
    void setTextSize(uint8_t size);
    void setCursor(int16_t x, int16_t y);
    void println(const char *text);

    // Placeholder para controle de brilho (compatível com futura LovyanGFX real)
    void setBrightness(uint8_t value);
};

class Pwnagotchi {
public:
    void begin();
    void update();

    // Stats
    uint32_t uptime = 0;
    uint32_t aps_seen = 0;
    uint32_t handshakes = 0;
    uint32_t pmkids = 0;
    uint32_t deauths = 0;

private:
    LGFX lcd;
    void initDisplay();
    void initSD();
    void initWiFiMonitor();
    void initSensors();
    void showBootAnimation();
};