/*
 * Simulierter vehicle_data_provider fuer den PC-Simulator.
 *
 * Erzeugt plausible, zeitlich veraenderliche Werte fuer alle Kanaele aus
 * vehicle_data.h - nicht nur RPM/Uhr (Release-1-Umfang), sondern auch die
 * fuer spaetere Releases vorgesehenen Kanaele (Gang, Oel-/Wasser-/
 * Lufttemperatur, Bordnetzspannung, Backup-Akku). So laesst sich das
 * Zusammenspiel schon jetzt am PC durchspielen, ohne dass die Sensoren oder
 * das ESP32-Board vorhanden sind.
 *
 * main.c ruft sim_provider_update() einmal pro Hauptschleifen-Durchlauf auf
 * und pumpt dabei auch die SDL-Tastaturereignisse (siehe dortigen Kommentar).
 */
#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>
#include "sim_provider.h"

static vehicle_data_t data;

/* RPM-Sweep-Zustand */
static bool auto_sweep = true;
static bool rpm_rising = true;

/* Cranking-Dip-Zustand (Taste C) */
static bool crank_active = false;
static uint32_t crank_elapsed_ms = 0;
#define CRANK_DURATION_MS 1500u

/* Edge-Detection fuer Tasten, die nur "getriggert" werden sollen */
static bool key_prev_g = false;
static bool key_prev_c = false;
static bool key_prev_a = false;
static bool key_prev_r = false;

/* Laufzeit-Uhrzeit fuers Uhr-Zifferblatt (rein simuliert, ohne echte RTC) */
static uint32_t clock_ms_acc = 0;

/* fuer das periodische Konsolen-Log */
static uint32_t log_ms_acc = 0;

static void sim_init(void)
{
    data.rpm = 0;
    data.rtc_hour = 10;
    data.rtc_minute = 32;

    data.backup_batt_voltage_v = 4.05f; /* 1S LiPo, ca. 70% */
    data.backup_batt_percent = 70;
    data.backup_batt_charging = true;
    data.main_power_present = true;

    data.gear = 0;
    data.oil_temp_c = 16.0f;
    data.water_temp_c = 15.0f;
    data.air_temp_c = 18.0f;
    data.board_battery_v = 12.6f;
    data.speed_kmh = 0.0f;
    data.lean_angle_deg = 0.0f;

    data.rpm_source = DATA_SRC_SIM;
    data.gear_source = DATA_SRC_SIM;
    data.oil_temp_source = DATA_SRC_SIM;
    data.water_temp_source = DATA_SRC_SIM;
    data.air_temp_source = DATA_SRC_SIM;
    data.speed_source = DATA_SRC_SIM;

    printf("[sim] Simulierter Datenprovider aktiv. Tasten: Pfeil hoch/runter=RPM, "
           "A=Auto-Sweep an/aus, G=Gang weiterschalten, C=Bordnetz-Einbruch simulieren, "
           "R=RPM-Reset.\n");
}

static void handle_keys(uint32_t elapsed_ms)
{
    SDL_PumpEvents();
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    /* Auto-Sweep an/aus (Taste A, nur auf steigende Flanke reagieren) */
    bool key_a = keys[SDL_SCANCODE_A];
    if(key_a && !key_prev_a) {
        auto_sweep = !auto_sweep;
        printf("[sim] Auto-RPM-Sweep %s\n", auto_sweep ? "AN" : "AUS");
    }
    key_prev_a = key_a;

    /* Manuelles RPM-Nudging nur, wenn Auto-Sweep aus ist */
    if(!auto_sweep) {
        if(keys[SDL_SCANCODE_UP]) {
            int32_t r = (int32_t)data.rpm + 40;
            data.rpm = (uint16_t)(r > 9500 ? 9500 : r);
        }
        if(keys[SDL_SCANCODE_DOWN]) {
            int32_t r = (int32_t)data.rpm - 40;
            data.rpm = (uint16_t)(r < 0 ? 0 : r);
        }
    }

    /* RPM-Reset (Taste R) */
    bool key_r = keys[SDL_SCANCODE_R];
    if(key_r && !key_prev_r) {
        data.rpm = 0;
        printf("[sim] RPM zurueckgesetzt.\n");
    }
    key_prev_r = key_r;

    /* Gang weiterschalten (Taste G) */
    bool key_g = keys[SDL_SCANCODE_G];
    if(key_g && !key_prev_g) {
        data.gear = (int8_t)((data.gear + 1) % 7); /* 0..6 */
        printf("[sim] Gang -> %d\n", data.gear);
    }
    key_prev_g = key_g;

    /* Bordnetz-Einbruch simulieren (Taste C, nur wenn nicht schon aktiv) */
    bool key_c = keys[SDL_SCANCODE_C];
    if(key_c && !key_prev_c && !crank_active) {
        crank_active = true;
        crank_elapsed_ms = 0;
        printf("[sim] Bordnetz-Einbruch (Anlasser) simuliert - Backup-Akku "
               "sollte jetzt uebernehmen...\n");
    }
    key_prev_c = key_c;

    (void)elapsed_ms;
}

static void update_rpm(uint32_t elapsed_ms)
{
    if(!auto_sweep) return;

    /* ca. 3000 RPM/s Aenderungsrate, unabhaengig von der Framerate */
    int32_t delta = (int32_t)(3000.0f * (elapsed_ms / 1000.0f));
    if(delta < 1) delta = 1;

    if(rpm_rising) {
        int32_t r = (int32_t)data.rpm + delta;
        if(r >= 9500) { r = 9500; rpm_rising = false; }
        data.rpm = (uint16_t)r;
    }
    else {
        int32_t r = (int32_t)data.rpm - delta;
        if(r <= 0) { r = 0; rpm_rising = true; }
        data.rpm = (uint16_t)r;
    }
}

static void update_temperatures(uint32_t elapsed_ms)
{
    bool engine_running = data.rpm > 0;
    float dt = elapsed_ms / 1000.0f;

    /* Wassertemperatur naehert sich bei laufendem Motor ~92 degC,
     * kuehlt im Stand langsam auf Umgebungstemperatur ab. */
    float water_target = engine_running ? 92.0f : 18.0f;
    float water_rate = engine_running ? 6.0f : 0.5f; /* degC/s, grob */
    if(data.water_temp_c < water_target)
        data.water_temp_c += water_rate * dt;
    else if(data.water_temp_c > water_target)
        data.water_temp_c -= water_rate * dt;

    /* Oeltemperatur folgt der Wassertemperatur mit Verzoegerung und
     * etwas hoeherem Beharrungswert (typisch fuer Motorrad-Einzylinder). */
    float oil_target = water_target + (engine_running ? 15.0f : 0.0f);
    float oil_rate = engine_running ? 3.0f : 0.3f;
    if(data.oil_temp_c < oil_target)
        data.oil_temp_c += oil_rate * dt;
    else if(data.oil_temp_c > oil_target)
        data.oil_temp_c -= oil_rate * dt;

    /* Lufttemperatur: leichtes Rauschen um einen Basiswert, unabhaengig
     * vom Motor (repraesentiert einen separaten Ansaug-/Umgebungsfuehler). */
    static float t_acc = 0.0f;
    t_acc += dt;
    data.air_temp_c = 18.0f + 1.5f * sinf(t_acc * 0.2f);
}

static void update_board_power(uint32_t elapsed_ms)
{
    if(crank_active) {
        crank_elapsed_ms += elapsed_ms;

        /* Bordnetz bricht waehrend des Anlassens auf ~7-8V ein */
        data.board_battery_v = 7.5f;
        data.main_power_present = false;

        /* Backup-Akku uebernimmt: kein Laden, sichtbarer, aber langsamer
         * Prozent-Verlust waehrend der simulierten Ueberbrueckung. */
        data.backup_batt_charging = false;
        float drain = (elapsed_ms / (float)CRANK_DURATION_MS) * 3.0f; /* ca. 3% je Dip */
        if(data.backup_batt_percent > (uint8_t)drain)
            data.backup_batt_percent -= (uint8_t)drain;

        if(crank_elapsed_ms >= CRANK_DURATION_MS) {
            crank_active = false;
            printf("[sim] Bordnetz wieder stabil (12.6V). Backup-Akku laedt "
                   "wieder nach.\n");
        }
        return;
    }

    /* Normalbetrieb: Lichtmaschine haelt Spannung, wenn der Motor laeuft */
    data.board_battery_v = (data.rpm > 800) ? 14.4f : 12.6f;
    data.main_power_present = true;
    data.backup_batt_charging = (data.backup_batt_percent < 100);
    if(data.backup_batt_charging) {
        static float charge_acc = 0.0f;
        charge_acc += (elapsed_ms / 1000.0f) * 0.5f; /* langsames Nachladen */
        if(charge_acc >= 1.0f) {
            charge_acc -= 1.0f;
            if(data.backup_batt_percent < 100) data.backup_batt_percent++;
        }
    }
    data.backup_batt_voltage_v = 3.3f + (data.backup_batt_percent / 100.0f) * 0.9f;
}

static void log_snapshot(uint32_t elapsed_ms)
{
    log_ms_acc += elapsed_ms;
    if(log_ms_acc < 500) return;
    log_ms_acc = 0;

    printf("[sim] RPM=%5u  Gang=%d  Wasser=%5.1fC  Oel=%5.1fC  Luft=%5.1fC  "
           "Bordnetz=%4.1fV%s  Backup=%3u%%%s\n",
           data.rpm, data.gear, data.water_temp_c, data.oil_temp_c, data.air_temp_c,
           data.board_battery_v, data.main_power_present ? "" : " (AUSGEFALLEN!)",
           data.backup_batt_percent, data.backup_batt_charging ? " (laedt)" : "");
}

static void sim_update(uint32_t elapsed_ms)
{
    if(elapsed_ms == 0) elapsed_ms = 1;

    handle_keys(elapsed_ms);
    update_rpm(elapsed_ms);
    update_temperatures(elapsed_ms);
    update_board_power(elapsed_ms);

    /* simulierte Uhr: 60x Realzeit, damit man den Minutenzeiger im
     * Simulator tatsaechlich laufen sieht, ohne eine Stunde zu warten */
    clock_ms_acc += elapsed_ms * 60;
    if(clock_ms_acc >= 60000) {
        clock_ms_acc -= 60000;
        data.rtc_minute++;
        if(data.rtc_minute >= 60) {
            data.rtc_minute = 0;
            data.rtc_hour = (data.rtc_hour + 1) % 24;
        }
    }

    log_snapshot(elapsed_ms);
}

static vehicle_data_t sim_get(void)
{
    return data;
}

static const vehicle_data_provider_t provider = {
    .init = sim_init,
    .update = sim_update,
    .get = sim_get,
};

const vehicle_data_provider_t *sim_provider_get(void)
{
    return &provider;
}
