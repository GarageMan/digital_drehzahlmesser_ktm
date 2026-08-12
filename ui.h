#ifndef UI_H
#define UI_H

#include "vehicle_data.h"

/* Baut beide Screens (Drehzahlmesser + Analoguhr) auf. Einmalig nach
 * lv_init()/Display-Treiber-Init aufrufen. */
void ui_build(void);

/* Einmal pro Hauptschleifen-Durchlauf aufrufen, nachdem der aktive
 * vehicle_data_provider aktualisiert wurde. Aktualisiert Zeigerwinkel,
 * Shift-Light-Blinken usw. anhand der uebergebenen Werte. */
void ui_tick(const vehicle_data_t *data, uint32_t elapsed_ms);

#endif /* UI_H */
