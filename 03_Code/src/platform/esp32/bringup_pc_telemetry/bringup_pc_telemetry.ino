/*
 * Bring-up-Test #1 fuer das reale CrowPanel-2.1"-Board (ESP32-S3R8):
 * Zeigt PC-Telemetrie (CPU-/GPU-Temperatur, Last, RAM) an, umschaltbar per
 * Touch-Tap UND per Dreh-/Druckmechanismus des Encoders. Ziel ist NICHT die
 * finale Drehzahlmesser-UI, sondern ein einfacher, schnell zum Laufen zu
 * bringender Test, der zeigt: Display + Touch + Encoder + serielle
 * Datenuebernahme vom PC funktionieren auf dem echten Board.
 *
 * WICHTIG - bewusst NICHT in dieser Datei enthalten (siehe Sprint-1-Lehre:
 * keine erfundenen Pin-/Init-Daten):
 *   - Die Display-Panel-Initialisierung (ST7701 RGB-Timing ueber
 *     Arduino_ESP32RGBPanel bzw. die vom Beispiel genutzte Bibliothek)
 *   - Der Touch-Treiber (CST8xx via I2C)
 *   - Das PCF8574-Setup fuer Touch-Reset/-Interrupt und LCD-Power/-Reset
 *
 * Diese drei Bloecke MUESSEN unveraendert aus dem offiziellen Beispiel-Sketch
 * "RotaryScreen_2_1.ino" (Ordner example/ im GitHub-Repo Elecrow-RD/
 * CrowPanel-2.1inch-HMI-ESP32-Rotary-Display-480-480-IPS-Round-Touch-Knob-
 * Screen, Branch "master") in dieses Projekt kopiert werden. Alles unten
 * baut auf den Funktionen/Objekten auf, die jenes Beispiel bereitstellt -
 * falls die tatsaechlichen Funktionsnamen dort anders lauten, bitte
 * anpassen (NICHT raten).
 *
 * Vorgehen (siehe auch README.md in diesem Ordner):
 *   1. RotaryScreen_2_1.ino oeffnen, den Display-/Touch-/PCF8574-Init-Block
 *      aus setup() 1:1 hier in setup() uebernehmen (siehe TODO-Marker unten).
 *   2. Die Funktion, die dort Touch-Events liest (Timer-Callback fuer
 *      lv_indev), ebenso uebernehmen - LVGL braucht sie fuer Touch-Events.
 *   3. Bibliothek "lvgl" in Version 8.3.11 ueber den Arduino Library Manager
 *      installieren (siehe README.md, exakte Version bestaetigt aus der
 *      Elecrow-Dokumentation).
 *   4. Board-Einstellungen wie im offiziellen Beispiel (Partition Scheme:
 *      "Huge APP", PSRAM: OPI PSRAM aktiviert) - siehe README.md.
 *
 * Bestaetigte Pins/Adressen (aus dem offiziellen Beispiel-Sketch
 * recherchiert, siehe Sprint3_Code_Grundgeruest.md Abschnitt 6/7):
 */
#define PIN_BACKLIGHT           6     // PWM, Kanal 0, 5kHz, 8-bit
#define PIN_I2C_SDA             38
#define PIN_I2C_SCL             39
#define PIN_ENCODER_A           42
#define PIN_ENCODER_B           4
#define PIN_BREATHING_LED       43    // im Original-Beispiel belegt (Breathing-LED) - hier ungenutzt

#define PCF8574_I2C_ADDR        0x21
#define PCF8574_BIT_TOUCH_RESET 0
#define PCF8574_BIT_TOUCH_INT   2
#define PCF8574_BIT_LCD_POWER   3
#define PCF8574_BIT_LCD_RESET   4
#define PCF8574_BIT_ENCODER_SW  5     // Encoder-Taster, active-low mit Pullup

#define TOUCH_I2C_ADDR          0x15  // CST8xx

/*
 * NICHT bestaetigt / noch offen (siehe Sprint3-Doku Abschnitt 6):
 *   - UART1 TX/RX-GPIOs (Anfrage an Elecrow-Support raus, Antwort steht aus)
 * Fuer DIESEN Bring-up-Test irrelevant: Wir nutzen fuer die PC-Telemetrie
 * die bereits vorhandene USB-Verbindung (Serial, ueber USB-5V-IN), nicht
 * UART1. UART1 wird erst fuer den spaeteren CAN-Transceiver benoetigt.
 */

#include <Arduino.h>
#include <lvgl.h>

// ---- TODO: Includes aus dem offiziellen Beispiel uebernehmen, z.B. ----
// #include <Arduino_GFX_Library.h>       // oder die dort verwendete Panel-Lib
// #include <Wire.h>
// #include "touch.h"                      // CST8xx-Treiber aus dem Beispiel
// #include "pcf8574.h"                    // IO-Expander-Zugriff aus dem Beispiel

// =====================================================================
// Anwendungslogik (PC-Telemetrie, Screens, Encoder/Touch-Umschaltung) -
// das ist der Teil, den dieser Bring-up-Test tatsaechlich neu beitraegt.
// =====================================================================

typedef struct {
    float cpu_temp_c;
    float gpu_temp_c;
    float cpu_load_pct;
    float ram_used_pct;
    bool  data_fresh;
} pc_telemetry_t;

static pc_telemetry_t g_telemetry = {0, 0, 0, 0, false};
static uint32_t g_last_data_ms = 0;

#define NUM_SCREENS 3
static const char *SCREEN_TITLES[NUM_SCREENS] = { "CPU-Temp", "GPU-Temp", "Last / RAM" };
static lv_obj_t *g_screens[NUM_SCREENS];
static lv_obj_t *g_value_labels[NUM_SCREENS];
static int g_current_screen = 0;

// ---- Vorwaertsdeklarationen ----
static void next_screen(void);
static void prev_screen(void);
static void update_active_screen_value(void);
static void screen_touch_cb(lv_event_t *e);
static void parse_telemetry_line(const char *line);
static void poll_serial(void);
static void poll_encoder_rotation(void);
static void poll_encoder_button(void);

// ---- UI-Aufbau ----
static void build_screens(void) {
    for (int i = 0; i < NUM_SCREENS; i++) {
        g_screens[i] = lv_obj_create(NULL);

        lv_obj_t *title = lv_label_create(g_screens[i]);
        lv_label_set_text(title, SCREEN_TITLES[i]);
        lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 60);

        g_value_labels[i] = lv_label_create(g_screens[i]);
        lv_label_set_text(g_value_labels[i], "--");
        lv_obj_set_style_text_font(g_value_labels[i], &lv_font_montserrat_48, 0);
        lv_obj_center(g_value_labels[i]);

        lv_obj_t *hint = lv_label_create(g_screens[i]);
        lv_label_set_text(hint, "Tippen oder Encoder drehen/druecken zum Wechseln");
        lv_obj_set_style_text_font(hint, &lv_font_montserrat_14, 0);
        lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -40);

        lv_obj_add_flag(g_screens[i], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(g_screens[i], screen_touch_cb, LV_EVENT_CLICKED, NULL);
    }
}

static void screen_touch_cb(lv_event_t *e) {
    (void)e;
    next_screen();
}

static void next_screen(void) {
    g_current_screen = (g_current_screen + 1) % NUM_SCREENS;
    lv_scr_load_anim(g_screens[g_current_screen], LV_SCR_LOAD_ANIM_MOVE_LEFT, 200, 0, false);
}

static void prev_screen(void) {
    g_current_screen = (g_current_screen - 1 + NUM_SCREENS) % NUM_SCREENS;
    lv_scr_load_anim(g_screens[g_current_screen], LV_SCR_LOAD_ANIM_MOVE_RIGHT, 200, 0, false);
}

static void update_active_screen_value(void) {
    char buf[40];
    bool stale = (!g_telemetry.data_fresh) || ((millis() - g_last_data_ms) > 3000);
    if (stale) {
        lv_label_set_text(g_value_labels[g_current_screen], "--");
        return;
    }
    switch (g_current_screen) {
        case 0: snprintf(buf, sizeof(buf), "%.1f C", g_telemetry.cpu_temp_c); break;
        case 1:
            if (g_telemetry.gpu_temp_c < 0) {
                snprintf(buf, sizeof(buf), "n/a");
            } else {
                snprintf(buf, sizeof(buf), "%.1f C", g_telemetry.gpu_temp_c);
            }
            break;
        case 2: snprintf(buf, sizeof(buf), "%.0f%% / %.0f%%", g_telemetry.cpu_load_pct, g_telemetry.ram_used_pct); break;
        default: snprintf(buf, sizeof(buf), "--"); break;
    }
    lv_label_set_text(g_value_labels[g_current_screen], buf);
}

// ---- Serielles Protokoll: "CPU:47.2;GPU:63.5;LOAD:22.0;RAM:71.4\n" ----
// Passend zu tools/pc_telemetry_sender/pc_telemetry_sender.py
static void parse_telemetry_line(const char *line) {
    float cpu = g_telemetry.cpu_temp_c, gpu = g_telemetry.gpu_temp_c;
    float load = g_telemetry.cpu_load_pct, ram = g_telemetry.ram_used_pct;
    bool got_any = false;

    char buf[128];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tok = strtok(buf, ";");
    while (tok != NULL) {
        char key[8] = {0};
        float val = 0;
        if (sscanf(tok, "%7[^:]:%f", key, &val) == 2) {
            if (strcmp(key, "CPU") == 0)  { cpu = val;  got_any = true; }
            if (strcmp(key, "GPU") == 0)  { gpu = val;  got_any = true; }
            if (strcmp(key, "LOAD") == 0) { load = val; got_any = true; }
            if (strcmp(key, "RAM") == 0)  { ram = val;  got_any = true; }
        }
        tok = strtok(NULL, ";");
    }

    if (got_any) {
        g_telemetry.cpu_temp_c = cpu;
        g_telemetry.gpu_temp_c = gpu;
        g_telemetry.cpu_load_pct = load;
        g_telemetry.ram_used_pct = ram;
        g_telemetry.data_fresh = true;
        g_last_data_ms = millis();
    }
}

static void poll_serial(void) {
    static char line_buf[128];
    static size_t line_len = 0;

    while (Serial.available()) {
        char c = (char)Serial.read();
        if (c == '\n') {
            line_buf[line_len] = '\0';
            if (line_len > 0) {
                parse_telemetry_line(line_buf);
            }
            line_len = 0;
        } else if (c != '\r' && line_len < sizeof(line_buf) - 1) {
            line_buf[line_len++] = c;
        }
    }
}

// ---- Encoder: Drehen wechselt Screens, Taste (PCF8574 P5) ebenfalls ----
// TODO: Quadratur-Dekodierung (Pins PIN_ENCODER_A/B) und PCF8574-Tasterlesen
// 1:1 aus dem offiziellen Beispiel uebernehmen. Die zwei Funktionen unten
// sind Platzhalter, die nur zeigen, WAS bei einem erkannten Dreh-/Druck-
// Ereignis passieren soll - nicht WIE die Rohsignale gelesen werden.
static void poll_encoder_rotation(void) {
    // TODO: bei erkanntem Rechtsdreh-Schritt: next_screen();
    // TODO: bei erkanntem Linksdreh-Schritt:  prev_screen();
}

static void poll_encoder_button(void) {
    // TODO: PCF8574-Bit PCF8574_BIT_ENCODER_SW lesen (active low).
    // Bei fallender Flanke (Tastendruck erkannt), zum Testen des
    // Druckmechanismus ebenfalls: next_screen();
}

void setup() {
    Serial.begin(115200);
    Serial.println("Bring-up: PC-Telemetrie-Test startet...");

    // ============================================================
    // TODO: HIER den kompletten Display-/Touch-/PCF8574-Init-Block aus
    // RotaryScreen_2_1.ino (setup()-Funktion) einfuegen. Typischerweise:
    //   - Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    //   - PCF8574-Init (Touch-Reset toggeln, LCD-Power/-Reset toggeln)
    //   - Panel-Init (ST7701 RGB-Timing) + lv_init() + Display-/Touch-
    //     Treiber bei LVGL registrieren (lv_disp_drv_register,
    //     lv_indev_drv_register)
    //   - Backlight: ledcSetup/ledcAttachPin auf PIN_BACKLIGHT, Helligkeit
    //     setzen
    // ============================================================

    build_screens();
    lv_scr_load(g_screens[0]);
}

void loop() {
    lv_timer_handler();

    poll_serial();
    poll_encoder_rotation();
    poll_encoder_button();
    update_active_screen_value();

    delay(5);
}
