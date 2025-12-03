#pragma once

#include <stdint.h>

// Interface de modelo utilizada pela NEURA9.
//
// O conteúdo real é gerado offline a partir de um arquivo .tflite
// (ver ai/neura9_trainer.py ou ai_training/export_to_tflite.py) e
// atualmente é definido em ai/neura9_defense_model_data.cpp.
extern const unsigned char neura9_defense_model_tflite[];
extern const unsigned int neura9_defense_model_tflite_len;