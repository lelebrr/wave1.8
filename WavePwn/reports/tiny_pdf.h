#pragma once

#include <Arduino.h>
#include <SD.h>

#include "pwnagotchi.h"

// Contador global de ameacas detectadas pela NEURA9.
// Definido em pwnagotchi.cpp.
extern uint32_t threat_count;

// Instancia global do WavePwn (declarada em WavePwn.ino).
extern Pwnagotchi pwn;

// Implementacao minimalista de gerador de \"PDF\".
// Para manter o firmware leve, esta versao salva um relatorio em texto
// simples com extensao .pdf. Em leitores modernos isso ainda pode ser
// aberto, e a estrutura permite evoluir depois para um writer PDF real.
class PDF {
public:
    PDF() = default;

    void addPage() {
        buffer = "";
    }

    void setFont(const char* name, int size) {
        (void)name;
        (void)size;
    }

    void text(int x, int y, const String& line) {
        (void)x;
        (void)y;
        buffer += line;
        buffer += "\n";
    }

    bool save(const String& fullPath) {
        File f = SD.open(fullPath.c_str(), FILE_WRITE);
        if (!f) {
            Serial.printf("[PDF] Falha ao abrir %s\n", fullPath.c_str());
            return false;
        }

        f.println("WAVE PWN v2 - Relatorio de Seguranca");
        f.print(buffer);
        f.close();

        Serial.printf("[PDF] Relatorio salvo em %s (formato simplificado)\n",
                      fullPath.c_str());
        return true;
    }

private:
    String buffer;
};

// Gera um relatorio simples com estatisticas atuais do dispositivo.
inline void generate_report() {
    String path = "/sd/reports/relatorio_";
    path += String(millis());
    path += ".pdf";

    PDF pdf;
    pdf.addPage();
    pdf.setFont("Helvetica", 16);
    pdf.text(20, 30, "WAVE PWN v2 - Relatorio de Seguranca");
    pdf.text(20, 50, String("Handshakes capturados: ") + String(pwn.handshakes));
    pdf.text(20, 70, String("Ameacas detectadas: ") + String(threat_count));
    (void)pdf.save(path);
}