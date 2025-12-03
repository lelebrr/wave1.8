/*
  wifi_evil_twin_sim.h - Wrapper para simulação acadêmica de "Evil Twin Wi-Fi"

  A lógica de simulação está em SimulationManager::evil_twin_sim(), que:
  - Não cria APs falsos reais.
  - Apenas registra o cenário e aciona banners de SIMULAÇÃO na UI.

  Este header organiza conceitualmente a simulação de Evil Twin em
  ambientes de laboratório.
*/

#pragma once

#include "simulation_manager.h"
#include <stdint.h>

inline void wifi_evil_twin_sim_run(const char* ssid, uint8_t channel) {
  SimulationManager::evil_twin_sim(ssid, channel);
}