/*
  wifi_karma_attack_sim.h - Wrapper para simulação acadêmica de "KARMA / MANA"

  A lógica de simulação está em SimulationManager::karma_attack_sim(), que:
  - Nunca responde a probes reais.
  - Apenas registra o cenário sintético e aciona banners de SIMULAÇÃO.

  Use rótulos genéricos de SSID para fins de laboratório (ex.: "LAB_WIFI_*").
*/

#pragma once

#include "simulation_manager.h"
#include <stdint.h>

inline void wifi_karma_attack_sim_run(const char* ssid_pattern,
                                      uint8_t channel) {
  SimulationManager::karma_attack_sim(ssid_pattern, channel);
}