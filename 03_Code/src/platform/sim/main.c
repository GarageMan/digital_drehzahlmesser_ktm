/*
 * PC-Simulator-Einstiegspunkt (LVGL v9.1.0, eingebauter SDL2-Treiber).
 *
 * Baut exakt dieselben Screens auf wie spaeter das ESP32-Firmware-Image
 * (src/core/ui.c ist identisch fuer beide Ziele). Nur dieser main.c-Teil
 * und der Datenprovider (sim_provider.c) sind PC-spezifisch; auf dem
 * ESP32 tritt an ihre Stelle ein Arduino-Sketch mit den echten
 * Display-/Sensor-Treibern, siehe src/platform/esp32/main.cpp.
 *
 * Auf v9 portiert (siehe Sprint3_Code_Grundgeruest.md, Abschnitt
 * "LVGL-Versionswechsel"): v9 bringt seinen SDL-Treiber selbst mit
 * (lv_sdl_window_create()/lv_sdl_mouse_create()) - kein manuelles
 * Draw-Buffer-/Flush-Callback-Setup mehr noetig wie noch unter v8.
 */
#include <stdio.h>
#include <SDL2/SDL.h>
#include "lvgl/lvgl.h"
#include "core/ui.h"
#include "sim_provider.h"

#define DISP_HOR_RES 480
#define DISP_VER_RES 480

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    lv_init();
    lv_tick_set_cb(SDL_GetTicks); /* dieselbe Methode wie im ESP32-Sketch (dort millis()) */

    lv_display_t *display = lv_sdl_window_create(DISP_HOR_RES, DISP_VER_RES);
    lv_sdl_window_set_title(display, "Digitaler Drehzahlmesser - PC-Simulator");

    /* Maus als Touch-Ersatz: ein Linksklick im Fenster = ein Tap auf das
     * (spaeter echte) kapazitive Touch-Display. */
    lv_indev_t *mouse = lv_sdl_mouse_create();
    lv_indev_set_display(mouse, display);

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

        lv_timer_handler(); /* pumpt auch SDL-Events (Maus/Fenster) */
        provider->update(elapsed_ms > 0 ? elapsed_ms : 1);

        vehicle_data_t data = provider->get();
        ui_tick(&data, elapsed_ms > 0 ? elapsed_ms : 1);

        SDL_Delay(5);
    }

    return 0;
}
