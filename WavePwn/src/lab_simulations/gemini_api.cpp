#include "lab_simulations/gemini_api.h"

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <SD.h>
#include <ArduinoJson.h>

// Caminho do arquivo que armazena a chave de API do Gemini.
static const char* GEMINI_KEY_PATH = "/config/gemini_key.txt";

bool GeminiAPI::has_key() {
  return SD.exists(GEMINI_KEY_PATH);
}

String GeminiAPI::read_key() {
  if (!SD.exists(GEMINI_KEY_PATH)) {
    return String();
  }

  File f = SD.open(GEMINI_KEY_PATH);
  if (!f) {
    return String();
  }

  String key = f.readStringUntil('\n');
  f.close();
  key.trim();
  return key;
}

String GeminiAPI::extract_text(const String& response) {
  StaticJsonDocument<4096> doc;
  DeserializationError err = deserializeJson(doc, response);
  if (err) {
    // Se não conseguimos parsear como JSON, devolve a resposta bruta.
    return response;
  }

  JsonArray candidates = doc["candidates"].as<JsonArray>();
  if (candidates.isNull() || candidates.size() == 0) {
    return String(F("Resposta vazia do Gemini"));
  }

  JsonObject first = candidates[0];
  JsonArray parts = first["content"]["parts"].as<JsonArray>();
  if (!parts.isNull()) {
    for (JsonObject part : parts) {
      const char* text = part["text"];
      if (text && text[0]) {
        return String(text);
      }
    }
  }

  return String(F("Resposta do Gemini sem campo \"text\" utilizável"));
}

String GeminiAPI::ask(const String& prompt) {
  String key = read_key();
  if (key.isEmpty()) {
    return F("API key não configurada em /config/gemini_key.txt");
  }

  if (WiFi.status() != WL_CONNECTED) {
    return F("Wi-Fi não conectado — Gemini requer acesso à Internet");
  }

  WiFiClientSecure client;
  client.setInsecure();  // Para simplificar no ESP32; ideal seria usar CA raiz.

  HTTPClient http;
  String url =
      String(F("https://generativelanguage.googleapis.com/v1beta/models/"
               "gemini-pro:generateContent?key=")) +
      key;

  if (!http.begin(client, url)) {
    return F("Falha ao iniciar conexão HTTPS com o Gemini");
  }

  http.addHeader("Content-Type", "application/json");

  String json;
  json.reserve(512);
  json += F("{\"contents\":[{\"parts\":[{\"text\":\"");
  // Escapa aspas e barras invertidas no prompt.
  String safePrompt = prompt;
  safePrompt.replace("\\", "\\\\");
  safePrompt.replace("\"", "\\\"");
  json += safePrompt;
  json += F("\"}]}]}");

  int code = http.POST(json);
  String response = http.getString();
  http.end();

  if (code < 200 || code >= 300) {
    String err = F("Erro HTTP ao chamar Gemini: ");
    err += String(code);
    return err;
  }

  return extract_text(response);
}