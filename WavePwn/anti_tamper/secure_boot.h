#pragma once

#include &lt;Arduino.h&gt;
#include &lt;esp_system.h&gt;
#include &lt;esp_err.h&gt;
#include &lt;esp_secure_boot.h&gt;

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
            delay(1000);
        }
    } else {
        Serial.println("[SECURE] Secure boot OK");
    }
#else
    Serial.println("[SECURE] Secure boot nao habilitado (build de desenvolvimento)");
#endif
}