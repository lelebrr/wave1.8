/*
  WavePwn - Pwnagotchi para ESP32-S3 Touch AMOLED 1.8"
  Baseado no repositório Wave + Minigotchi + ESP32Marauder
  Autor: [seu nome ou lelebrr + você]
  Licença: GPL-3.0
*/

#include <Arduino.h>
#include "config.h"
#include "pwnagotchi.h"

Pwnagotchi pwnagotchi;

void setup() {
  Serial.begin(115200);
  while (!Serial); delay(500);
  Serial.println("\n=== WavePwn Iniciando ===");

  pwnagotchi.begin();
}

void loop() {
  pwnagotchi.update();
}