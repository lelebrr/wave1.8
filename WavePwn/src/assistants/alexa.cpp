#include "assistants/alexa.h"

#include <WiFi.h>
#include <fauxmoESP.h>

#include "reports/tiny_pdf.h"
#include "pwnagotchi.h"
#include "ai/neura9_inference.h"

extern Pwnagotchi pwn;
extern Neura9 neura9;

// Instância global do fauxmoESP
static fauxmoESP g_fauxmo;
static String    g_device_name;
static bool      g_alexa_enabled = false;

static void handle_alexa_state(unsigned char device_id,
                               const char *device_name,
                               bool state,
                               unsigned char value) {
    (void)device_id;
    (void)value;

    if (!device_name) return;

    String name = device_name;

    // Exemplo de comando especial: se o dispositivo contiver "relatório",
    // gera um PDF simplificado usando tiny_pdf.h.
    if (name.indexOf("relatório") >= 0 || name.indexOf("relatorio") >= 0) {
        if (state) {
            generate_report();
        }
        return;
    }

    // Para outros comandos, apenas logamos no Serial.
    Serial.printf("[Alexa] Dispositivo '%s' mudou para %s (value=%u)\n",
                  device_name,
                  state ? "ON" : "OFF",
                  value);
}

void alexa_init(const String &deviceName) {
    g_device_name = deviceName;

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Alexa] WiFi não conectado — adiando inicialização da Alexa");
        return;
    }

    // Porta padrão (80) é a esperada por dispositivos Alexa.
    // Em caso de conflito com outros servidores HTTP, isso pode precisar
    // ser ajustado ou integrado via servidor externo.
    g_fauxmo.setPort(80);
    g_fauxmo.enable(true);

    g_fauxmo.addDevice(g_device_name.c_str());
    g_fauxmo.addDevice((g_device_name + " relatório").c_str());

    g_fauxmo.onSetState(handle_alexa_state);

    g_alexa_enabled = true;

    Serial.printf("[Alexa] fauxmoESP inicializado para '%s'\n",
                  g_device_name.c_str());
}

void alexa_speak(const char *text) {
    if (!g_alexa_enabled || !text) {
        return;
    }

    // fauxmoESP em si não faz TTS; esta função é um ponto central para
    // integrar com automações no Alexa (ex.: rotinas que disparam
    // quando um dispositivo virtual muda de estado). Aqui, apenas
    // registramos a mensagem.
    Serial.printf("[Alexa] %s\n", text);

    // Mantém o fauxmo processando requests.
    g_fauxmo.handle();
}

void alexa_send_status(const char *status) {
    if (!g_alexa_enabled || !status) {
        return;
    }

    Serial.printf("[Alexa] Status: %s\n", status);
    g_fauxmo.handle();
}