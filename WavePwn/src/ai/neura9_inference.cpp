#include "neura9_inference.h"

#include <cstring>

#include "pwnagotchi.h"
#include "ui.h"
#include "sensors.h"
#include "ai/neura9_defense_model_data.h"

extern Pwnagotchi pwn;

// Instância global
Neura9 neura9;

const char* NEURA9_THREAT_LABELS[10] = {
    "SAFE",
    "CROWDED",
    "OPEN_NETWORK",
    "EVIL_TWIN_RISK",
    "DEAUTH_DETECTED",
    "ROGUE_AP",
    "HIGH_RISK",
    "BATTERY_CRITICAL",
    "GESTURE_COMMAND",
    "LEARNING_MODE"
};

bool Neura9::begin() {
    const tflite::Model* model = tflite::GetModel(neura9_defense_model_tflite);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        Serial.printf("[NEURA9] Versao de schema TFLite incompativel (modelo=%d, esperado=%d)\n",
                      model->version(), TFLITE_SCHEMA_VERSION);
        interpreter = nullptr;
        input = nullptr;
        output = nullptr;
        return false;
    }

    static tflite::MicroMutableOpResolver<8> resolver;
    resolver.AddFullyConnected();
    resolver.AddReshape();
    resolver.AddSoftmax();

    static tflite::MicroInterpreter static_interpreter(
        model,
        resolver,
        tensor_arena,
        sizeof(tensor_arena));

    interpreter = &static_interpreter;

    if (interpreter->AllocateTensors() != kTfLiteOk) {
        Serial.println("[NEURA9] AllocateTensors() falhou");
        interpreter = nullptr;
        input = nullptr;
        output = nullptr;
        return false;
    }

    input = interpreter->input(0);
    output = interpreter->output(0);

    Serial.println("[NEURA9] IA defensiva carregada — placeholder — 100% offline");
    return true;
}

void Neura9::extract_features() {
    // Preenche features com dados básicos do ambiente; o restante fica para
    // integrações futuras (sniffer detalhado, IMU, RTC, etc.).
    int i = 0;

    // Visão geral de redes e capturas
    features[i++] = static_cast<float>(pwn.aps_seen);
    features[i++] = static_cast<float>(pwn.handshakes);
    features[i++] = static_cast<float>(pwn.pmkids);
    features[i++] = static_cast<float>(pwn.deauths);

    // Estado de bateria / energia
    features[i++] = pwn.battery_percent / 100.0f;
    features[i++] = pwn.is_charging ? 1.0f : 0.0f;

    // Movimento / contexto físico
    features[i++] = pwn.is_moving ? 1.0f : 0.0f;

    // Tempo de uso (horas aproximadas desde boot)
    features[i++] = static_cast<float>(pwn.uptime) / 3600.0f;

    // Canal atual (placeholder)
    features[i++] = static_cast<float>(pwn.current_channel);

    // Demais features ficam zeradas até termos fontes reais
    for (; i < static_cast<int>(sizeof(features) / sizeof(features[0])); ++i) {
        features[i] = 0.0f;
    }

    gesture_detection();
    battery_prediction();
    threat_detection();
}

uint8_t Neura9::predict() {
    extract_features();

    if (!interpreter || !input || !output) {
        // Fallback leve baseado em regras simples se TFLM não estiver ativo.
        if (pwn.deauths > 50) {
            ui_set_mood(MOOD_ANGRY);
            last_confidence = 1.0f;
            return 4; // DEAUTH_DETECTED
        }
        last_confidence = 0.0f;
        return 0; // SAFE
    }

    if (input->type != kTfLiteFloat32) {
        Serial.println("[NEURA9] Tipo de tensor de entrada inesperado (esperado float32)");
        last_confidence = 0.0f;
        return 0;
    }

    const int input_len = input->bytes / static_cast<int>(sizeof(float));
    const int copy_len = input_len < 72 ? input_len : 72;
    std::memcpy(input->data.f, features, copy_len * sizeof(float));

    if (interpreter->Invoke() != kTfLiteOk) {
        Serial.println("[NEURA9] Invoke() falhou, usando fallback heuristico");
        if (pwn.deauths > 50) {
            ui_set_mood(MOOD_ANGRY);
            last_confidence = 1.0f;
            return 4;
        }
        last_confidence = 0.0f;
        return 0;
    }

    if (output->type != kTfLiteFloat32) {
        Serial.println("[NEURA9] Tipo de tensor de saída inesperado (esperado float32)");
        last_confidence = 0.0f;
        return 0;
    }

    int num_classes = output->bytes / static_cast<int>(sizeof(float));
    if (num_classes > 10) {
        num_classes = 10;
    }

    uint8_t best = 0;
    float max_conf = output->data.f[0];
    for (int i = 1; i < num_classes; ++i) {
        if (output->data.f[i] > max_conf) {
            max_conf = output->data.f[i];
            best = static_cast<uint8_t>(i);
        }
    }

    last_confidence = max_conf;

    // Reação leve na UI conforme o perfil identificado.
    switch (best) {
        case 0: // SAFE
            ui_set_mood(MOOD_HAPPY);
            break;
        case 1: // CROWDED
            ui_set_mood(MOOD_COOL);
            break;
        case 2: // OPEN_NETWORK
            ui_set_mood(MOOD_SNEAKY);
            break;
        case 3: // EVIL_TWIN_RISK
            ui_set_mood(MOOD_SHOCKED);
            break;
        case 4: // DEAUTH_DETECTED
            ui_set_mood(MOOD_ANGRY);
            break;
        case 5: // ROGUE_AP
            ui_set_mood(MOOD_SHOCKED);
            break;
        case 6: // HIGH_RISK
            ui_set_mood(MOOD_SAD);
            break;
        case 7: // BATTERY_CRITICAL
            ui_set_mood(MOOD_LOWBAT);
            break;
        case 8: // GESTURE_COMMAND
            ui_set_mood(MOOD_EXCITED);
            break;
        case 9: // LEARNING_MODE
            ui_set_mood(MOOD_HAPPY);
            break;
        default:
            break;
    }

    Serial.printf("[NEURA9] classe=%u (%s) conf=%.2f\n",
                  best,
                  NEURA9_THREAT_LABELS[best],
                  max_conf);

    return best;
}

void Neura9::update_from_environment() {
    // Hook para futuras rotinas de aprendizado leve / ajuste de thresholds.
    // Mantido deliberadamente simples para evitar escrita em flash/SD em excesso.
}

float Neura9::get_confidence() const {
    return last_confidence;
}

void Neura9::gesture_detection() {
    // Futuro: ler IMU (QMI8658) para reconhecer gestos como comandos secretos.
    // Nesta etapa, mantemos apenas o placeholder lógico.
}

void Neura9::battery_prediction() {
    // Futuro: usar histórico de consumo para prever drenagem e sugerir modos
    // de economia antes de atingir BATTERY_CRITICAL.
}

void Neura9::threat_detection() {
    // Futuro: agregar estatísticas do sniffer para detectar padrões suspeitos
    // (deauth em massa, APs mal configurados, etc.) e alimentar features.
}