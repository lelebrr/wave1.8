/*
  ui.cpp - Implementação da interface LVGL do WavePwn
*/

#include "ui.h"

static lv_obj_t *label_boot = nullptr;

void ui_init(void) {
    lv_obj_t *scr = lv_scr_act();

    label_boot = lv_label_create(scr);
    lv_label_set_text(label_boot, "WAVE PWN v2.0");
    lv_obj_center(label_boot);
}