#include "neura9_inference.h"

// Este arquivo existe para permitir futura extração de features mais rica
// (por exemplo, estatísticas detalhadas do sniffer, IMU, RTC, etc.).
// No momento, a lógica principal de extração está em Neura9::extract_features()
// dentro de neura9_inference.cpp para manter o código compacto.
//
// Quando o pipeline de dados crescer, as funções auxiliares podem migrar
// para cá, mantendo a classe Neura9 enxuta.