/*
 * PC-Simulator-Einstiegspunkt (LVGL v8 + SDL2).
 *
 * Baut exakt dieselben Screens auf wie spaeter das ESP32-Firmware-Image
 * (src/core/ui.c ist identisch fuer beide Ziele). Nur dieser main.c-Teil
 * und der Datenprovider (sim_provider.c) sind PC-spezifisch; auf dem
 * ESP32 tritt an ihre Stelle ein Arduino-Sketch mit den echten
 * Display-/Sensor-Treibern, siehe src/platform/esp32/README.md.
 */
#include <stdio.h>
#include <SDL2/SDL.h>
#include "lvgl/lvgl.h"
#include "lv_drivers/sdl/sdl.h"
#include "core/ui.h"
#include "sim_provider.h"

#define DISP_HOR_RES 480
#define DISP_VER_RES 480

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[DISP_HOR_RES * 10];
static lv_color_t buf2[DISP_HOR_RES * 10];

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    lv_init();
    sdl_init();

    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, DISP_HOR_RES * 10);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &draw_buf;
    disp_drv.flush_cb = sdl_display_flush;
    disp_drv.hor_res = DISP_HOR_RES;
    disp_drv.ver_res = DISP_VER_RES;
    lv_disp_drv_register(&disp_drv);

    /* Maus als Touch-Ersatz: ein Linksklick im Fenster = ein Tap auf das
     * (spaeter echte) kapazitive Touch-Display. */
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = sdl_mouse_read;
    lv_indev_drv_register(&indev_drv);

    ui_build();

    const vehicle_data_provider_t *provider = sim_provider_get();
    provider->init();

    printf("\n=== Digitaler Drehzahlmesser - PC-Simulator ===\n");
    printf("Fenster: %dx%d (entspricht dem runden 2.1\"-CrowPanel-Display)\n",
           DISP_HOR_RES, DISP_VER_RES);
    printf("Klick ins Fenster = Touch-Umschaltung RPM <-> Uhr.\n");
    printf("Tastaturkuerzel siehe sim_provider.h.\n\n");

    uint32_t last_ms = SDL_GetTicks();
    while(1) {
        uint32_t now_ms = SDL_GetTicks();
        uint32_t elapsed_ms = now_ms - last_ms;
        last_ms = now_ms;

        lv_tick_inc(elapsed_ms > 0 ? elapsed_ms : 1);
        lv_timer_handler(); /* pumpt auch SDL-Events (Maus/Fenster) */
        provider->update(elapsed_ms);

        vehicle_data_t data = provider->get();
        ui_tick(&data, elapsed_ms);

        SDL_Delay(5);
    }

    return 0;
}
