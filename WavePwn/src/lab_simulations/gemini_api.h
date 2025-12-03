/*
  gemini_api.h - Integração opcional com Google Gemini para análise de logs

  Esta API é pensada para:
  - Enviar descrições de cenários sintetizados (logs, resumos, texto).
  - Receber respostas em linguagem natural para apoio didático.

  Não deve ser usada para enviar dados pessoais sensíveis ou payloads
  completos de tráfego. Priorize resumos e estatísticas agregadas.

  A chave de API é lida de /config/gemini_key.txt no microSD.
*/

#pragma once

#include <Arduino.h>

class GeminiAPI {
public:
  // Retorna true se uma chave de API foi configurada em /config/gemini_key.txt.
  static bool has_key();

  // Envia um prompt ao modelo Gemini e retorna o texto principal da resposta.
  // Em caso de erro, retorna uma string descritiva em português.
  static String ask(const String& prompt);

private:
  static String read_key();
  static String extract_text(const String& response);
};