#pragma once

#include <stddef.h>
#include <stdint.h>

// Inicia uma atualização OTA \"segura\". Retorna true se o buffer interno
// foi preparado com sucesso.
bool ota_begin_secure(size_t size);

// Escreve um bloco de firmware recebido via HTTP.
bool ota_write_chunk(const uint8_t *data, size_t len);

// Finaliza a atualização e opcionalmente reinicia o dispositivo.
bool ota_finalize(bool reboot);