# WavePwn - Pwnagotchi para ESP32-S3 AMOLED 1.8"

O primeiro Pwnagotchi 100% nativo para a placa Wave (ESP32-S3 Touch AMOLED 1.8" com microSD).

## Funcionalidades (em desenvolvimento)
- Captura passiva e ativa de handshakes WPA/WPA2 + PMKID
- Interface LVGL com carinhas animadas
- Touch + sensores (movimento ativa modo agressivo)
- Armazenamento em microSD 128 GB
- Deauth + sniffing + epochs
- Whitelist + bateria no display
- Sons de evento

## Como usar
1. Abra a pasta WavePwn no Arduino IDE
2. Selecione: ESP32S3 Dev Module
3. PSRAM: Enabled | Flash Size: 16MB | Partition: Huge App
4. Upload!

Em breve: interface web via Wi-Fi, OTA, Pwngrid BLE.