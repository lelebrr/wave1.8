#pragma once

#include <stdint.h>

// PwnGrid - rede BLE cooperativa entre WavePwns amigos.
// 100% defensivo: apenas presença e nível de ameaça compartilhados.
class PwnGrid {
public:
    void begin();
    void advertise_presence();                 // "Oi, sou um WavePwn amigo"
    void share_threat_level(uint8_t level);    // outros WavePwns mostram alerta
    void on_friend_detected(const char* name); // callback simples de amizade

private:
    uint8_t current_level = 0;
};

// Instância global usada pelo firmware.
extern PwnGrid pwnGrid;