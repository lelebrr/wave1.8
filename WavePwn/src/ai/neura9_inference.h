#pragma once

#include <stdint.h>
#include <stddef.h>
#include <Arduino.h>

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

// Encapsula a IA defensiva local NEURA9.
class Neura9 {
public:
    // Inicializa TensorFlow Lite Micro e o modelo em RAM/PSRAM.
    bool begin();

    // Executa inferência com base nas features atuais e retorna a classe 0-9.
    uint8_t predict();

    // Hook para ajustes leves a partir do ambiente (pode registrar eventos,
    // salvar estatísticas, etc. – implementação futura).
    void update_from_environment();

    // Confiança (0.0–1.0) da última predição.
    float get_confidence() const;

private:
    tflite::MicroInterpreter* interpreter = nullptr;
    TfLiteTensor* input = nullptr;
    TfLiteTensor* output = nullptr;

    // Arena dedicada à TFLM (alocada em PSRAM quando disponível).
    uint8_t tensor_arena[160 * 1024];

    // Vetor de entrada com até 72 features normalizadas.
    float features[72];

    float last_confidence = 0.0f;

    void extract_features();
    void gesture_detection();
    void battery_prediction();
    void threat_detection();
};

// Labels textuais das classes de risco/perfil de ambiente.
extern const char* NEURA9_THREAT_LABELS[10];

// Instância global utilizada pelo restante do firmware.
extern Neura9 neura9;