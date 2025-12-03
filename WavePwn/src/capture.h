#pragma once

#include <Arduino.h>
#include <cstdint>
#include <SD.h>

struct Handshake {
    uint8_t ap[6];
    uint8_t sta[6];
    uint8_t snonce[32];
    uint8_t anonce[32];
    uint8_t mic[16];
    uint8_t eapol[256];
    uint16_t eapol_size;
    uint8_t key_version;
    uint8_t message_num;
    uint64_t timestamp; // milliseconds since boot
};

struct PMKID {
    uint8_t pmkid[16];
    uint8_t ap[6];
    uint8_t sta[6];
    char ssid[33];
    uint64_t timestamp; // milliseconds since boot
};

// Inicialização do subsistema de captura (abre primeiro PCAP, zera deduplicação)
void capture_init();

// Handler chamado pelo callback promíscuo do Wi-Fi
void capture_packet_handler(uint8_t* buf, uint16_t len, uint8_t channel);

// Escrita raw de um frame 802.11 em PCAP (com cabeçalho por pacote)
void capture_write_pcap(const uint8_t* data, uint32_t len, uint32_t channel);

// Persistência de estruturas já parseadas
bool capture_save_handshake(const Handshake* hs);
bool capture_save_pmkid(const PMKID* pmkid);

// Deduplicação em RAM (AP+STA)
bool capture_is_duplicate(const uint8_t* ap, const uint8_t* sta);

// Rotação de arquivos PCAP quando ultrapassar o limite
void capture_rotate_files();

// Caminho do PCAP atual em uso
String capture_get_current_pcap();