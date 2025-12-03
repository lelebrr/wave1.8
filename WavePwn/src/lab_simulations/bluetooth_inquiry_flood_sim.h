/*
  bluetooth_inquiry_flood_sim.h - Wrapper para simulação de "inquiry flood" BT

  A lógica de simulação está em SimulationManager::bluetooth_inquiry_flood_sim(),
  que modela apenas a ideia de muitas consultas/inquiries em um curto período,
  sem enviar qualquer pacote de descoberta real.

  Útil para aulas sobre saturação de processos de descoberta Bluetooth.
*/

#pragma once

#include "simulation_manager.h"

inline void bluetooth_inquiry_flood_sim_run(int duration_seconds = 10) {
  SimulationManager::bluetooth_inquiry_flood_sim(duration_seconds);
}