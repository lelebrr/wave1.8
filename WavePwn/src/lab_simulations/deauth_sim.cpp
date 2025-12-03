/*
  deauth_sim.cpp - Stub de simulação de deauth

  A implementação prática da simulação acadêmica de "deauth burst" está em
  SimulationManager::deauth_burst_sim(), que:
  - Checa o modo laboratório (/sd/.enable_lab_attacks).
  - Registra logs em /sd/lab_logs/.
  - Exibe um banner de SIMULACAO na UI.

  Este arquivo é mantido como local apropriado para futuras extensões
  específicas da simulação de deauth (ex.: métricas extras, contadores
  de cenário, etc.), sempre sem envio de frames reais.
*/

#include "deauth_sim.h"