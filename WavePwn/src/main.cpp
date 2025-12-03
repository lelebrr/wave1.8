#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pwnagotchi.h"

Pwnagotchi pwn;

void setup() {
  Serial.begin(115200);
  vTaskDelay(pdMS_TO_TICKS(1000));
  Serial.println("\n\n=== WAVE PWN v2.0 - O RENASCIMENTO ===");
  pwn.begin();
}

void loop() {
  pwn.update();
}