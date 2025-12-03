/*
  wifi_pmkid_flood_sim.h - Wrapper para simulação acadêmica de "PMKID flood"

  A lógica de simulação está em SimulationManager::pmkid_flood_sim(), que
  modela apenas o cenário em logs e UI, sem gerar frames PMKID reais.

  Use rótulos genéricos (ex.: "LAB_AP_1") para fins didáticos.
*/

#pragma once

#include "simulation_manager.h"

inline void wifi_pmkid_flood_sim_run(const char* ap_label,
                                     int frames = 50) {
  SimulationManager::pmkid_flood_sim(ap_label, frames);
}