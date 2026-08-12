> **Hinweis:** Diese Datei ist eine Text-Extraktion aus dem Original-Dokument "MotorradDrehzahlanzeige mit ESP32.pdf" im Projekt (Export eines Gemini-Chats, Quelle: https://gemini.google.com/app/02fa34aea0680364). Das Original-PDF konnte über die verfügbaren Werkzeuge nicht als Binärdatei abgerufen werden (nur der extrahierte Textinhalt war zugänglich). Die automatische Text-Extraktion aus dem PDF enthält bei den zahlreichen SVG-Code-Blöcken und mathematischen Formeln (Gradangaben) OCR-/Extraktions-Artefakte (abgeschnittene Zahlen, verschobene Zeilenumbrüche, vereinzelt fehlerhafte Sonderzeichen). Der Inhalt ist daher hier bewusst zu einer sachlich korrekten, lesbaren Zusammenfassung verdichtet statt 1:1 der fehlerhaften Rohextraktion. Das Original mit vollständiger Formatierung kann direkt aus dem Projekt in der Claude-App heruntergeladen werden.
>
> **Wichtiger Kontext:** Dieses Dokument ist Teil desselben ursprünglichen Gemini-Chats wie "Digitale Drehzahlanzeige.docx" (siehe `Digitale_Drehzahlanzeige_Ausgangsidee_TEXTAUSZUG.md` im Ordner `01_Projektdokumente`) und enthält die Pin-Tabelle, die in Sprint 1 als teilweise fabriziert identifiziert wurde, sowie die iterative Entwicklung der SVG-Zifferblatt-Grafiken (rpm_dial, clock_dial, Zeiger). Für die tatsächlich im Projekt verwendeten, finalen PNG-Grafiken siehe die Bilddateien in diesem Ordner (`rpm_dial.png`, `clock_dial.png`, `rpm_needle.png`, `hour_needle.png`, `min_needle.png`).

---

# Motorrad-Drehzahlanzeige mit ESP32 — Gemini-Chat-Export (Fortsetzung)

Quelle: https://gemini.google.com/app/02fa34aea0680364, Chat vom 04.08.2026 (Fortsetzung des Chats aus "Digitale Drehzahlanzeige.docx")

## 1. Pin-Ansteuerung (Pinout ESP32-S3) — ⚠️ unverifiziert/in Sprint 1 als teilweise fabriziert bewertet

Das Display TTC248XRS-01 wurde hier laut (späterer als falsch erkannter) Annahme auf dem ST7701S-Treiber basierend beschrieben, mit einer Kombi-Schnittstelle (3-Wire SPI für Initialisierung + 16-Bit Parallel RGB für Bilddaten). Das tatsächliche Datenblatt (siehe `TTC248XRS01_TEXTAUSZUG.md`) weist stattdessen ein reines MIPI-DSI-Interface aus — diese Diskrepanz war einer der zentralen kritischen Befunde aus Sprint 1.

### A. Display & Touch (TTC248XRS-01 / ST7701S) — wie ursprünglich angenommen

| Display Pin | Signal-Funktion | ESP32-S3 GPIO | Beschreibung |
| --- | --- | --- | --- |
| LEDA / LEDK | Backlight VCC / GND | 5V / GND | Hintergrundbeleuchtung (Anode 5V, Kathode GND) |
| VCI / IOVCC | Logik-Spannung | 3.3V / GND | Logik-Versorgung Display |
| RESET | Display Reset | GPIO 4 | Hardware-Reset für ST7701S |
| CS / SCLK / SDA | 3-Wire SPI | GPIO 5, 6, 7 | Befehls-Schnittstelle zur Display-Initialisierung |
| PCLK / DE / HS / VS | RGB Control Clocks | GPIO 1, 2, 3, 41 | Pixel Clock, Data Enable, H-Sync, V-Sync |
| D0 ... D15 | RGB565 Data Bus | GPIO 8–15, 17–24 | 16 Parallel-Datenleitungen für 65.536 Farben |
| TP_SDA / TP_SCL | Touch I2C Bus | GPIO 38, 39 | Touch-Datenverbindung (FT6336 / CST816) |
| TP_INT / TP_RST | Touch Interrupt/Reset | GPIO 40, 42 | Touch-Ereignis-Erkennung |

### B. Sensoren & Peripherie

| Baugruppe | Signal-Funktion | ESP32-S3 GPIO | Beschreibung |
| --- | --- | --- | --- |
| Kurbelwellensensor | RPM Pulse Input | GPIO 35 | Interrupt-Pin (nach Signalaufbereitung) |
| DS3231 RTC Module | I2C Bus (SDA / SCL) | GPIO 36, 37 | Hardware-Uhrzeit (3.3V & GND) |

## 2. Erstellung der PNG-Grafiken für Simulator & LVGL

Ziel: 5 separate 480x480-Pixel-PNGs mit transparentem Hintergrund (PNG-24), erstellbar mit Photoshop, GIMP, Figma oder Canva.

1. **rpm_dial.png** — Weißes Zifferblatt, Skala "0" bis "6" bzw. "0" bis "9" (nach Iteration), Schriftzug "rpm x 1000", roter/rosa Warnbereich. Mittelpunkt frei oder mit Abdeckkappe.
2. **clock_dial.png** — Schwarzes Zifferblatt mit Ziffern 12, 3, 6, 9 und Strich-Indizes für alle 12 Stunden.
3. **rpm_needle.png** — Rote Nadel, exakt senkrecht nach oben (12-Uhr-Position), Drehpunkt exakt bei Bildmitte (240, 240) — wichtig, damit im Code keine Offset-Berechnung nötig ist.
4. **hour_needle.png** — Stundenzeiger, ebenfalls nach oben zeigend, Drehachse bei (240, 240).
5. **min_needle.png** — Minutenzeiger, gleiche Konvention.

Einbindung in LVGL (PC-Simulator wie später ESP32): PNGs über das offizielle **LVGL Image Converter** Tool (lvgl.io/tools/imageconverter) in C-Arrays umwandeln — Farbformat `CF_TRUE_COLOR_ALPHA` (für Transparenz), Output `C array`. Erzeugte `.c`-Dateien in `src/assets/` ablegen.

```c
LV_IMG_DECLARE(rpm_dial);
LV_IMG_DECLARE(rpm_needle);

lv_obj_t * bg = lv_img_create(screen_rpm);
lv_img_set_src(bg, &rpm_dial);
lv_obj_align(bg, LV_ALIGN_CENTER, 0, 0);

lv_obj_t * needle = lv_img_create(screen_rpm);
lv_img_set_src(needle, &rpm_needle);
lv_obj_align(needle, LV_ALIGN_CENTER, 0, 0);
lv_img_set_pivot(needle, 240, 240);
```

## 3. Alternative: Vektor-SVGs für Bildbearbeitungsprogramme

Da ein KI-Modell keine fertigen PNG-Dateien zum Download erzeugen kann, wurden stattdessen SVG-Textblöcke bereitgestellt, die sich in Inkscape, Illustrator oder Photoshop öffnen und exportieren lassen. Empfohlene Open-Source-Tools:

- **Inkscape** — professionelles Vektorgrafik-Tool, exportiert SVG direkt als transparentes 480x480-PNG (Datei → Exportieren)
- **GIMP** — rastert SVGs beim Öffnen direkt auf gewünschte Auflösung, gut für Effekte (Schatten, Texturen)
- **ImageMagick** (CLI) — Kommandozeilen-Konvertierung, z. B. `magick convert -background none -density 300 rpm_dial.svg rpm_dial.png`

## 4. Iterative Verfeinerung der SVG-Grafiken (Zusammenfassung des Dialogs)

Der Chat enthält eine lange Iterationsreihe zur exakten Geometrie des Drehzahlmesser-Zifferblatts, die hier inhaltlich zusammengefasst wird (die Originaldatei enthält jeden einzelnen SVG-Codestand; wegen OCR-Qualität der PDF-Extraktion wird hier nur der fachliche Kern wiedergegeben):

1. **Ausgangsversion**: Skala 0–6, Warnbereich 6000–9000 U/min als rotes Band, "rpm x 1000" auf 6-Uhr-Position, schwarzer Mittelkreis als Nadelkopf.
2. **Erweiterung auf Skala 0–9**: rpm-Bereich bis 9000, roter Balken 6000–9000 U/min, schwarzer ausgefüllter Kreis im Zentrum.
3. **Korrektur Skalenbogen**: Zahlen 0–9 sollen nicht vollen Kreis bilden, sondern von 8-Uhr- bis 4-Uhr-Position verlaufen (240°-Bogen) — klassische Tacho-Optik.
4. **Klärung "innerer Kreis"**: Nutzer meinte ursprünglich nicht den Nadelkopf, sondern eine störende zweite Skalenlinie unter dem roten Band — diese wurde entfernt, der Nadelkopf (Abdeckkappe) blieb erhalten.
5. **Verschiebung des roten Warnbands**: mehrere Korrekturrunden zu Start-/Endwinkel (6500–9500 U/min), Breite, Position relativ zu den Ziffern — inklusive eines Rechenfehlers bei der Umrechnung zwischen "Uhrzeigersinn ab 6-Uhr-Position" (fachlicher Bezug des Nutzers) und dem SVG-Standardwinkelsystem (0° = 3-Uhr-Position, im Uhrzeigersinn), der vom Nutzer explizit korrigiert wurde ("Seit wann liegt die 11 auf einem Uhrenziffernblatt bei 350 Grad im svg-Kontext...").
6. **SVG-Arc-Flag-Fehler**: Ein falsch gesetztes `large-arc-flag` ließ den Bogen einmal über die "falsche", lange Seite des Kreises verlaufen ("verstrubbelt") — korrigiert durch Wechsel des Flags auf den kurzen Bogen im Uhrzeigersinn.
7. **Zweifarbiges Warnband**: finale Version mit zwei Teilbögen — helles Rosa (`#FFB3D1`) als Vorwarnbereich von 5500–7000 U/min, kräftiges Rot (`#FF2200`) als kritischer Bereich von 7000–9500 U/min, nahtlos ineinander übergehend.
8. **Feinjustierung**: Schriftgröße der Ziffern um 75% vergrößert (32px → 56px), "rpm x 1000"-Schriftzug weiter Richtung Zentrum verschoben, rotes Band nach außen verbreitert, sodass es die Ziffern 7/8/9 mittig überlagert.

Das Uhren-Zifferblatt (`clock_dial.svg`) wurde parallel verfeinert: Ziffern 12/3/6/9 ganz außen am Rand, Teilstriche für alle 12 Stunden weiter innen platziert, schwarzer Mittelkreis für die Zeigerachse.

Die tatsächlich fertiggestellten, finalen PNG-Dateien (`rpm_dial.png`, `clock_dial.png`, `rpm_needle.png`, `hour_needle.png`, `min_needle.png`) liegen unverändert im Ordner `02_Bilder_und_Referenzdateien` dieses Pakets bei und sind identisch mit den im Projekt hinterlegten Original-Bilddateien.

## 5. Referenz-C++-Code (finale, im Chat konsolidierte Fassung)

```cpp
#include <Arduino.h>
#include <lvgl.h>
#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

enum ViewState { VIEW_RPM, VIEW_CLOCK };
ViewState current_view = VIEW_RPM;

lv_obj_t * main_screen;
lv_obj_t * dial_bg;
lv_obj_t * rpm_needle;
lv_obj_t * clock_hour_needle;
lv_obj_t * clock_min_needle;

LV_IMG_DECLARE(img_rpm_dial);
LV_IMG_DECLARE(img_clock_dial);
LV_IMG_DECLARE(img_needle_red);
LV_IMG_DECLARE(img_needle_black);

volatile uint32_t pulse_count = 0;
uint32_t last_calc_time = 0;
uint16_t current_rpm = 0;

uint32_t last_blink_time = 0;
bool blink_state = false;

void IRAM_ATTR rpm_isr() {
    pulse_count++;
}

static void screen_click_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (current_view == VIEW_RPM) {
            current_view = VIEW_CLOCK;
            lv_img_set_src(dial_bg, &img_clock_dial);
            lv_obj_add_flag(rpm_needle, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(clock_hour_needle, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(clock_min_needle, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_bg_opa(main_screen, LV_OPA_TRANSP, 0);
        } else {
            current_view = VIEW_RPM;
            lv_img_set_src(dial_bg, &img_rpm_dial);
            lv_obj_clear_flag(rpm_needle, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(clock_hour_needle, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(clock_min_needle, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void build_gui() {
    main_screen = lv_scr_act();
    lv_obj_set_style_bg_opa(main_screen, LV_OPA_TRANSP, 0);
    lv_obj_add_flag(main_screen, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(main_screen, screen_click_event_cb, LV_EVENT_CLICKED, NULL);

    dial_bg = lv_img_create(main_screen);
    lv_img_set_src(dial_bg, &img_rpm_dial);
    lv_obj_center(dial_bg);

    rpm_needle = lv_img_create(main_screen);
    lv_img_set_src(rpm_needle, &img_needle_red);
    lv_obj_center(rpm_needle);
    lv_img_set_pivot(rpm_needle, 240, 240);

    clock_hour_needle = lv_img_create(main_screen);
    lv_img_set_src(clock_hour_needle, &img_needle_black);
    lv_obj_center(clock_hour_needle);
    lv_img_set_pivot(clock_hour_needle, 240, 240);
    lv_obj_add_flag(clock_hour_needle, LV_OBJ_FLAG_HIDDEN);

    clock_min_needle = lv_img_create(main_screen);
    lv_img_set_src(clock_min_needle, &img_needle_black);
    lv_obj_center(clock_min_needle);
    lv_img_set_pivot(clock_min_needle, 240, 240);
    lv_obj_add_flag(clock_min_needle, LV_OBJ_FLAG_HIDDEN);
}

void update_display_logic() {
    uint32_t now = millis();
    if (now - last_calc_time >= 100) {
        noInterrupts();
        uint32_t pulses = pulse_count;
        pulse_count = 0;
        interrupts();
        current_rpm = (pulses * (60000 / (now - last_calc_time))) * 2;
        last_calc_time = now;

        if (current_view == VIEW_RPM) {
            // 0 RPM = 20.0 Grad relativ zu 6-Uhr, 9000 RPM = 340.0 Grad, Sweep = 320.0 Grad
            int32_t start_angle = 1100;
            int32_t total_sweep = 3200;
            int32_t needle_angle = start_angle + ((int32_t)current_rpm * total_sweep / 9000);
            lv_img_set_angle(rpm_needle, needle_angle % 3600);

            if (current_rpm >= 8900) {
                if (now - last_blink_time >= 100) {
                    blink_state = !blink_state;
                    last_blink_time = now;
                    lv_obj_set_style_bg_opa(main_screen, blink_state ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
                }
            } else {
                lv_obj_set_style_bg_opa(main_screen, LV_OPA_TRANSP, 0);
            }
        }
    }

    if (current_view == VIEW_CLOCK) {
        DateTime rtc_now = rtc.now();
        int32_t hour_angle = ((rtc_now.hour() % 12) * 300 + (rtc_now.minute() * 5));
        int32_t min_angle = (rtc_now.minute() * 60);
        lv_img_set_angle(clock_hour_needle, hour_angle);
        lv_img_set_angle(clock_min_needle, min_angle);
    }
}

void setup() {
    Serial.begin(115200);
    if (!rtc.begin()) {
        Serial.println("RTC nicht gefunden!");
    }
    pinMode(4, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(4), rpm_isr, RISING);

    lv_init();
    // [Hardware Display Driver Init Code hier]
    build_gui();
}

void loop() {
    lv_timer_handler();
    update_display_logic();
    delay(5);
}
```

## 6. Simulator-Setup (Windows 11/WSL2 vs. Linux Mint)

Der Nutzer fragte zunächst nach Windows 11 mit WSL2 — die Anleitung dazu (Ubuntu-Pakete via `apt`, VS Code WSL-Extension, `git clone --recursive https://github.com/lvgl/lv_port_pc_vscode.git`, WSLg für native Fensterausgabe) wurde bereitgestellt, aber der Nutzer entschied sich anschließend explizit für seinen Linux-Mint-Rechner ("WSL ist mir doch etwas zu 'unfertig'"). Die native Linux-Mint-Anleitung ist identisch mit der in `Digitale_Drehzahlanzeige_Ausgangsidee_TEXTAUSZUG.md` dokumentierten Vorgehensweise.
