#include "neura9_defense_model_data.h"

// Modelo mínimo dummy (não faz predições úteis).
// Serve apenas para que o firmware compile antes do modelo real existir.
const unsigned char neura9_defense_model_tflite[] = {
    0x54, 0x46, 0x4C, 0x33  // 'T','F','L','3' – magic de arquivos .tflite
};
const unsigned int neura9_defense_model_tflite_len = sizeof(neura9_defense_model_tflite);