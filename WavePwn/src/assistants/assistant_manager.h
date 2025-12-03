#pragma once

#include <Arduino.h>

// Gerencia o nome do dispositivo e a escolha dinâmica de assistentes
// de voz (Alexa / Google Home / ambos / nenhum), com base em
// src/config/device_config.json.
//
// Formato esperado:
//
// {
//   "device_name": "CyberGuard Pro",
//   "assistant": "alexa",      // "alexa" | "google" | "both" | "none"
//   "language": "pt-BR",
//   "theme": "dark",
//   "owner": "Seu Nome",
//   "enable_voice_alerts": true,
//   "neura9_sensitivity": 0.78
// }

class AssistantManager {
public:
    void begin();
    void speak(const char *text);
    void send_status();

    const String &getDeviceName() const { return device_name; }
    const String &getAssistantType() const { return assistant_type; }
    bool voiceAlertsEnabled() const { return enable_voice_alerts; }
    float neura9Sensitivity() const { return neura9_sensitivity; }

private:
    String device_name = "CyberGuard Pro";
    String assistant_type = "none";
    String language = "pt-BR";
    String theme = "dark";
    String owner = "";
    bool   enable_voice_alerts = true;
    float  neura9_sensitivity = 0.78f;

    void load_from_sd();
};

extern AssistantManager assistantManager;