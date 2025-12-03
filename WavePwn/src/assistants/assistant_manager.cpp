#include "assistants/assistant_manager.h"

#include <SD.h>
#include <ArduinoJson.h>

#include "assistants/alexa.h"
#include "assistants/google_home.h"
#include "ai/neura9_inference.h"
#include "pwnagotchi.h"

extern Pwnagotchi pwn;
extern Neura9     neura9;

AssistantManager assistantManager;

void AssistantManager::load_from_sd() {
    File f = SD.open("/config/device_config.json");
    if (!f) {
        Serial.println("[Assistant] device_config.json não encontrado em /config — usando defaults");
        return;
    }

    DynamicJsonDocument doc(1024);
    DeserializationError err = deserializeJson(doc, f);
    f.close();

    if (err) {
        Serial.printf("[Assistant] Erro ao parsear device_config.json: %s\n", err.c_str());
        return;
    }

    device_name        = doc["device_name"]       | device_name;
    assistant_type     = doc["assistant"]         | assistant_type;
    language           = doc["language"]          | language;
    theme              = doc["theme"]             | theme;
    owner              = doc["owner"]             | owner;
    enable_voice_alerts = doc["enable_voice_alerts"] | enable_voice_alerts;
    neura9_sensitivity = doc["neura9_sensitivity"] | neura9_sensitivity;

    device_name.trim();
    assistant_type.trim();
    language.trim();
    theme.trim();
    owner.trim();

    if (assistant_type.length() == 0) {
        assistant_type = "none";
    }

    Serial.printf("[Assistant] device_name='%s', assistant='%s', lang='%s', theme='%s'\n",
                  device_name.c_str(),
                  assistant_type.c_str(),
                  language.c_str(),
                  theme.c_str());
}

void AssistantManager::begin() {
    load_from_sd();

    if (assistant_type == "alexa" || assistant_type == "both") {
        alexa_init(device_name);
    }
    if (assistant_type == "google" || assistant_type == "both") {
        google_init(device_name);
    }
}

void AssistantManager::speak(const char *text) {
    if (!text || !enable_voice_alerts) {
        return;
    }

    String msg = device_name;
    msg += " diz: ";
    msg += text;

    if (assistant_type.indexOf("alexa") >= 0) {
        alexa_speak(msg.c_str());
    }
    if (assistant_type.indexOf("google") >= 0) {
        google_speak(msg.c_str());
    }
}

void AssistantManager::send_status() {
    String status;
    status.reserve(128);

    uint8_t cls = pwn.threat_level;
    if (cls > 9) cls = 0;

    status += "Status de ";
    status += device_name;
    status += ": ambiente ";
    status += NEURA9_THREAT_LABELS[cls];
    status += ", confiança ";
    status += String(neura9.get_confidence(), 2);
    status += ", bateria ";
    status += String(pwn.battery_percent, 0);
    status += "%.";

    if (assistant_type.indexOf("alexa") >= 0) {
        alexa_send_status(status.c_str());
    }
    if (assistant_type.indexOf("google") >= 0) {
        google_send_status(status.c_str());
    }
}