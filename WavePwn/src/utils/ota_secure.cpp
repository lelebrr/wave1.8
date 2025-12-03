#include "utils/ota_secure.h"

#include <Arduino.h>
#include <Update.h>

// Pequena camada em torno da API Update, para permitir futuros checks
// de integridade (hash, assinatura, etc.). Nesta etapa, o foco é manter
// o código organizado e pronto para endurecimento adicional.

bool ota_begin_secure(size_t size) {
    // No futuro, poderíamos validar tamanho máximo, versão, etc.
    if (!Update.begin(size)) {
        Update.printError(Serial);
        return false;
    }
    return true;
}

bool ota_write_chunk(const uint8_t *data, size_t len) {
    if (Update.write(data, len) != len) {
        Update.printError(Serial);
        return false;
    }
    return true;
}

bool ota_finalize(bool reboot) {
    if (!Update.end(reboot)) {
        Update.printError(Serial);
        return false;
    }
    return true;
}