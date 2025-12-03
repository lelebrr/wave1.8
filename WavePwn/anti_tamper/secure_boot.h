#pragma once

#include <Arduino.h>
#include <esp_system.h>
#include <esp_err.h>
#include <esp_secure_boot.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Verificação simples de secure boot / anti‑tamper.
//
// Em builds de desenvolvimento (sem secure boot habilitado) a função
// apenas registra no Serial. Quando CONFIG_SECURE_BOOT estiver ativo,
// qualquer falha na verificação trava o dispositivo em um loop infinito.
inline void anti_tamper_check() {
#ifdef CONFIG_SECURE_BOOT
    esp_err_t res = esp_secure_boot_verify_signature();
    if (res != ESP_OK) {
        Serial.println("[SECURE] TAMPER DETECTADO - travando dispositivo");
        while (true) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    } else {
        Serial.println("[SECURE] Secure boot OK");
    }
#else
    Serial.println("[SECURE] Secure boot nao habilitado (build de desenvolvimento)");
#endif
}