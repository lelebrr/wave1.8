#include "webserver.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <Update.h>
#include <SD.h>
#include <ArduinoJson.h>

#include "utils/ota_secure.h"

#include "pwnagotchi.h"
#include "ai/neura9_inference.h"
#include "lab_simulations/simulation_manager.h"
#include "lab_simulations/gemini_api.h"

extern bool lab_mode;

// Arquivos embutidos no firmware (PlatformIO board_build.embed_files)
extern const uint8_t data_web_index_html_start[] asm("_binary_data_web_index_html_start");
extern const uint8_t data_web_index_html_end[]   asm("_binary_data_web_index_html_end");

extern const uint8_t data_web_style_css_start[]  asm("_binary_data_web_style_css_start");
extern const uint8_t data_web_style_css_end[]    asm("_binary_data_web_style_css_end");

extern const uint8_t data_web_chart_min_js_start[] asm("_binary_data_web_chart_min_js_start");
extern const uint8_t data_web_chart_min_js_end[]   asm("_binary_data_web_chart_min_js_end");

extern const uint8_t data_web_favicon_ico_start[] asm("_binary_data_web_favicon_ico_start");
extern const uint8_t data_web_favicon_ico_end[]   asm("_binary_data_web_favicon_ico_end");

extern const uint8_t data_web_config_html_start[] asm("_binary_data_web_config_html_start");
extern const uint8_t data_web_config_html_end[]   asm("_binary_data_web_config_html_end");

extern const uint8_t ota_update_html_start[]  asm("_binary_ota_update_html_start");
extern const uint8_t ota_update_html_end[]    asm("_binary_ota_update_html_end");

extern Pwnagotchi pwn;

// Servidor HTTP + WebSocket
static WebServer http_server(80);
static WebSocketsServer ws_server(81);

// OTA básico com HTTP Basic Auth
static const char* OTA_USER = "admin";
static const char* OTA_PASS = "wavepwn";

static String last_log_line;

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

static void log_line(const String& line) {
    last_log_line = line;

    if (lab_mode) {
        // Modo laboratorio: aplica uma ofuscacao simples nos logs enviados
        // pelo dashboard (mantendo o Serial legivel para debug local).
        String obf = line;
        for (size_t i = 0; i < obf.length(); ++i) {
            obf[i] = obf[i] ^ 0x42;
        }
        Serial.println(obf);
    } else {
        Serial.println(line);
    }
}

static void serve_embedded(const uint8_t* start,
                           const uint8_t* end,
                           const char* contentType) {
    size_t len = end - start;
    http_server.send_P(200, contentType, (PGM_P)start, len);
}

static String format_uptime(uint32_t seconds) {
    char buf[16];
    uint32_t h = seconds / 3600;
    uint32_t m = (seconds % 3600) / 60;
    uint32_t s = seconds % 60;
    snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu",
             (unsigned long)h,
             (unsigned long)m,
             (unsigned long)s);
    return String(buf);
}

static bool ensure_config_dir() {
    if (SD.exists("/config")) {
        return true;
    }
    return SD.mkdir("/config");
}

// -----------------------------------------------------------------------------
// HTTP handlers
// -----------------------------------------------------------------------------

static void handle_root() {
    serve_embedded(data_web_index_html_start, data_web_index_html_end, "text/html");
}

static void handle_config_page() {
    serve_embedded(data_web_config_html_start, data_web_config_html_end, "text/html");
}

static void handle_index() {
    handle_root();
}

static void handle_style() {
    serve_embedded(data_web_style_css_start, data_web_style_css_end, "text/css");
}

static void handle_chart() {
    serve_embedded(data_web_chart_min_js_start, data_web_chart_min_js_end, "application/javascript");
}

static void handle_favicon() {
    serve_embedded(data_web_favicon_ico_start, data_web_favicon_ico_end, "image/x-icon");
}

// --------------------------
// Config REST helpers
// --------------------------

static void handle_api_config_device_get() {
    DynamicJsonDocument doc(512);

    File f = SD.open("/config/device_config.json");
    if (f) {
        DeserializationError err = deserializeJson(doc, f);
        f.close();
        if (err) {
            doc.clear();
        }
    }

    if (!doc.containsKey("device_name"))        doc["device_name"]        = "CyberGuard Pro";
    if (!doc.containsKey("assistant"))          doc["assistant"]          = "none";
    if (!doc.containsKey("language"))           doc["language"]           = "pt-BR";
    if (!doc.containsKey("theme"))              doc["theme"]              = "dark";
    if (!doc.containsKey("owner"))              doc["owner"]              = "";
    if (!doc.containsKey("enable_voice_alerts")) doc["enable_voice_alerts"] = true;
    if (!doc.containsKey("neura9_sensitivity")) doc["neura9_sensitivity"] = 0.78f;

    String out;
    serializeJson(doc, out);
    http_server.send(200, "application/json", out);
}

static void handle_api_config_device_post() {
    String body = http_server.arg("plain");
    if (body.isEmpty()) {
        http_server.send(400, "application/json", "{\"error\":\"empty body\"}");
        return;
    }

    DynamicJsonDocument doc(1024);
    DeserializationError err = deserializeJson(doc, body);
    if (err) {
        http_server.send(400, "application/json", "{\"error\":\"invalid json\"}");
        return;
    }

    if (!ensure_config_dir()) {
        http_server.send(500, "application/json", "{\"error\":\"config dir\"}");
        return;
    }

    File f = SD.open("/config/device_config.json", FILE_WRITE);
    if (!f) {
        http_server.send(500, "application/json", "{\"error\":\"open config\"}");
        return;
    }

    serializeJson(doc, f);
    f.close();
    http_server.send(200, "application/json", "{\"ok\":true}");
}

// --------------------------
// Lab Mode REST helpers
// --------------------------

static bool load_lab_pin(String &pin_out) {
    File f = SD.open("/config/lab_config.json");
    if (!f) {
        return false;
    }

    DynamicJsonDocument doc(256);
    DeserializationError err = deserializeJson(doc, f);
    f.close();
    if (err) {
        return false;
    }

    const char* p = doc["lab_pin"];
    if (!p) {
        return false;
    }

    pin_out = String(p);
    if (pin_out.length() != 6) {
        return false;
    }
    for (size_t i = 0; i < pin_out.length(); ++i) {
        if (pin_out[i] < '0' || pin_out[i] > '9') {
            return false;
        }
    }
    return true;
}

static void handle_api_lab_status() {
    bool guard = SD.exists("/sd/.enable_lab_attacks");
    String stored_pin;
    bool pin_set = load_lab_pin(stored_pin);

    DynamicJsonDocument doc(256);
    doc["lab_guard_file"] = guard;
    doc["pin_set"]        = pin_set;
    doc["lab_unlocked"]   = SimulationManager::is_lab_unlocked();

    String out;
    serializeJson(doc, out);
    http_server.send(200, "application/json", out);
}

static void handle_api_lab_set_pin() {
    String body = http_server.arg("plain");
    DynamicJsonDocument doc(256);
    DeserializationError err = deserializeJson(doc, body);
    if (err) {
        http_server.send(400, "application/json", "{\"error\":\"invalid json\"}");
        return;
    }

    const char* pin = doc["pin"];
    if (!pin) {
        http_server.send(400, "application/json", "{\"error\":\"pin missing\"}");
        return;
    }

    String pin_s(pin);
    pin_s.trim();
    if (pin_s.length() != 6) {
        http_server.send(400, "application/json", "{\"error\":\"pin length\"}");
        return;
    }
    for (size_t i = 0; i < pin_s.length(); ++i) {
        if (pin_s[i] < '0' || pin_s[i] > '9') {
            http_server.send(400, "application/json", "{\"error\":\"pin digits\"}");
            return;
        }
    }

    if (!ensure_config_dir()) {
        http_server.send(500, "application/json", "{\"error\":\"config dir\"}");
        return;
    }

    DynamicJsonDocument outDoc(128);
    outDoc["lab_pin"] = pin_s;

    File f = SD.open("/config/lab_config.json", FILE_WRITE);
    if (!f) {
        http_server.send(500, "application/json", "{\"error\":\"open lab_config\"}");
        return;
    }
    serializeJson(outDoc, f);
    f.close();

    // Sempre que o PIN é alterado, o modo lab volta a ficar bloqueado.
    SimulationManager::set_lab_unlocked(false);

    http_server.send(200, "application/json", "{\"ok\":true}");
}

static void handle_api_lab_unlock() {
    String body = http_server.arg("plain");
    DynamicJsonDocument doc(256);
    DeserializationError err = deserializeJson(doc, body);
    if (err) {
        http_server.send(400, "application/json", "{\"ok\":false,\"error\":\"invalid json\"}");
        return;
    }

    const char* pin = doc["pin"];
    if (!pin) {
        http_server.send(400, "application/json", "{\"ok\":false,\"error\":\"pin missing\"}");
        return;
    }

    String stored;
    if (!load_lab_pin(stored)) {
        http_server.send(400, "application/json", "{\"ok\":false,\"error\":\"pin not set\"}");
        return;
    }

    if (stored != String(pin)) {
        http_server.send(403, "application/json", "{\"ok\":false,\"error\":\"pin mismatch\"}");
        return;
    }

    SimulationManager::set_lab_unlocked(true);

    http_server.send(200, "application/json", "{\"ok\":true}");
}

// --------------------------
// Gemini REST helpers
// --------------------------

static void handle_api_gemini_key() {
    String body = http_server.arg("plain");
    DynamicJsonDocument doc(256);
    DeserializationError err = deserializeJson(doc, body);
    if (err) {
        http_server.send(400, "application/json", "{\"error\":\"invalid json\"}");
        return;
    }

    const char* key = doc["key"];
    if (!key || !key[0]) {
        http_server.send(400, "application/json", "{\"error\":\"key missing\"}");
        return;
    }

    if (!ensure_config_dir()) {
        http_server.send(500, "application/json", "{\"error\":\"config dir\"}");
        return;
    }

    File f = SD.open("/config/gemini_key.txt", FILE_WRITE);
    if (!f) {
        http_server.send(500, "application/json", "{\"error\":\"open gemini_key\"}");
        return;
    }
    f.println(key);
    f.close();

    http_server.send(200, "application/json", "{\"ok\":true}");
}

static void handle_api_gemini_ask() {
    String body = http_server.arg("plain");
    DynamicJsonDocument doc(512);
    DeserializationError err = deserializeJson(doc, body);
    if (err) {
        http_server.send(400, "application/json", "{\"error\":\"invalid json\"}");
        return;
    }

    const char* prompt = doc["prompt"];
    if (!prompt || !prompt[0]) {
        http_server.send(400, "application/json", "{\"error\":\"prompt missing\"}");
        return;
    }

    String answer = GeminiAPI::ask(String(prompt));

    DynamicJsonDocument outDoc(1024);
    outDoc["response"] = answer;
    String out;
    serializeJson(outDoc, out);
    http_server.send(200, "application/json", out);
}

static bool ensure_ota_auth() {
    if (!http_server.authenticate(OTA_USER, OTA_PASS)) {
        http_server.requestAuthentication();
        return false;
    }
    return true;
}

static void handle_ota_page() {
    if (!ensure_ota_auth()) return;
    serve_embedded(ota_update_html_start, ota_update_html_end, "text/html");
}

static void handle_ota_upload() {
    if (!ensure_ota_auth()) return;

    HTTPUpload& upload = http_server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        log_line("[OTA] Iniciando update: " + String(upload.filename.c_str()));
        if (!ota_begin_secure(UPDATE_SIZE_UNKNOWN)) {
            log_line("[OTA] Falha ao iniciar buffer OTA seguro");
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (!ota_write_chunk(upload.buf, upload.currentSize)) {
            log_line("[OTA] Erro ao escrever chunk OTA");
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (ota_finalize(true)) {
            log_line("[OTA] Update concluído com sucesso, reiniciando...");
        } else {
            log_line("[OTA] Falha ao finalizar OTA");
        }
    }
}

static void handle_ota_result() {
    if (!ensure_ota_auth()) return;
    if (Update.hasError()) {
        http_server.send(500, "text/plain", "OTA FAILED");
    } else {
        http_server.send(200, "text/plain", "OTA OK, rebooting...");
    }
    delay(500);
    ESP.restart();
}

static void handle_not_found() {
    http_server.send(404, "text/plain", "Not found");
}

// -----------------------------------------------------------------------------
// WebSocket
// -----------------------------------------------------------------------------

static void on_ws_event(uint8_t num,
                        WStype_t type,
                        uint8_t* payload,
                        size_t length) {
    (void)payload;
    (void)length;

    switch (type) {
        case WStype_CONNECTED: {
            IPAddress ip = ws_server.remoteIP(num);
            String msg = "[WS] Cliente conectado: " + ip.toString();
            log_line(msg);
            break;
        }
        case WStype_DISCONNECTED:
            log_line("[WS] Cliente desconectado");
            break;
        default:
            break;
    }
}

// -----------------------------------------------------------------------------
// API pública
// -----------------------------------------------------------------------------

void webserver_start() {
    // Rotas HTTP principais
    http_server.on("/", HTTP_GET, handle_root);
    http_server.on("/index.html", HTTP_GET, handle_index);
    http_server.on("/style.css", HTTP_GET, handle_style);
    http_server.on("/chart.min.js", HTTP_GET, handle_chart);
    http_server.on("/favicon.ico", HTTP_GET, handle_favicon);
    http_server.on("/config.html", HTTP_GET, handle_config_page);

    // API de configuração do dispositivo
    http_server.on("/api/config/device", HTTP_GET, handle_api_config_device_get);
    http_server.on("/api/config/device", HTTP_POST, handle_api_config_device_post);

    // API de Lab Mode (simulações acadêmicas)
    http_server.on("/api/lab/status", HTTP_GET, handle_api_lab_status);
    http_server.on("/api/lab/set_pin", HTTP_POST, handle_api_lab_set_pin);
    http_server.on("/api/lab/unlock", HTTP_POST, handle_api_lab_unlock);

    // API de integração com Gemini
    http_server.on("/api/gemini/key", HTTP_POST, handle_api_gemini_key);
    http_server.on("/api/gemini/ask", HTTP_POST, handle_api_gemini_ask);

    // OTA seguro
    http_server.on("/ota/update.html", HTTP_GET, handle_ota_page);
    http_server.on(
        "/ota/firmware",
        HTTP_POST,
        handle_ota_result,
        handle_ota_upload
    );

    http_server.onNotFound(handle_not_found);

    http_server.begin();
    log_line("[WEB] HTTP server iniciado na porta 80");

    ws_server.begin();
    ws_server.onEvent(on_ws_event);
    log_line("[WEB] WebSocket server iniciado na porta 81");
}

void webserver_send_stats() {
    http_server.handleClient();
    ws_server.loop();

    if (ws_server.connectedClients() == 0) {
        return;
    }

    // NEURA9 - garante que a IA defensiva continue ativa mesmo sem UI física.
    uint8_t cls = neura9.predict();
    pwn.threat_level = cls;
    pwn.threat_confidence = neura9.get_confidence();

    String json;
    json.reserve(256);

    json += "{";
    json += "\"uptime\":";
    json += String(pwn.uptime);
    json += ",\"battery\":";
    json += String(pwn.battery_percent, 0);
    json += ",\"aps\":";
    json += String(pwn.aps_seen);
    json += ",\"hs\":";
    json += String(pwn.handshakes);
    json += ",\"pmkid\":";
    json += String(pwn.pmkids);
    json += ",\"ai\":\"";
    json += NEURA9_THREAT_LABELS[cls];
    json += "\"";

    if (last_log_line.length() > 0) {
        json += ",\"log\":\"";
        String safe = last_log_line;
        safe.replace("\\", "\\\\");
        safe.replace("\"", "\\\"");
        safe.replace("\n", "\\n");
        json += safe;
        json += "\"";
    } else {
        json += ",\"log\":\"\"";
    }

    json += "}";

    ws_server.broadcastTXT(json);
}