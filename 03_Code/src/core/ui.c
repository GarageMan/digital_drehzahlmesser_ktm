/*
 * UI-Aufbau und -Update fuer beide Screens (Drehzahlmesser / Analoguhr).
 *
 * Bewusst reines C + LVGL-v9-API (kein Arduino, kein C++), damit dieselbe
 * Datei unveraendert im ESP32-Arduino-Sketch mit eingebunden werden kann
 * (Arduino kompiliert .c-Dateien im selben Projekt anstandslos mit).
 *
 * Auf v9 portiert (Nachtrag, siehe Sprint3_Code_Grundgeruest.md Abschnitt
 * "LVGL-Versionswechsel"): das reale CrowPanel-Board nutzt nachweislich
 * LVGL 9.1.0 (Elecrow-eigenes Beispielprojekt), nicht die urspruenglich in
 * Sprint 3 dokumentierte 8.3.11 - PC-Simulator und core/ui.c wurden
 * entsprechend nachgezogen, um die "1:1 Code-Uebernahme PC->ESP32" wieder
 * herzustellen.
 *
 * Zeigerwinkel-Konvention (siehe Sprint-1-Doku): 0 Zehntelgrad = 3-Uhr-
 * Position, im Uhrzeigersinn steigend. Die Nadel-PNGs sind so gezeichnet,
 * dass sie bei Winkel 0 exakt nach oben (12 Uhr) zeigen, mit Drehpunkt
 * bei (240,240) - siehe img_*_needle.h.
 */
#include <stdio.h>
#include "ui.h"
#include "lvgl/lvgl.h"
#include "img_rpm_dial.h"
#include "img_rpm_needle.h"
#include "img_clock_dial.h"
#include "img_hour_needle.h"
#include "img_min_needle.h"

/* --- Drehzahlmesser-Geometrie ---
 * Zifferblatt: 0..9 (x1000 U/min) gleichmaessig ueber einen 240-Grad-Bogen
 * verteilt, von der 8-Uhr- bis zur 4-Uhr-Position (siehe rpm_dial.png).
 * Die Nadel darf bis 9500 U/min etwas ueber die "9" hinaus in den
 * markierten Grenzbereich schwingen. */
#define RPM_START_ANGLE_TENTHS 1500 /* 150.0 deg = 8-Uhr-Position */
#define RPM_SWEEP_TENTHS       2400 /* 240.0 deg Gesamtbogen, entspricht 0..9 */
#define RPM_LABEL_MAX          9000 /* "9" auf dem Zifferblatt */
#define RPM_NEEDLE_CLAMP       9500 /* mechanischer/optischer Anschlag */
#define RPM_SHIFT_BLINK        8900 /* ab hier 5Hz-Blinken (siehe Sprint 1) */
#define SHIFT_BLINK_HALF_MS    100  /* 100ms an / 100ms aus = 5Hz */

static lv_obj_t *scr_rpm;
static lv_obj_t *scr_clock;

static lv_obj_t *rpm_needle_img;
static lv_obj_t *shift_flash;

static lv_obj_t *hour_needle_img;
static lv_obj_t *min_needle_img;

static uint32_t shift_blink_acc_ms;
static bool shift_blink_on;

static void screen_click_cb(lv_event_t *e)
{
    (void)e;
    if(lv_screen_active() == scr_rpm) {
        lv_screen_load_anim(scr_clock, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);
        printf("[ui] Touch/Klick erkannt -> Umschaltung auf Analoguhr\n");
    }
    else {
        lv_screen_load_anim(scr_rpm, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);
        printf("[ui] Touch/Klick erkannt -> Umschaltung auf Drehzahlmesser\n");
    }
}

static lv_obj_t *build_rpm_screen(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(scr, screen_click_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *dial = lv_image_create(scr);
    lv_image_set_src(dial, &img_rpm_dial);
    lv_obj_center(dial);
    lv_obj_remove_flag(dial, LV_OBJ_FLAG_CLICKABLE); /* Klicks durchreichen */

    rpm_needle_img = lv_image_create(scr);
    lv_image_set_src(rpm_needle_img, &img_rpm_needle);
    lv_obj_center(rpm_needle_img);
    lv_image_set_pivot(rpm_needle_img, 240, 240);
    lv_obj_remove_flag(rpm_needle_img, LV_OBJ_FLAG_CLICKABLE);

    /* Shift-Light-Flash: halbtransparente rote Flaeche, die beim
     * Erreichen von RPM_SHIFT_BLINK mit 5Hz ein-/ausgeblendet wird. */
    shift_flash = lv_obj_create(scr);
    lv_obj_remove_style_all(shift_flash);
    lv_obj_set_size(shift_flash, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(shift_flash, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_bg_opa(shift_flash, LV_OPA_40, 0);
    lv_obj_remove_flag(shift_flash, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(shift_flash, LV_OBJ_FLAG_HIDDEN);

    return scr;
}

static lv_obj_t *build_clock_screen(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(scr, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(scr, screen_click_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *dial = lv_image_create(scr);
    lv_image_set_src(dial, &img_clock_dial);
    lv_obj_center(dial);
    lv_obj_remove_flag(dial, LV_OBJ_FLAG_CLICKABLE);

    hour_needle_img = lv_image_create(scr);
    lv_image_set_src(hour_needle_img, &img_hour_needle);
    lv_obj_center(hour_needle_img);
    lv_image_set_pivot(hour_needle_img, 240, 240);
    lv_obj_remove_flag(hour_needle_img, LV_OBJ_FLAG_CLICKABLE);

    min_needle_img = lv_image_create(scr);
    lv_image_set_src(min_needle_img, &img_min_needle);
    lv_obj_center(min_needle_img);
    lv_image_set_pivot(min_needle_img, 240, 240);
    lv_obj_remove_flag(min_needle_img, LV_OBJ_FLAG_CLICKABLE);

    return scr;
}

void ui_build(void)
{
    scr_rpm = build_rpm_screen();
    scr_clock = build_clock_screen();
    lv_screen_load(scr_rpm);
    printf("[ui] Screens aufgebaut. Klick/Touch auf das Fenster schaltet um.\n");
}

static void update_rpm_needle(uint16_t rpm)
{
    uint16_t clamped = rpm > RPM_NEEDLE_CLAMP ? RPM_NEEDLE_CLAMP : rpm;
    int32_t angle = RPM_START_ANGLE_TENTHS +
                    ((int32_t)clamped * RPM_SWEEP_TENTHS) / RPM_LABEL_MAX;
    lv_image_set_rotation(rpm_needle_img, angle % 3600);
}

static void update_shift_light(uint16_t rpm, uint32_t elapsed_ms)
{
    if(rpm >= RPM_SHIFT_BLINK) {
        shift_blink_acc_ms += elapsed_ms;
        if(shift_blink_acc_ms >= SHIFT_BLINK_HALF_MS) {
            shift_blink_acc_ms = 0;
            shift_blink_on = !shift_blink_on;
            if(shift_blink_on)
                lv_obj_remove_flag(shift_flash, LV_OBJ_FLAG_HIDDEN);
            else
                lv_obj_add_flag(shift_flash, LV_OBJ_FLAG_HIDDEN);
        }
    }
    else {
        shift_blink_acc_ms = 0;
        shift_blink_on = false;
        lv_obj_add_flag(shift_flash, LV_OBJ_FLAG_HIDDEN);
    }
}

static void update_clock_needles(uint8_t hour, uint8_t minute)
{
    int32_t hour_angle = ((hour % 12) * 300) + (minute * 5);
    int32_t min_angle = minute * 60;
    lv_image_set_rotation(hour_needle_img, hour_angle % 3600);
    lv_image_set_rotation(min_needle_img, min_angle % 3600);
}

void ui_tick(const vehicle_data_t *data, uint32_t elapsed_ms)
{
    /* RPM-Screen wird immer aktualisiert (auch im Hintergrund), damit
     * beim Zurueckschalten sofort der korrekte Stand steht. */
    update_rpm_needle(data->rpm);
    update_shift_light(data->rpm, elapsed_ms);
    update_clock_needles(data->rtc_hour, data->rtc_minute);
}
