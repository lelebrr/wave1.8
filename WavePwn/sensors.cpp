/*
  sensors.cpp - Implementação dos sensores (IMU, PMIC, RTC) do WavePwn
*/

#include "sensors.h"
#include <Arduino.h>
#include <esp_sleep.h>
#include <driver/gpio.h>

// Inicializa o wake por movimento usando o IMU (QMI8658) em modo Wake-on-Motion.
// Nesta etapa mantemos um stub seguro, pronto para ser ligado ao driver real.
void init_motion_wakeup(void) {
    Serial.println("[WavePwn] init_motion_wakeup() - stub para QMI8658 Wake-on-Motion");

    // Exemplo de fonte de wake externa compartilhada com a IMU.
    // Quando o pino de interrupção do QMI8658 estiver definido como IMU_INT_PIN,
    // basta descomentar o define em config.h (ou outro header) para ativar:
#ifdef IMU_INT_PIN
    esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(IMU_INT_PIN), 0);
#endif
}