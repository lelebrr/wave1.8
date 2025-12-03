/*
  bluetooth_hid_injection_sim.h - Wrapper para simulação de HID injection

  A lógica de simulação está em SimulationManager::bluetooth_hid_injection_sim(), que:
  - Não cria teclados Bluetooth reais.
  - Não envia comandos para hosts reais.
  - Apenas registra o payload sintético para fins didáticos.

  Use comandos fictícios (ex.: "demo_command") para ilustrar o conceito
  sem afetar máquinas reais.
*/

#pragma once

#include "simulation_manager.h"

inline void bluetooth_hid_injection_sim_run(const char* payload) {
  SimulationManager::bluetooth_hid_injection_sim(payload);
}