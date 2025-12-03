#include <Arduino.h>
#include "pwnagotchi.h"

Pwnagotchi pwn;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n=== WAVE PWN v2.0 - O RENASCIMENTO ===");
  pwn.begin();
}

void loop() {
  pwn.update();
}