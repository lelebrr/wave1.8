/*
  wifi_deauth_sim.h - Wrapper para simulação acadêmica de "Wi-Fi deauth burst"

  A lógica de simulação está em SimulationManager::deauth_burst_sim(), que:
  - Respeita o modo laboratório (/sd/.enable_lab_attacks).
  - Gera apenas logs e indicadores visuais.
  - NÃO envia frames 802.11 reais.

  Este wrapper existe para facilitar a organização conceitual das
  simulações de Wi-Fi, sem adicionar comportamento ofensivo.
*/

#pragma once

#include "simulation_manager.h"
#include <stdint.h>

inline void wifi_deauth_sim_run(uint8_t channel, int packets = 50) {
  SimulationManager::deauth_burst_sim(channel, packets);
}