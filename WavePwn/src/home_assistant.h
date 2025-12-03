#pragma once

#include <stdint.h>

// Integração leve com Home Assistant / Google Home via Home Assistant.
//
// Por padrão, a integração fica desativada (HA_ENABLED = 0). Para ativar,
// defina os macros abaixo em config.h ou via build_flags:
//
//   -DHA_ENABLED=1
//   -DHA_BASE_URL=\"http://homeassistant.local:8123\"
//   -DHA_TOKEN=\"SEU_LONG_LIVED_ACCESS_TOKEN\"\n
//
// Quando ativado, o WavePwn publica:
//   - sensor.wavepwn_threat_level
//   - sensor.wavepwn_battery
//   - binary_sensor.wavepwn_deauth_attack
//
// O Google Home enxerga esses sensores através do Home Assistant.

#ifdef __cplusplus
extern "C" {
#endif

// Inicializa NTP e imprime o status da integração.
void ha_init(void);

// Atualiza timers internos (ex.: NTP) – chame no loop principal.
void ha_loop(void);

// Envia o nível de ameaça atual para o Home Assistant.
// `level` é a string da classe (ex.: "SAFE", "HIGH_RISK", etc.).
void ha_send_threat(const char *level);

#ifdef __cplusplus
}
#endif