#pragma once

// === CONFIGURAÇÕES DO WavePwn ===

// Nome do seu pet
#define PET_NAME                    "WavePwn"

// Canais Wi-Fi para varredura (1-13, use 1,6,11 para menos hop)
const uint8_t WIFI_CHANNELS[] = {1, 6, 11, 2, 7, 3, 8, 4, 9, 5, 10, 12, 13};
#define CHANNEL_COUNT               13
#define CHANNEL_HOP_DELAY           10000  // ms

// Deauth settings
#define DEAUTH_PACKETS_PER_TARGET   15
#define DEAUTH_DELAY_BETWEEN        100   // ms

// Whitelist (SSIDs que NUNCA serão atacados)
const char* WHITELIST[] = {
  "SuaCasa-WiFi",
  "SeuCelular",
  "NOME_DA_SUA_REDE"
};
#define WHITELIST_COUNT             3

// Display
#define SCREEN_TIMEOUT              300000  // 5 min sem toque = sleep
#define BRIGHTNESS_ACTIVE           200
#define BRIGHTNESS_IDLE             50

// Sons
#define ENABLE_SOUND                true
#define SOUND_HANDSHAKE             1000  // Hz
#define SOUND_NEW_AP                800

// Storage
#define LOG_FILE                    "/wavepwn.log"
#define PCAP_FILE                   "/captures/handshakes.pcap"
#define SESSION_FILE                "/session.json"

// Outros
#define INACTIVITY_TIMEOUT          600000  // 10 min sem redes = modo triste