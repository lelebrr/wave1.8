/*
  wifi_wps_attack_sim.h - Wrapper para simulação acadêmica de "WPS PIN brute-force"

  A lógica de simulação está em SimulationManager::wps_attack_sim(), que:
  - Respeita o modo laboratório (/sd/.enable_lab_attacks + PIN de 6 dígitos).
  - Gera apenas logs e indicadores visuais.
  - NÃO envia frames WPS reais.

  Este wrapper ajuda a organizar cenários didáticos envolvendo WPS.
*/

#pragma once

#include "simulation_manager.h"
#include <stdint.h>

inline void wifi_wps_attack_sim_run(const uint8_t* ap_mac, uint8_t channel) {
  SimulationManager::wps_attack_sim(ap_mac, channel);
}