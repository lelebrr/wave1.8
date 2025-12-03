#include "assistants/google_home.h"

#include <Arduino.h>

#include "pwnagotchi.h"
#include "src/home_assistant.h"
#include "ai/neura9_inference.h"

extern Pwnagotchi pwn;
extern Neura9 neura9;

static String g_device_name;
static bool   g_google_enabled = false;

void google_init(const String &deviceName) {
    g_device_name    = deviceName;
    g_google_enabled = true;

    Serial.printf("[Google] Integração Google Home inicializada para '%s' (via Home Assistant opcional)\n",
                  g_device_name.c_str());
}

void google_speak(const char *text) {
    if (!g_google_enabled || !text) {
        return;
    }

    // Nesta etapa, não fazemos TTS direto. A ideia é que automações no
    // Home Assistant usem ha_send_threat ou sensores publicados para
    // anunciar o texto. Aqui registramos no log.
    Serial.printf("[Google] %s\n", text);
}

void google_send_status(const char *status) {
    if (!g_google_enabled || !status) {
        return;
    }

    Serial.printf("[Google] Status: %s\n", status);

    // Publica o nível de ameaça atual no Home Assistant, que por sua vez
    // pode expor isso ao Google Home.
    uint8_t cls = pwn.threat_level;
    if (cls > 9) cls = 0;
    ha_send_threat(NEURA9_THREAT_LABELS[cls]);
}