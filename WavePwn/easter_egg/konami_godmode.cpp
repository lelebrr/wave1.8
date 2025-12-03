#include <Arduino.h>
#include <SD.h>
#include <lvgl.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "pwnagotchi.h"
#include "ui.h"

// Indica se o modo laboratorio esta ativo.
bool lab_mode = false;

// Forward da instancia global (definida em WavePwn.ino).
extern Pwnagotchi pwn;

// Condicao auxiliar: toque longo no olho esquerdo detectado pela UI.

// Easter Egg final do WavePwn.
//
// So ativa se:
// 1. Codigo Konami digitado (gatilho vem da ui.cpp)
// 2. Toque de 5 segundos no olho esquerdo
// 3. Bateria > 80%
// 4. Cartao SD tem arquivo secreto /sd/.wavepwn_master
void konami_godmode() {
    // Verifica bateria
    if (pwn.battery_percent <= 80.0f) {
        Serial.println("[EASTER] Bateria insuficiente para GODMODE (precisa > 80%)");
        return;
    }

    // Verifica toque longo no olho esquerdo
    if (!ui_eye_left_longpress_reached()) {
        Serial.println("[EASTER] Toque de 5s no olho esquerdo ainda nao detectado");
        return;
    }

    // Verifica arquivo secreto no microSD (sem re-inicializar SD)
    if (!SD.begin()) {
        Serial.println("[EASTER] SD nao inicializado - GODMODE abortado");
        ui_reset_eye_left_longpress_flag();
        return;
    }

    if (!SD.exists("/sd/.wavepwn_master")) {
        Serial.println("[EASTER] Arquivo secreto /sd/.wavepwn_master ausente");
        ui_reset_eye_left_longpress_flag();
        return;
    }

    ui_reset_eye_left_longpress_flag();

    ui_set_mood(MOOD_GODMODE);

    lv_obj_t *scr = lv_scr_act();
    if (scr) {
        lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

        lv_obj_t *label1 = lv_label_create(scr);
        lv_label_set_text(label1, "MESTRE");
        lv_obj_set_style_text_color(label1, lv_color_hex(0x00FFFF), 0);
        lv_obj_set_style_text_font(label1, LV_FONT_DEFAULT, 0);
        lv_obj_align(label1, LV_ALIGN_TOP_LEFT, 20, 100);

        lv_obj_t *label2 = lv_label_create(scr);
        lv_label_set_text(label2, "RECONHECIDO");
        lv_obj_set_style_text_color(label2, lv_color_hex(0x00FFFF), 0);
        lv_obj_set_style_text_font(label2, LV_FONT_DEFAULT, 0);
        lv_obj_align(label2, LV_ALIGN_TOP_LEFT, 20, 150);
    }

    vTaskDelay(pdMS_TO_TICKS(3000));

    lab_mode = true;
    Serial.println("[EASTER] LAB MODE ATIVADO - logs e experimentos protegidos");
}