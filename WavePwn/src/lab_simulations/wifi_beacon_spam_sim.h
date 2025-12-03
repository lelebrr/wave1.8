/*
  wifi_beacon_spam_sim.h - Wrapper para simulação acadêmica de "beacon spam"

  A lógica de simulação está em SimulationManager::beacon_spam_sim(), que
  não envia beacons falsos reais; apenas registra parâmetros e sinaliza
  visualmente o cenário.

  Este wrapper permite organizar as simulações de beacon spam em Wi-Fi.
*/

#pragma once

#include "simulation_manager.h"

inline void wifi_beacon_spam_sim_run(int count = 100) {
  SimulationManager::beacon_spam_sim(count);
}