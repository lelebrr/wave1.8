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
  // Implícito: Pwnagotchi::initSD() já inicializou o microSD.
  static bool is_lab_mode_enabled() {
    return SD.exists("/sd/.enable_lab_attacks");
  }

  // Simulações acadêmicas (NÃO enviam pacotes reais).
  // Todas as rotinas devem:
  // - Checar is_lab_mode_enabled().
  // - Registrar logs em /sd/lab_logs/.
  // - Exibir na UI um banner claro de "SIMULAÇÃO".
  static void deauth_burst_sim(uint8_t channel, int packets = 50);
  static void evil_twin_sim(const char* ssid, uint8_t channel);
  static void beacon_spam_sim(int count = 100);
  static void rogue_ap_sim();
  static void nfc_replay_sim(const char* profile_name);
  static void bluetooth_spam_sim(const char* profile_name);
};