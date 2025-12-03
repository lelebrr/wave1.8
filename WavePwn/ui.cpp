/*
  ui.cpp - Implementação da interface LVGL do WavePwn
  ETAPA 2 DO RENASCIMENTO – rostos, partículas, olhos vivos, braços e menu secreto
*/

#include <Arduino.h>
#include <math.h>
#include "ui.h"
#include "config.h"
#include <esp_sleep.h>
#include <driver/gpio.h>

extern void konami_godmode(void);

WavePwn_UI ui = { 0 };

// -----------------------------------------------------------------------------
// Estado interno
// -----------------------------------------------------------------------------

static lv_timer_t *blink_timer   = nullptr;
static lv_timer_t *particle_timer = nullptr;

static lv_obj_t *eye_left  = nullptr;
static lv_obj_t *eye_right = nullptr;
static lv_obj_t *secret_menu = nullptr;

static bool     eye_left_pressed       = false;
static uint32_t eye_left_press_start   = 0;
static bool     eye_left_longpress_flag = false;

typedef struct {
    float vx;
    float vy;
    float life;
    lv_color_t color;
    bool active;
} ParticleState;

static ParticleState particle_state[40];

static const char *mood_names[] = {
    "BOOT","HAPPY","EXCITED","LOVE","COOL","SNEAKY",
    "BORED","SAD","ANGRY","SHOCKED","SLEEPING","DEAD",
    "HANDSHAKE","PMKID","DEAUTH","SCANNING","LOWBAT",
    "CHARGING","OTA","WIFI_OFF","VICTORY","GODMODE"
};

// -----------------------------------------------------------------------------
// Helpers para criação de layout
// -----------------------------------------------------------------------------

static void create_face(lv_obj_t *parent);
static void create_hud(lv_obj_t *parent);
static void on_touch_event(lv_event_t *e);
static void eye_left_event_cb(lv_event_t *e);
static void blink_timer_cb(lv_timer_t *t);
static void particle_timer_cb(lv_timer_t *t);
static void update_eyes_from_touch(const lv_point_t &p);
static void set_arms_idle(void);
static void set_arms_angry(void);
static void set_arms_victory(void);
static void spawn_particles_burst(uint8_t count,
                                  lv_color_t color,
                                  bool upward,
                                  lv_coord_t spread_x,
                                  lv_coord_t spread_y);
static void apply_mood_visuals(Mood mood);

// -----------------------------------------------------------------------------
// Inicialização principal da UI
// -----------------------------------------------------------------------------

void ui_init(void) {
    memset(&ui, 0, sizeof(ui));

    lv_disp_t *disp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(
        disp,
        lv_color_hex(0x00FFFF),   // cor principal
        lv_color_hex(0xFF00FF),   // cor secundária
        true,                     // modo escuro
        LV_FONT_DEFAULT           // fonte padrão (fonts custom entram depois)
    );
    lv_disp_set_theme(disp, theme);

    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    create_face(scr);
    create_hud(scr);

    // Partículas (40 slots)
    for (int i = 0; i < 40; ++i) {
        ui.particles[i] = lv_obj_create(scr);
        lv_obj_remove_style_all(ui.particles[i]);
        lv_obj_set_size(ui.particles[i], 6, 6);
        lv_obj_set_style_radius(ui.particles[i], LV_RADIUS_CIRCLE, 0);
        lv_obj_add_flag(ui.particles[i], LV_OBJ_FLAG_HIDDEN);
        particle_state[i].active = false;
        particle_state[i].life   = 0.0f;
    }
    ui.particle_count = 0;

    // Eventos de entrada – toque e teclado (Konami)
    lv_obj_add_event_cb(scr, ui_konami_code_handler, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(scr, on_touch_event, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(scr, on_touch_event, LV_EVENT_PRESSING, NULL);

    ui.current_mood       = MOOD_BOOT;
    ui.last_blink         = lv_tick_get();
    ui.eyes_follow_touch  = true;
    ui.last_touch.x       = 0;
    ui.last_touch.y       = 0;

    blink_timer    = lv_timer_create(blink_timer_cb, 3500, NULL);  // piscadas
    particle_timer = lv_timer_create(particle_timer_cb, 16, NULL); // ~60 FPS

    ui_set_mood(MOOD_BOOT);
}

// -----------------------------------------------------------------------------
// Construção do rosto / corpo principal
// -----------------------------------------------------------------------------

static void create_face(lv_obj_t *parent) {
    // Cabeça
    ui.face = lv_obj_create(parent);
    lv_obj_remove_style_all(ui.face);
    lv_obj_set_size(ui.face, 260, 260);
    lv_obj_align(ui.face, LV_ALIGN_CENTER, 0, -20);
    lv_obj_set_style_radius(ui.face, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(ui.face, lv_color_hex(0x101020), 0);
    lv_obj_set_style_bg_opa(ui.face, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(ui.face, 4, 0);
    lv_obj_set_style_border_color(ui.face, lv_color_hex(0x00FFFF), 0);

    // Área dos olhos
    ui.eyes = lv_obj_create(ui.face);
    lv_obj_remove_style_all(ui.eyes);
    lv_obj_set_size(ui.eyes, 220, 80);
    lv_obj_align(ui.eyes, LV_ALIGN_TOP_MID, 0, 40);

    // Olho esquerdo
    eye_left = lv_obj_create(ui.eyes);
    lv_obj_remove_style_all(eye_left);
    lv_obj_set_size(eye_left, 28, 28);
    lv_obj_set_style_radius(eye_left, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(eye_left, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(eye_left, LV_OPA_COVER, 0);
    lv_obj_set_pos(eye_left, 60, 20);

    // Eventos para detectar toque longo (Easter Egg)
    lv_obj_add_event_cb(eye_left, eye_left_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(eye_left, eye_left_event_cb, LV_EVENT_PRESSING, NULL);
    lv_obj_add_event_cb(eye_left, eye_left_event_cb, LV_EVENT_RELEASED, NULL);

    // Pupila esquerda
    lv_obj_t *pupil_l = lv_obj_create(eye_left);
    lv_obj_remove_style_all(pupil_l);
    lv_obj_set_size(pupil_l, 12, 12);
    lv_obj_set_style_radius(pupil_l, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(pupil_l, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(pupil_l, LV_OPA_COVER, 0);
    lv_obj_center(pupil_l);

    // Olho direito
    eye_right = lv_obj_create(ui.eyes);
    lv_obj_remove_style_all(eye_right);
    lv_obj_set_size(eye_right, 28, 28);
    lv_obj_set_style_radius(eye_right, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(eye_right, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(eye_right, LV_OPA_COVER, 0);
    lv_obj_set_pos(eye_right, 130, 20);

    // Pupila direita
    lv_obj_t *pupil_r = lv_obj_create(eye_right);
    lv_obj_remove_style_all(pupil_r);
    lv_obj_set_size(pupil_r, 12, 12);
    lv_obj_set_style_radius(pupil_r, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(pupil_r, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(pupil_r, LV_OPA_COVER, 0);
    lv_obj_center(pupil_r);

    // Boca
    ui.mouth = lv_obj_create(ui.face);
    lv_obj_remove_style_all(ui.mouth);
    lv_obj_set_size(ui.mouth, 120, 50);
    lv_obj_align(ui.mouth, LV_ALIGN_BOTTOM_MID, 0, -40);
    lv_obj_set_style_radius(ui.mouth, 30, 0);
    lv_obj_set_style_bg_opa(ui.mouth, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ui.mouth, 4, 0);
    lv_obj_set_style_border_color(ui.mouth, lv_color_white(), 0);

    // Braço esquerdo
    ui.arms_l = lv_obj_create(parent);
    lv_obj_remove_style_all(ui.arms_l);
    lv_obj_set_size(ui.arms_l, 60, 70);
    lv_obj_set_style_radius(ui.arms_l, 30, 0);
    lv_obj_set_style_bg_color(ui.arms_l, lv_color_hex(0x101020), 0);
    lv_obj_set_style_bg_opa(ui.arms_l, LV_OPA_COVER, 0);
    lv_obj_align(ui.arms_l, LV_ALIGN_CENTER, -120, 40);

    // Braço direito
    ui.arms_r = lv_obj_create(parent);
    lv_obj_remove_style_all(ui.arms_r);
    lv_obj_set_size(ui.arms_r, 60, 70);
    lv_obj_set_style_radius(ui.arms_r, 30, 0);
    lv_obj_set_style_bg_color(ui.arms_r, lv_color_hex(0x101020), 0);
    lv_obj_set_style_bg_opa(ui.arms_r, LV_OPA_COVER, 0);
    lv_obj_align(ui.arms_r, LV_ALIGN_CENTER, 120, 40);

    set_arms_idle();
}

// -----------------------------------------------------------------------------
// HUD: nome, uptime, stats, canal, bateria, sinal
// -----------------------------------------------------------------------------

static void create_hud(lv_obj_t *parent) {
    // Nome do pet
    ui.lbl_name = lv_label_create(parent);
    lv_label_set_recolor(ui.lbl_name, true);
#ifdef PET_NAME
    lv_label_set_text_fmt(ui.lbl_name, "#00ffff %s#", PET_NAME);
#else
    lv_label_set_text(ui.lbl_name, "#00ffff WAVE PWN#");
#endif
    lv_obj_align(ui.lbl_name, LV_ALIGN_TOP_LEFT, 8, 4);

    // Uptime
    ui.lbl_uptime = lv_label_create(parent);
    lv_label_set_text(ui.lbl_uptime, "UP: 0s");
    lv_obj_align(ui.lbl_uptime, LV_ALIGN_TOP_LEFT, 8, 24);

    // Barra de bateria
    ui.bar_battery = lv_bar_create(parent);
    lv_obj_set_size(ui.bar_battery, 80, 10);
    lv_obj_align(ui.bar_battery, LV_ALIGN_TOP_RIGHT, -10, 8);
    lv_bar_set_range(ui.bar_battery, 0, 100);
    lv_bar_set_value(ui.bar_battery, 100, LV_ANIM_OFF);

    // Label de bateria
    ui.lbl_battery = lv_label_create(parent);
    lv_label_set_text(ui.lbl_battery, "100%");
    lv_obj_align_to(ui.lbl_battery, ui.bar_battery, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 2);

    // Barra de sinal Wi-Fi (0–5)
    ui.bar_signal = lv_bar_create(parent);
    lv_obj_set_size(ui.bar_signal, 60, 6);
    lv_obj_align(ui.bar_signal, LV_ALIGN_BOTTOM_RIGHT, -10, -4);
    lv_bar_set_range(ui.bar_signal, 0, 5);
    lv_bar_set_value(ui.bar_signal, 0, LV_ANIM_OFF);

    // Stats gerais
    ui.lbl_stats = lv_label_create(parent);
    lv_label_set_text(ui.lbl_stats, "APS:0  HS:0  PMKID:0  DE:0");
    lv_obj_align(ui.lbl_stats, LV_ALIGN_BOTTOM_LEFT, 8, -20);

    // Canal
    ui.lbl_channel = lv_label_create(parent);
    lv_label_set_text(ui.lbl_channel, "CH:-");
    lv_obj_align(ui.lbl_channel, LV_ALIGN_BOTTOM_LEFT, 8, -4);
}

// -----------------------------------------------------------------------------
// Controle de humor e animações ligadas ao humor
// -----------------------------------------------------------------------------

void ui_set_mood(Mood mood) {
    if (ui.current_mood == mood) return;
    ui.current_mood = mood;

    apply_mood_visuals(mood);

    // Animações específicas por evento
    if (mood == MOOD_HANDSHAKE) {
        ui_celebrate_handshake();
    } else if (mood == MOOD_PMKID) {
        ui_celebrate_pmkid();
    } else if (mood == MOOD_DEAUTH) {
        ui_deauth_rage();
    } else if (mood == MOOD_LOWBAT) {
        ui_low_battery_warning();
    }
}

static void apply_mood_visuals(Mood mood) {
    lv_color_t face_color  = lv_color_hex(0x101020);
    lv_color_t border_color = lv_color_hex(0x00FFFF);
    lv_color_t mouth_color  = lv_color_white();

    switch (mood) {
        case MOOD_BOOT:
            face_color  = lv_color_hex(0x101020);
            border_color = lv_color_hex(0x00FFFF);
            mouth_color  = lv_color_hex(0x00FFFF);
            break;
        case MOOD_HAPPY:
            face_color  = lv_color_hex(0x103030);
            border_color = lv_color_hex(0x00FFAA);
            mouth_color  = lv_color_hex(0xFFFFFF);
            set_arms_idle();
            break;
        case MOOD_EXCITED:
            face_color  = lv_color_hex(0x203050);
            border_color = lv_color_hex(0xFFAA00);
            mouth_color  = lv_color_hex(0xFFFFFF);
            set_arms_victory();
            break;
        case MOOD_LOVE:
            face_color  = lv_color_hex(0x301020);
            border_color = lv_color_hex(0xFF3366);
            mouth_color  = lv_color_hex(0xFF99CC);
            set_arms_victory();
            break;
        case MOOD_COOL:
            face_color  = lv_color_hex(0x001020);
            border_color = lv_color_hex(0x00FFFF);
            mouth_color  = lv_color_hex(0x00FFFF);
            break;
        case MOOD_SNEAKY:
            face_color  = lv_color_hex(0x202020);
            border_color = lv_color_hex(0xFF00FF);
            mouth_color  = lv_color_hex(0xFF00FF);
            break;
        case MOOD_BORED:
            face_color  = lv_color_hex(0x101018);
            border_color = lv_color_hex(0x808080);
            mouth_color  = lv_color_hex(0x808080);
            break;
        case MOOD_SAD:
            face_color  = lv_color_hex(0x081020);
            border_color = lv_color_hex(0x0077FF);
            mouth_color  = lv_color_hex(0x0077FF);
            break;
        case MOOD_ANGRY:
            face_color  = lv_color_hex(0x300000);
            border_color = lv_color_hex(0xFF0000);
            mouth_color  = lv_color_hex(0xFF0000);
            set_arms_angry();
            break;
        case MOOD_SHOCKED:
            face_color  = lv_color_hex(0x302020);
            border_color = lv_color_hex(0xFFFF00);
            mouth_color  = lv_color_hex(0xFFFF00);
            break;
        case MOOD_SLEEPING:
            face_color  = lv_color_hex(0x050510);
            border_color = lv_color_hex(0x404080);
            mouth_color  = lv_color_hex(0x8080FF);
            break;
        case MOOD_DEAD:
            face_color  = lv_color_hex(0x050505);
            border_color = lv_color_hex(0x404040);
            mouth_color  = lv_color_hex(0x606060);
            break;
        case MOOD_HANDSHAKE:
        case MOOD_PMKID:
        case MOOD_DEAUTH:
        case MOOD_SCANNING:
        case MOOD_LOWBAT:
        case MOOD_CHARGING:
        case MOOD_OTA:
        case MOOD_WIFI_OFF:
        case MOOD_VICTORY:
        case MOOD_GODMODE:
            // Para estes, herdamos base feliz e ajustamos nas animações
            face_color  = lv_color_hex(0x103030);
            border_color = lv_color_hex(0x00FFAA);
            mouth_color  = lv_color_hex(0xFFFFFF);
            break;
    }

    if (ui.face) {
        lv_obj_set_style_bg_color(ui.face, face_color, 0);
        lv_obj_set_style_border_color(ui.face, border_color, 0);
    }
    if (ui.mouth) {
        lv_obj_set_style_border_color(ui.mouth, mouth_color, 0);
    }
}

// -----------------------------------------------------------------------------
// Partículas e celebrações
// -----------------------------------------------------------------------------

static void spawn_particles_burst(uint8_t count,
                                  lv_color_t color,
                                  bool upward,
                                  lv_coord_t spread_x,
                                  lv_coord_t spread_y) {
    if (!ui.face) return;
    lv_coord_t cx = lv_obj_get_x(ui.face) + lv_obj_get_width(ui.face) / 2;
    lv_coord_t cy = lv_obj_get_y(ui.face) + lv_obj_get_height(ui.face) / 2;

    uint8_t spawned = 0;
    for (int i = 0; i < 40 && spawned < count; ++i) {
        if (particle_state[i].active) continue;

        lv_obj_clear_flag(ui.particles[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(ui.particles[i],
                       cx + (random(-spread_x, spread_x + 1)),
                       cy + (random(-spread_y, spread_y + 1)));

        particle_state[i].active = true;
        particle_state[i].life   = 0.9f + (random(0, 300) / 1000.0f);
        particle_state[i].color  = color;

        float angle = (float)random(0, 360) * 0.0174533f;
        float speed = upward ? (80.0f + random(0, 60)) : (40.0f + random(0, 40));

        particle_state[i].vx = cosf(angle) * speed;
        particle_state[i].vy = sinf(angle) * speed;
        if (upward && particle_state[i].vy > 0) {
            particle_state[i].vy = -particle_state[i].vy;
        }

        lv_obj_set_style_bg_color(ui.particles[i], color, 0);
        lv_obj_set_style_bg_opa(ui.particles[i], LV_OPA_COVER, 0);

        spawned++;
    }
    ui.particle_count = spawned;
}

void ui_celebrate_handshake(void) {
    // Chuva de corações / faíscas
    spawn_particles_burst(30, lv_color_hex(0xFF3366), true, 40, 40);
    set_arms_victory();
}

void ui_celebrate_pmkid(void) {
    spawn_particles_burst(24, lv_color_hex(0xFFD700), true, 30, 30);
    set_arms_victory();
}

void ui_deauth_rage(void) {
    spawn_particles_burst(26, lv_color_hex(0xFF3300), false, 60, 10);
    set_arms_angry();
}

void ui_low_battery_warning(void) {
    spawn_particles_burst(16, lv_color_hex(0xFFFF00), false, 50, 10);
}

// Timer de partículas (~60 FPS)
static void particle_timer_cb(lv_timer_t *t) {
    (void)t;
    const float dt = 0.016f; // 16 ms

    lv_obj_t *scr = lv_scr_act();
    lv_coord_t h = lv_obj_get_height(scr);

    for (int i = 0; i < 40; ++i) {
        if (!particle_state[i].active) continue;

        particle_state[i].vy += 200.0f * dt; // gravidade leve

        lv_coord_t x = lv_obj_get_x(ui.particles[i]);
        lv_coord_t y = lv_obj_get_y(ui.particles[i]);

        x += (lv_coord_t)(particle_state[i].vx * dt);
        y += (lv_coord_t)(particle_state[i].vy * dt);

        lv_obj_set_pos(ui.particles[i], x, y);

        particle_state[i].life -= dt;
        if (particle_state[i].life <= 0.0f || y > h + 20) {
            particle_state[i].active = false;
            lv_obj_add_flag(ui.particles[i], LV_OBJ_FLAG_HIDDEN);
        } else {
            uint8_t opa = (uint8_t)(particle_state[i].life * 255.0f);
            if (opa > 255) opa = 255;
            lv_obj_set_style_opa(ui.particles[i], opa, 0);
        }
    }
}

// -----------------------------------------------------------------------------
// Braços
// -----------------------------------------------------------------------------

static void set_arms_idle(void) {
    if (ui.arms_l) {
        lv_obj_set_style_transform_angle(ui.arms_l, 10 * 10, 0);
    }
    if (ui.arms_r) {
        lv_obj_set_style_transform_angle(ui.arms_r, -10 * 10, 0);
    }
}

static void set_arms_angry(void) {
    if (ui.arms_l) {
        lv_obj_set_style_transform_angle(ui.arms_l, -40 * 10, 0);
    }
    if (ui.arms_r) {
        lv_obj_set_style_transform_angle(ui.arms_r, 40 * 10, 0);
    }
}

static void set_arms_victory(void) {
    if (ui.arms_l) {
        lv_obj_set_style_transform_angle(ui.arms_l, -70 * 10, 0);
    }
    if (ui.arms_r) {
        lv_obj_set_style_transform_angle(ui.arms_r, 70 * 10, 0);
    }
}

// -----------------------------------------------------------------------------
// Olhos – piscadas e seguir o toque
// -----------------------------------------------------------------------------

static void eyes_opa_exec_cb(void *obj, int32_t v) {
    lv_obj_set_style_opa((lv_obj_t *)obj, (lv_opa_t)v, 0);
}

static void blink_timer_cb(lv_timer_t *t) {
    (void)t;
    if (!ui.eyes) return;
    if (ui.current_mood == MOOD_SLEEPING || ui.current_mood == MOOD_DEAD) return;

    uint32_t now = lv_tick_get();
    if (now - ui.last_blink < 1500) return;
    ui.last_blink = now;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, ui.eyes);
    lv_anim_set_exec_cb(&a, eyes_opa_exec_cb);
    lv_anim_set_values(&a, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_time(&a, 120);
    lv_anim_set_playback_time(&a, 120);
    lv_anim_set_repeat_count(&a, 1);
    lv_anim_start(&a);
}

static void update_eyes_from_touch(const lv_point_t &p) {
    if (!ui.eyes_follow_touch || !eye_left || !eye_right) return;

    lv_obj_t *scr = lv_scr_act();
    lv_coord_t scr_w = lv_obj_get_width(scr);
    lv_coord_t scr_h = lv_obj_get_height(scr);

    // Normaliza ponto de toque para [-1, 1]
    float nx = (2.0f * p.x / (float)scr_w) - 1.0f;
    float ny = (2.0f * p.y / (float)scr_h) - 1.0f;

    const lv_coord_t offset_max = 8;

    lv_coord_t dx = (lv_coord_t)(nx * offset_max);
    lv_coord_t dy = (lv_coord_t)(ny * offset_max);

    lv_obj_set_pos(eye_left,
                   60 + dx,
                   20 + dy);
    lv_obj_set_pos(eye_right,
                   130 + dx,
                   20 + dy);
}

static void on_touch_event(lv_event_t *e) {
    (void)e;
    lv_indev_t *indev = lv_indev_get_act();
    if (!indev) return;

    lv_point_t p;
    lv_indev_get_point(indev, &p);
    ui.last_touch = p;
    update_eyes_from_touch(p);
}

static void eye_left_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_PRESSED) {
        eye_left_pressed = true;
        eye_left_press_start = lv_tick_get();
        eye_left_longpress_flag = false;
    } else if (code == LV_EVENT_PRESSING) {
        if (eye_left_pressed &amp;&amp; !eye_left_longpress_flag) {
            uint32_t now = lv_tick_get();
            if (now - eye_left_press_start &gt;= 5000) {
                eye_left_longpress_flag = true;
            }
        }
    } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        eye_left_pressed = false;
    }
}

bool ui_eye_left_longpress_reached(void) {
    return eye_left_longpress_flag;
}

void ui_reset_eye_left_longpress_flag(void) {
    eye_left_longpress_flag = false;
}

// -----------------------------------------------------------------------------
// Menu secreto - GOD MODE
// -----------------------------------------------------------------------------

static void secret_menu_close_cb(lv_event_t *e);

void ui_show_secret_menu(void) {
    if (secret_menu) {
        lv_obj_del(secret_menu);
        secret_menu = nullptr;
        return;
    }

    lv_obj_t *scr = lv_scr_act();
    secret_menu = lv_obj_create(scr);
    lv_obj_remove_style_all(secret_menu);
    lv_obj_set_size(secret_menu, 240, 260);
    lv_obj_center(secret_menu);
    lv_obj_set_style_bg_color(secret_menu, lv_color_hex(0x050510), 0);
    lv_obj_set_style_bg_opa(secret_menu, LV_OPA_90, 0);
    lv_obj_set_style_radius(secret_menu, 16, 0);

    lv_obj_t *title = lv_label_create(secret_menu);
    lv_label_set_recolor(title, true);
    lv_label_set_text(title, "#ff00ff GOD MODE DESBLOQUEADO#");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);

    lv_obj_t *txt = lv_label_create(secret_menu);
    lv_label_set_text(txt,
                      "Konami code aceito.\\n"
                      "Funcoes futuras aqui:\\n"
                      "- Overclock secreto\\n"
                      "- Deauth insano\\n"
                      "- Tweaks de LVGL\\n"
                      "- Hacks experimentais");
    lv_obj_align(txt, LV_ALIGN_TOP_LEFT, 8, 40);

    lv_obj_t *btn_close = lv_btn_create(secret_menu);
    lv_obj_set_size(btn_close, 120, 32);
    lv_obj_align(btn_close, LV_ALIGN_BOTTOM_MID, 0, -12);

    lv_obj_t *lbl = lv_label_create(btn_close);
    lv_label_set_text(lbl, "Voltar");
    lv_obj_center(lbl);

    lv_obj_add_event_cb(btn_close, secret_menu_close_cb, LV_EVENT_CLICKED, NULL);
}

static void secret_menu_close_cb(lv_event_t *e) {
    (void)e;
    ui_show_secret_menu();
}

// -----------------------------------------------------------------------------
// Atualização de stats e estados derivados
// -----------------------------------------------------------------------------

void ui_update_stats(uint32_t aps,
                     uint32_t hs,
                     uint32_t pmkid,
                     uint32_t deauth,
                     uint8_t channel,
                     float battery,
                     bool moving) {
    if (ui.lbl_stats) {
        lv_label_set_text_fmt(ui.lbl_stats,
                              "APS:%lu  HS:%lu  PMKID:%lu  DE:%lu",
                              (unsigned long)aps,
                              (unsigned long)hs,
                              (unsigned long)pmkid,
                              (unsigned long)deauth);
    }

    if (ui.lbl_channel) {
        if (channel == 0) {
            lv_label_set_text(ui.lbl_channel, "CH:-");
        } else {
            lv_label_set_text_fmt(ui.lbl_channel, "CH:%u", channel);
        }
    }

    if (ui.bar_battery) {
        int val = (int)(battery + 0.5f);
        if (val < 0) val = 0;
        if (val > 100) val = 100;
        lv_bar_set_value(ui.bar_battery, val, LV_ANIM_OFF);
    }

    if (ui.lbl_battery) {
        lv_label_set_text_fmt(ui.lbl_battery, "%.0f%%", battery);
    }

    if (ui.bar_signal) {
        uint8_t strength = 0;
        if      (aps == 0)      strength = 0;
        else if (aps < 5)       strength = 1;
        else if (aps < 10)      strength = 2;
        else if (aps < 20)      strength = 3;
        else if (aps < 40)      strength = 4;
        else                    strength = 5;
        if (strength > 5) strength = 5;
        lv_bar_set_value(ui.bar_signal, strength, LV_ANIM_OFF);
    }

    if (ui.lbl_uptime) {
        lv_label_set_text_fmt(ui.lbl_uptime,
                              "UP: %lus",
                              (unsigned long)(millis() / 1000));
    }

    // Regras simples de humor reativo
    if (battery < 15.0f &&
        ui.current_mood != MOOD_LOWBAT &&
        ui.current_mood != MOOD_CHARGING &&
        ui.current_mood != MOOD_DEAD) {
        ui_set_mood(MOOD_LOWBAT);
    } else if (moving &&
               (ui.current_mood == MOOD_HAPPY ||
                ui.current_mood == MOOD_BORED ||
                ui.current_mood == MOOD_SNEAKY)) {
        ui_set_mood(MOOD_EXCITED);
    } else if (!moving &&
               ui.current_mood == MOOD_EXCITED) {
        ui_set_mood(MOOD_HAPPY);
    }
}

// -----------------------------------------------------------------------------
// Sleep / Wake
// -----------------------------------------------------------------------------

void ui_sleep(void) {
    ui_set_mood(MOOD_SLEEPING);

    // Configura fontes de wake: toque e movimento (IMU)
    esp_sleep_enable_touchpad_wakeup();
    // GPIO0 é placeholder; a linha real da IMU será conectada em sensors.cpp
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);
}

void ui_wake(void) {
    ui_set_mood(MOOD_HAPPY);
}

// -----------------------------------------------------------------------------
// Código Konami - GOD MODE
// -----------------------------------------------------------------------------

void ui_konami_code_handler(lv_event_t *e) {
    if (lv_event_get_code(e) != LV_EVENT_KEY) return;

    static const uint8_t konami[] = {
        LV_KEY_UP, LV_KEY_UP,
        LV_KEY_DOWN, LV_KEY_DOWN,
        LV_KEY_LEFT, LV_KEY_RIGHT,
        LV_KEY_LEFT, LV_KEY_RIGHT,
        'B', 'A'
    };
    static uint8_t step = 0;

    lv_key_t key = lv_event_get_key(e);

    if (key == konami[step]) {
        step++;
        if (step >= sizeof(konami)) {
            step = 0;
            konami_godmode();
        }
    } else {
        step = 0;
    }
}