#ifndef UI_H
#define UI_H

#include "vehicle_data.h"

/* ui.c wird immer als C kompiliert (siehe Kommentar dort), aber auf dem
 * ESP32 von einem .ino-Sketch aufgerufen, den die Arduino-Toolchain immer
 * als C++ kompiliert. Ohne extern "C" wuerde der C++-Aufrufer C++-Name-
 * Mangling fuer diese Funktionen erwarten, ui.c liefert aber C-Symbole ->
 * Linker-Fehler ("undefined reference"). Der PC-Simulator (main.c, reines
 * C) war davon nie betroffen, deshalb ist das erst beim ESP32-Sketch
 * aufgefallen. */
#ifdef __cplusplus
extern "C" {
#endif

/* Baut beide Screens (Drehzahlmesser + Analoguhr) auf. Einmalig nach
 * lv_init()/Display-Treiber-Init aufrufen. */
void ui_build(void);

/* Einmal pro Hauptschleifen-Durchlauf aufrufen, nachdem der aktive
 * vehicle_data_provider aktualisiert wurde. Aktualisiert Zeigerwinkel,
 * Shift-Light-Blinken usw. anhand der uebergebenen Werte. */
void ui_tick(const vehicle_data_t *data, uint32_t elapsed_ms);

#ifdef __cplusplus
}
#endif

#endif /* UI_H */
