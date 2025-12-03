#pragma once

#include <Arduino.h>

// Inicialização da integração com Google Home.
// Aqui usamos uma camada leve que pode ser integrada via Home Assistant
// ou outros conectores externos.
void google_init(const String &deviceName);

// Fala / envia uma frase para o ecossistema Google (quando configurado).
void google_speak(const char *text);

// Envia um resumo de status textual.
void google_send_status(const char *status);