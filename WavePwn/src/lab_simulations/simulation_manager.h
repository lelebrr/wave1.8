/*
  simulation_manager.h - Controle central de simulações acadêmicas de ataques

  IMPORTANTE:
  - Este módulo NÃO implementa ataques reais.
  - Todas as funções aqui expostas são pensadas para uso didático e em
    laboratório controlado, com autorização explícita.
  - A presença do arquivo /sd/.enable_lab_attacks é tratada como confirmação
    de que o operador leu ATTACK_SIMULATION.md e aceitou os termos.
*/

#pragma once

#include <stdint.h>
#include <SD.h>

class SimulationManager {
public:
  // Verifica se o modo de laboratório está habilitado.
  // Requer:
  // - microSD inicializado (Pwnagotchi::initSD()).
  // - Arquivo /sd/.enable_lab_attacks presente.
  // - PIN de laboratório desbloqueado na sessão atual.
  static bool is_lab_mode_enabled();

  // Marca o PIN de laboratório como desbloqueado na sessão atual.
  // Usado pelo painel web após validação do PIN de 6 dígitos.
  static void set_lab_unlocked(bool unlocked);
  static bool is_lab_unlocked();

  // Simulações acadêmicas (NÃO enviam pacotes reais).
  // Todas as rotinas devem:
  // - Checar is_lab_mode_enabled().
  // - Registrar logs em /sd/lab_logs/.
  // - Exibir na UI um banner claro de "SIMULAÇÃO".

  // Wi-Fi
  static void deauth_burst_sim(uint8_t channel, int packets = 50);
  static void evil_twin_sim(const char* ssid, uint8_t channel);
  static void beacon_spam_sim(int count = 100);
  static void rogue_ap_sim();
  static void pmkid_flood_sim(const char* ap_label, int frames = 50);

  // Ataques Wi-Fi avançados (sempre como SIMULAÇÃO)
  static void wps_attack_sim(const uint8_t* ap_mac, uint8_t channel);
  static void karma_attack_sim(const char* ssid_pattern, uint8_t channel);
  static void handshake_downgrade_sim(const char* ap_label,
                                      bool from_wpa3_to_wpa2);

  // Bluetooth
  static void bluetooth_spam_sim(const char* profile_name);
  static void bluetooth_jammer_sim(int duration_seconds = 10);
  static void bluetooth_inquiry_flood_sim(int duration_seconds = 10);
  static void bluetooth_hid_injection_sim(const char* payload);
};