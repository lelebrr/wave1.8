/*
  ui.h - Interface LVGL do WavePwn (rostos, stats, menus)
*/

#pragma once

#include <lvgl.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MOOD_BOOT,
    MOOD_HAPPY,
    MOOD_EXCITED,
    MOOD_LOVE,
    MOOD_COOL,
    MOOD_SNEAKY,
    MOOD_BORED,
    MOOD_SAD,
    MOOD_ANGRY,
    MOOD_SHOCKED,
    MOOD_SLEEPING,
    MOOD_DEAD,
    MOOD_HANDSHAKE,
    MOOD_PMKID,
    MOOD_DEAUTH,
    MOOD_SCANNING,
    MOOD_LOWBAT,
    MOOD_CHARGING,
    MOOD_OTA,
    MOOD_WIFI_OFF,
    MOOD_VICTORY,
    MOOD_GODMODE
} Mood;

typedef struct {
    lv_obj_t *face;
    lv_obj_t *eyes;
    lv_obj_t *mouth;
    lv_obj_t *arms_l;
    lv_obj_t *arms_r;

    lv_obj_t *lbl_name;
    lv_obj_t *lbl_uptime;
    lv_obj_t *lbl_stats;
    lv_obj_t *lbl_channel;
    lv_obj_t *lbl_battery;

    lv_obj_t *bar_battery;
    lv_obj_t *bar_signal;

    lv_obj_t *particles[40];
    int       particle_count;

    Mood      current_mood;
    uint32_t  last_blink;
    bool      eyes_follow_touch;
    lv_point_t last_touch;
} WavePwn_UI;

extern WavePwn_UI ui;

void ui_init(void);
void ui_set_mood(Mood mood);
void ui_celebrate_handshake(void);
void ui_celebrate_pmkid(void);
void ui_deauth_rage(void);
void ui_low_battery_warning(void);
void ui_sleep(void);
void ui_wake(void);
void ui_show_secret_menu(void);
void ui_update_stats(uint32_t aps,
                     uint32_t hs,
                     uint32_t pmkid,
                     uint32_t deauth,
                     uint8_t channel,
                     float battery,
                     bool moving);
void ui_konami_code_handler(lv_event_t *e);

#ifdef __cplusplus
}
#endif