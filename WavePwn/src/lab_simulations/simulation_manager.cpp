/*
  simulation_manager.cpp - Implementação das simulações acadêmicas

  AVISO IMPORTANTE:
  - Nenhuma função aqui envia frames 802.11, comandos Bluetooth ou NFC reais.
  - Tudo é tratado como "cenário sintético" para fins didáticos.
  - O modo laboratório só é considerado ativo quando o arquivo
    /sd/.enable_lab_attacks existe no microSD.
*/

#include <Arduino.h>
#include "simulation_manager.h"
#include "ui.h"

static void lab_ensure_log_dir() {
  if (!SD.exists("/sd/lab_logs")) {
    SD.mkdir("/sd/lab_logs");
  }
}

static void lab_log_event(const char* tag, const char* details) {
  lab_ensure_log_dir();

  char path[64];
  snprintf(path, sizeof(path), "/sd/lab_logs/%s.log", tag ? tag : "lab_sim");

  File f = SD.open(path, FILE_APPEND);
  if (!f) {
    Serial.printf("[LAB][SIM] Falha ao abrir log %s\n", path);
    return;
  }

  unsigned long t = millis() / 1000;
  f.printf("%lu;%s;%s\n",
           (unsigned long)t,
           tag ? tag : "event",
           details ? details : "");
  f.close();
}

static lv_obj_t* s_sim_banner = nullptr;

static void lab_show_banner(const char* text) {
  lv_obj_t* scr = lv_scr_act();
  if (!scr) return;

  if (!s_sim_banner) {
    s_sim_banner = lv_label_create(scr);
    lv_obj_set_width(s_sim_banner, lv_pct(100));
    lv_obj_set_style_text_align(s_sim_banner, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(s_sim_banner, lv_color_hex(0xFF0000), 0);
    lv_obj_align(s_sim_banner, LV_ALIGN_TOP_MID, 0, 8);
  }

  if (text) {
    lv_label_set_text(s_sim_banner, text);
  } else {
    lv_label_set_text(s_sim_banner, "SIMULACAO LABORATORIAL");
  }
}

static void lab_warn_mode_off(const char* feature) {
  Serial.printf(
      "[LAB][SIM] Modo laboratorio DESATIVADO - simulacao '%s' bloqueada. "
      "Crie /sd/.enable_lab_attacks apos ler ATTACK_SIMULATION.md\n",
      feature ? feature : "?");
  ui_set_mood(MOOD_SAD);
  lab_show_banner("LAB MODE OFF - LEIA ATTACK_SIMULATION.MD");
  lab_log_event("lab_mode_off", feature ? feature : "?");
}

// ---------------------------------------------------------------------------
// Implementações públicas de SimulationManager
// ---------------------------------------------------------------------------

void SimulationManager::deauth_burst_sim(uint8_t channel, int packets) {
  if (!is_lab_mode_enabled()) {
    lab_warn_mode_off("deauth_burst_sim");
    return;
  }

  Serial.printf(
      "[LAB][SIM] Deauth burst (SIMULACAO) no canal %u, pacotes=%d "
      "(nenhuma transmissao real)\n",
      channel,
      packets);

  ui_set_mood(MOOD_ANGRY);
  lab_show_banner("SIMULACAO DE DEAUTH (SEM TRANSMISSAO)");

  char details[96];
  snprintf(details, sizeof(details), "channel=%u;packets=%d", channel, packets);
  lab_log_event("deauth_burst_sim", details);
}

void SimulationManager::evil_twin_sim(const char* ssid, uint8_t channel) {
  if (!is_lab_mode_enabled()) {
    lab_warn_mode_off("evil_twin_sim");
    return;
  }

  const char* safe_ssid = ssid ? ssid : "(null)";

  Serial.printf(
      "[LAB][SIM] Evil Twin (SIMULACAO) SSID='%s' canal=%u "
      "(nenhum AP falso real criado)\n",
      safe_ssid,
      channel);

  ui_set_mood(MOOD_SNEAKY);
  lab_show_banner("SIMULACAO EVIL TWIN (SEM AP REAL)");

  char details[128];
  snprintf(details, sizeof(details), "ssid=%s;channel=%u", safe_ssid, channel);
  lab_log_event("evil_twin_sim", details);
}

void SimulationManager::beacon_spam_sim(int count) {
  if (!is_lab_mode_enabled()) {
    lab_warn_mode_off("beacon_spam_sim");
    return;
  }

  Serial.printf(
      "[LAB][SIM] Beacon spam (SIMULACAO) count=%d "
      "(nenhum beacon falso enviado)\n",
      count);

  ui_set_mood(MOOD_SCANNING);
  lab_show_banner("SIMULACAO BEACON SPAM (SEM FRAMES)");

  char details[64];
  snprintf(details, sizeof(details), "count=%d", count);
  lab_log_event("beacon_spam_sim", details);
}

void SimulationManager::rogue_ap_sim() {
  if (!is_lab_mode_enabled()) {
    lab_warn_mode_off("rogue_ap_sim");
    return;
  }

  Serial.println(
      "[LAB][SIM] Rogue AP (SIMULACAO) - apenas logs e UI, nenhum AP malicioso real");

  ui_set_mood(MOOD_WIFI_OFF);
  lab_show_banner("SIMULACAO ROGUE AP (SEM AP REAL)");

  lab_log_event("rogue_ap_sim", "simulacao_rogue_ap");
}

void SimulationManager::nfc_replay_sim(const char* profile_name) {
  if (!is_lab_mode_enabled()) {
    lab_warn_mode_off("nfc_replay_sim");
    return;
  }

  const char* safe_profile = profile_name ? profile_name : "(null)";

  Serial.printf(
      "[LAB][SIM] NFC replay (SIMULACAO) perfil='%s' "
      "(nenhum comando NFC real enviado)\n",
      safe_profile);

  ui_set_mood(MOOD_COOL);
  lab_show_banner("SIMULACAO NFC (SEM REPLAY REAL)");

  char details[128];
  snprintf(details, sizeof(details), "profile=%s", safe_profile);
  lab_log_event("nfc_replay_sim", details);
}

void SimulationManager::bluetooth_spam_sim(const char* profile_name) {
  if (!is_lab_mode_enabled()) {
    lab_warn_mode_off("bluetooth_spam_sim");
    return;
  }

  const char* safe_profile = profile_name ? profile_name : "(null)";

  Serial.printf(
      "[LAB][SIM] Bluetooth spam (SIMULACAO) perfil='%s' "
      "(nenhuma conexao BLE real criada)\n",
      safe_profile);

  ui_set_mood(MOOD_EXCITED);
  lab_show_banner("SIMULACAO BLUETOOTH (SEM SPAM REAL)");

  char details[128];
  snprintf(details, sizeof(details), "profile=%s", safe_profile);
  lab_log_event("bluetooth_spam_sim", details);
}