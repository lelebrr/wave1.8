/*
  wifi_handshake_downgrade_sim.h - Wrapper para simulação de downgrade WPA

  A lógica de simulação está em SimulationManager::handshake_downgrade_sim(), que:
  - Modela apenas o conceito de "forçar" um cliente a negociar em WPA2.
  - Não altera realmente a cifra, nem envia quadros de negociação reais.

  Útil para ilustrar, em laboratório, por que redes WPA3 mal configuradas
  podem ser vulneráveis a downgrades.
*/

#pragma once

#include "simulation_manager.h"

inline void wifi_handshake_downgrade_sim_run(const char* ap_label,
                                             bool from_wpa3_to_wpa2 = true) {
  SimulationManager::handshake_downgrade_sim(ap_label, from_wpa3_to_wpa2);
}