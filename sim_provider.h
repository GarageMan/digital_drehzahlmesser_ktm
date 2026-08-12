#ifndef SIM_PROVIDER_H
#define SIM_PROVIDER_H

#include "../../core/vehicle_data.h"

/* Liefert die Provider-Vtable fuer den PC-Simulator. main.c registriert sie
 * genau so, wie spaeter main.cpp auf dem ESP32 die reale (Analog-/CAN-)
 * Implementierung registrieren wird. */
const vehicle_data_provider_t *sim_provider_get(void);

/* Tastaturbelegung im Simulatorfenster (siehe README.md):
 *   Pfeil hoch/runter  RPM manuell nudgen (nur wenn Auto-Sweep aus)
 *   A                  Auto-RPM-Sweep an/aus (Standard: an)
 *   G                  Gang manuell weiterschalten (0=N .. 6)
 *   C                  Bordnetz-Einbruch simulieren (Anlasser-Cranking,
 *                       ~1.5s Spannungseinbruch + Backup-Akku-Uebernahme)
 *   R                  RPM sofort auf 0 zuruecksetzen
 */

#endif /* SIM_PROVIDER_H */
