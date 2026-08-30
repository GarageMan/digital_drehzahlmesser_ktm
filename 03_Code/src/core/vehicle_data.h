#ifndef VEHICLE_DATA_H
#define VEHICLE_DATA_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Zentrales Datenmodell fuer alle Werte, die die Anzeige darstellen oder
 * spaeter darstellen koennte. Bewusst breiter gefasst als der Funktionsumfang
 * von Release 1 (RPM + Uhr), damit sich der Simulator schon jetzt mit allen
 * fuer die Roadmap vorgesehenen Kanaelen fuettern laesst (siehe Feature-
 * Tabelle in Sprint3_Code_Grundgeruest.md). Welche Felder das UI tatsaechlich
 * anzeigt, entscheidet ui.c - nicht dieser Header.
 *
 * gear:  -1 = unbekannt/Sensor nicht verbunden, 0 = Leerlauf, 1..6 = Gang
 * *_source: woher der jeweilige Wert kommt (nur informativ, z.B. fuers Log
 *           und fuer eine spaetere Diagnoseanzeige)
 */

typedef enum {
    DATA_SRC_NONE = 0,     /* kein Sensor / nicht implementiert */
    DATA_SRC_SIM,          /* simulierter Wert (PC-Simulator) */
    DATA_SRC_ANALOG,       /* realer Analog-/ADC-Sensor am ESP32 */
    DATA_SRC_CAN,          /* aus dem Motorrad-CAN-Bus dekodiert */
} data_source_t;

typedef struct {
    /* Release 1 */
    uint16_t rpm;                  /* Motordrehzahl [1/min] */
    uint8_t  rtc_hour;             /* 0-23 */
    uint8_t  rtc_minute;           /* 0-59 */

    /* Backup-Akku am ESP32 (Ueberbrueckung von Bordnetz-Ausfaellen) */
    float    backup_batt_voltage_v;
    uint8_t  backup_batt_percent;  /* 0-100, grobe Schaetzung */
    bool     backup_batt_charging;
    bool     main_power_present;   /* 5V vom Bordnetz-Wandler liegt an */

    /* Release 2/3 - schon im Modell, im UI von Release 1 noch nicht sichtbar */
    int8_t   gear;                 /* -1 unbekannt, 0 Leerlauf, 1..6 */
    float    oil_temp_c;
    float    water_temp_c;
    float    air_temp_c;
    float    board_battery_v;      /* Bordnetzspannung (Starter-Einbruch sichtbar) */
    float    speed_kmh;            /* nur ueber CAN verfuegbar, siehe ktm-can */
    float    lean_angle_deg;       /* nur ueber CAN verfuegbar, siehe ktm-can */

    data_source_t rpm_source;
    data_source_t gear_source;
    data_source_t oil_temp_source;
    data_source_t water_temp_source;
    data_source_t air_temp_source;
    data_source_t speed_source;
} vehicle_data_t;

/* Provider-Schnittstelle: austauschbar zwischen Sim (PC) und realer Hardware
 * (ESP32). ui.c und main.c kennen nur dieses Interface, nie die konkrete
 * Implementierung - das ist der Kern der "1:1 Code-Uebernahme" auf den
 * ESP32, die schon in Sprint 1 als Ziel formuliert wurde. */
typedef struct {
    void (*init)(void);
    void (*update)(uint32_t elapsed_ms); /* seit letztem Aufruf */
    vehicle_data_t (*get)(void);
} vehicle_data_provider_t;

#endif /* VEHICLE_DATA_H */
