/*
  bluetooth_jammer_sim.h - Wrapper para simulação acadêmica de "Bluetooth jammer"

  A lógica de simulação está em SimulationManager::bluetooth_jammer_sim(), que:
  - Não altera canais de rádio nem gera interferência real.
  - Apenas registra parâmetros e exibe banners de SIMULAÇÃO.

  Serve para demonstrar, em laboratório, o conceito de saturação de espectro
  sem causar qualquer efeito em dispositivos reais.
*/

#pragma once

#include "simulation_manager.h"

inline void bluetooth_jammer_sim_run(int duration_seconds = 10) {
  SimulationManager::bluetooth_jammer_sim(duration_seconds);
}