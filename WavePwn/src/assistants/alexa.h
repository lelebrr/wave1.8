#pragma once

#include <Arduino.h>

// Inicialização da integração Alexa (via fauxmoESP).
// `deviceName` é o nome amigável configurado em device_config.json.
void alexa_init(const String &deviceName);

// Fala uma frase usando Alexa (quando configurado).
void alexa_speak(const char *text);

// Envia um resumo de status (bateria, ameaças etc.) como frase.
void alexa_send_status(const char *status);