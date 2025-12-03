#include "home_assistant.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <HTTPClient.h>

#include "pwnagotchi.h"
#include "ai/neura9_inference.h"

// Configuração padrão – pode ser sobrescrita em config.h ou via build_flags.
//
// Exemplo em platformio.ini:
//   build_flags =
//       -DHA_ENABLED=1
//       -DHA_BASE_URL=\"http://homeassistant.local:8123\"
//       -DHA_TOKEN=\"SEU_LONG_LIVED_ACCESS_TOKEN\"

#ifndef HA_ENABLED
#define HA_ENABLED 0
#endif

#ifndef HA_BASE_URL
#define HA_BASE_URL "http://homeassistant.local:8123"
#endif

#ifndef HA_TOKEN
#define HA_TOKEN ""
#endif

extern Pwnagotchi pwn;
extern Neura9 neura9;

static WiFiUDP ntpUDP;
static NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 3600000);

// Cache para evitar flood desnecessário no Home Assistant.
static String   last_published_level;
static uint32_t last_publish_ms = 0;

void ha_init(void) {
#if HA_ENABLED
    timeClient.begin();
    timeClient.update();
    Serial.println("[HA] Integração Home Assistant ativada (HA_ENABLED=1)");
#else
    Serial.println("[HA] Integração Home Assistant desativada (HA_ENABLED=0)");
#endif
}

void ha_loop(void) {
#if HA_ENABLED
    const uint32_t now = millis();
    static uint32_t last_ntp = 0;
    if (now - last_ntp > 3600000UL) {  // 1h
        timeClient.update();
        last_ntp = now;
    }
#else
    (void)timeClient;
#endif
}

#if HA_ENABLED
static bool ha_has_token() {
    return strlen(HA_TOKEN) > 0;
}

static void ha_post_state(const String &entity_id,
                          const String &state,
                          const String &attributes_json) {
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }
    if (!ha_has_token()) {
        return;
    }

    HTTPClient http;
    String url = String(HA_BASE_URL) + "/api/states/" + entity_id;

    if (!http.begin(url)) {
        return;
    }

    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", String("Bearer ") + HA_TOKEN);

    String body;
    body.reserve(256);
    body += "{\"state\":\"";
    body += state;
    body += "\"";

    if (attributes_json.length() > 0) {
        body += ",\"attributes\":";
        body += attributes_json;
    }

    body += "}";

    int code = http.POST(body);
    if (code <= 0) {
        Serial.printf("[HA] Falha ao enviar estado para %s (code=%d)\n",
                      entity_id.c_str(),
                      code);
    }

    http.end();
}
#endif

void ha_send_threat(const char *level) {
    if (!level) return;

#if !HA_ENABLED
    (void)level;
    return;
#else
    const uint32_t now = millis();

    // Evita flood: só publica se mudou de estado ou a cada 10s.
    if (last_published_level == level && (now - last_publish_ms) < 10000UL) {
        return;
    }

    last_published_level = level;
    last_publish_ms      = now;

    // Entidade principal: sensor.wavepwn_threat_level
    String threat_state = level;

    float conf      = neura9.get_confidence();
    float battery   = pwn.battery_percent;
    bool  deauth_on = (pwn.threat_level == 4);  // DEAUTH_DETECTED

    String attrs;
    attrs.reserve(256);
    attrs += "{";
    attrs += "\"friendly_name\":\"WavePwn Threat Level\",";
    attrs += "\"unit_of_measurement\":\"\",";
    attrs += "\"confidence\":";
    attrs += String(conf, 2);
    attrs += ",\"battery\":";
    attrs += String(battery, 0);
    attrs += ",\"deauth_detected\":";
    attrs += deauth_on ? "true" : "false";
    attrs += "}";

    ha_post_state("sensor.wavepwn_threat_level", threat_state, attrs);

    // Entidade de bateria: sensor.wavepwn_battery
    String bat_attrs = "{\"friendly_name\":\"WavePwn Battery\",\"unit_of_measurement\":\"%\",\"device_class\":\"battery\"}";
    ha_post_state("sensor.wavepwn_battery",
                  String((int)(battery + 0.5f)),
                  bat_attrs);

    // Entidade binária: binary_sensor.wavepwn_deauth_attack
    String bin_attrs = "{\"friendly_name\":\"WavePwn Deauth Attack\",\"device_class\":\"safety\"}";
    ha_post_state("binary_sensor.wavepwn_deauth_attack",
                  deauth_on ? "ON" : "OFF",
                  bin_attrs);

    Serial.printf("[HA] Publicado estado para Home Assistant: level=%s conf=%.2f bat=%.0f deauth=%s\n",
                  threat_state.c_str(),
                  conf,
                  battery,
                  deauth_on ? "true" : "false");
#endif
}