#include "webserver.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <Update.h>

#include "utils/ota_secure.h"

#include "pwnagotchi.h"
#include "ai/neura9_inference.h"

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

// -----------------------------------------------------------------------------
// HTTP handlers
// -----------------------------------------------------------------------------

static void handle_root() {
    serve_embedded(data_web_index_html_start, data_web_index_html_end, "text/html");
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