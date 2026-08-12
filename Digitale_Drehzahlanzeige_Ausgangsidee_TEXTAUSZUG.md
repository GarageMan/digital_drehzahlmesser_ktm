> **Hinweis:** Diese Datei ist eine reine Text-Extraktion aus dem Original-Dokument "Digitale Drehzahlanzeige.docx" im Projekt (der ursprüngliche, per Gemini-Chat exportierte Projekt-Ideengeber). Das Original-Word-Dokument mit seiner Formatierung konnte über die verfügbaren Werkzeuge nicht als Binärdatei abgerufen werden (nur der Textinhalt war zugänglich). Das Original mit vollständiger Formatierung kann direkt aus dem Projekt in der Claude-App heruntergeladen werden.
>
> **Wichtiger Kontext:** Dieses Dokument enthält die ursprüngliche Idee inklusive einer Pin-Tabelle für das Display TTC248XRS-01, die sich im weiteren Projektverlauf (siehe Sprint 1) als teilweise fabriziert/unzutreffend herausgestellt hat (u. a. wurde MIPI-DSI-Interface fälschlich als SPI/RGB dargestellt, und die Pin-Nummern waren erfunden). Es ist hier als historisches Ausgangsdokument archiviert, nicht als technisch verlässliche Referenz. Die verlässliche technische Bewertung findet sich in `Sprint1_Machbarkeitsanalyse` und den Folgedokumenten.

---

# Digitale Drehzahlanzeige – Ausgangsidee (Gemini-Chat-Export)

Ich möchte eine digitale Drehzahlanzeige für mein Motorrad (KTM Enduro 690, Baujahr 2021) bauen. Dazu möchte ich das runde Display TTC248XRS-01 (2,48 inch, 480x480 Pixel) als Basis verwenden. An was muss ich das Display anschließen, damit eine grafische Anzeige der Drehzahl des Motorrads dargestellt werden kann (oldschool analoge Darstellung) und welchen Controller brauche ich dafür? Kurbelwellensensor: Das rohe Drehzahlsignal kann direkt am Sensor des linken Motorgehäuses abgegriffen werden, indem der vorhandene Stecker getrennt und ein passender Zubehör-Zwischenstecker eingesetzt wird.

## Alternativen (Display)

- Adafruit Rundes RGB 666 TTL TFT Display, 2.1" 480x480 mit kapazitivem Touch, TL021WVC02CT-B1323, 5792
- Newhaven Display NHD-2-1-480480AF-ASXP

## Empfehlung: Controller

Für ein Display dieser Art wird Grafik-Rechenleistung und Signalverarbeitung benötigt, da ein rohes Kurbelwellensignal weder direkt verarbeitet noch ohne weiteres als Zeiger dargestellt werden kann.

Empfohlene Familie: **ESP32-S3** oder **Raspberry Pi Pico (RP2040)**.

Warum ESP32-S3:
- Ausreichend Rechenleistung für flüssige Animationen (analoge Nadel)
- Hardware-SPI, grafische Bibliotheken (LVGL, TFT_eSPI)
- Integriertes WLAN/Bluetooth (optional für GPS-Logging, Firmware-Updates)

Anforderungen an den Controller:
- GPIOs mit Interrupt-Unterstützung (Impulse des Kurbelwellensensors zählen)
- Display-Schnittstelle: meist SPI (4-Wire) oder QSPI

## Verdrahtung (Konzept)

Das Display wird nicht direkt an den Motorradsensor angeschlossen, sondern an den Mikrocontroller. Der Mikrocontroller sitzt als "Übersetzer" dazwischen: er misst die Impulse des Sensors, berechnet die Drehzahl und zeichnet das analoge Rundinstrument auf das Display.

| Komponente | Anschluss / Pin am Mikrocontroller (z. B. ESP32-S3) | Erklärung |
| --- | --- | --- |
| Display (TTC248XRS-01) | SPI-Bus (MOSI, SCLK, CS, DC, RST) | Übertragung der Bilddaten vom Controller zum Display |
| Display Spannungsversorgung | 3.3V und GND | Logikspannung und Hintergrundbeleuchtung |
| Kurbelwellensensor | Digitaler GPIO-Pin (mit Interrupt) | Rohes Drehzahlsignal (Pegelüberprüfung nötig!) |
| Bordstromversorgung | 12V Bordnetz → Spannungsregler (Buck-Converter 5V/3.3V) | Motorrad liefert 12–14,4V; Controller/Display brauchen 5V/3.3V |

Wichtige technische Hürden:
- Signalaufbereitung des Kurbelwellensensors: schwankende Spannungspegel müssen vor dem GPIO-Pin geglättet und auf saubere 0–3.3V Logikpegel gebracht werden (z. B. Schmitt-Trigger 74HC14 oder Optokoppler)
- Spannungsspitzen im Motorradnetz: sauberer automotive-tauglicher Buck-Converter, Entkopplung per Diode und Pufferkondensator empfohlen

## Gewünschte Darstellung

Touch-Funktion: durch Tap auf das Display zwischen "Drehzahlmesser" und "Analoguhr" wechseln.

## PC-Simulation vor Hardware-Anschaffung

Ja, das komplette Projekt kann zu 100% am PC simuliert werden, mit der Grafikbibliothek **LVGL (Light and Versatile Graphics Library)**. Der LVGL-PC-Simulator führt den C-Code direkt auf dem PC aus, öffnet ein 480x480-Fenster, Maus fungiert als Finger (Touch-Simulation), Drehzahl kann simuliert werden. Der C-Code für Layout und Touch-Logik bleibt zu 99% identisch bei Übertragung auf den ESP32.

### Setup-Schritte (Original-Ideengeber, LVGL v8, VS Code)

1. Software: VS Code, Git, SDL2 (`sudo apt-get install build-essential libsdl2-dev` unter Linux; Homebrew `brew install sdl2` unter macOS)
2. Repository klonen: `git clone --recursive https://github.com/lvgl/lv_port_pc_vscode.git`
3. SVGs (rpm_dial.svg, clock_dial.svg etc.) über den LVGL Online Image Converter in C-Arrays (CF_TRUE_COLOR_ALPHA, Output: C array) umwandeln
4. main.c/main.cpp anpassen: Hardware-Aufrufe (`Arduino.h`, `attachInterrupt`, `RTClib`) durch Simulationscode ersetzen (Sweep-Timer für simulierte Drehzahl)
5. Fenstergröße auf 480x480 setzen (`MY_DISP_HOR_RES`/`MY_DISP_VER_RES`)
6. Bauen und starten: `mkdir build && cd build && cmake .. && make -j$(nproc) && ./main`

Anmerkung: Diese Anleitung wurde im Projekt später aufgegriffen und für Linux Mint konkretisiert (siehe `Sprint1_Machbarkeitsanalyse`); die PC-Simulation selbst wurde im Projekt tatsächlich umgesetzt.

## Architektur-Konzept: zwei Screens (RPM / Uhr)

```
               +----------------------------------+
               |          TOUCH EVENT             |
               | (Klick irgendwo auf den Schirm)  |
               +----------------------------------+
                                |
             +------------------+------------------+
             v                                     v
   +--------------------+                 +---------------------+
   |   SCREEN 1: RPM    | -- Tap Switch ->|  SCREEN 2: CLOCK    |
   +--------------------+                 +---------------------+
   | - Hintergrund-Grafik|                | - Hintergrund-Grafik|
   |   (Tacho weiß)     |                 |   (Uhr schwarz)     |
   | - Zeiger (Drehung) |                 | - Zeiger Std/Min    |
   +--------------------+                 +---------------------+
```

Für den Analog-Look: PNG-Bilddateien mit Transparenz. Hintergrund = rundes Zifferblatt-PNG, Zeiger = eigenes PNG (Drehpunkt in der Bildmitte 240,240), ggf. Nadel-Abdeckkappe als eigenes Element.

## RTC (Echtzeituhr) – DS3231-Modul

- Kommunikation via I²C (nur SDA/SCL zum ESP32-S3 nötig)
- Eigene CR2032-Knopfzelle: läuft auch bei ausgeschalteter Zündung weiter, verliert über Jahre kaum Zeit/Datum
- Integrierter temperaturkompensierter Quarz (TCXO): Abweichung unter 2 Min/Jahr, ideal bei Temperaturschwankungen am Motorrad

## Gesamt-Blockschaltbild (Originalidee)

```
+-------------------------------------------------------------------------+
|                             MOTORRAD (12V)                              |
+-------------------------------------------------------------------------+
       |                                                    |
       | (Zündung / 12V Bordnetz)                           | (Kurbelwellensensor)
       v                                                    v
+-----------------------+                          +--------------------+
| Automotive Step-Down  |                          | Signalaufbereitung |
| (Buck-Converter)      |                          | (Optokoppler /     |
| 12V -> 5V / 3.3V      |                          |  Schmitt-Trigger)  |
+-----------------------+                          +--------------------+
       |                                                    |
       | (Hauptstromversorgung)                             | (Sauberes 3.3V
       |                                                    | Rechteck-Signal)
       +----------------------------+                       |
       |                            |                       |
       v                            v                       v
+--------------------+   +------------------------------------------------+
| DS3231 RTC-Modul   |   |           MIKROCONTROLLER (ESP32-S3)           |
| (Echtzeituhr)      |   |                                                |
|                    |   |  - GPIO-Interrupt   <-- Drehzahlimpulse (RPM)  |
| [Knopfzelle CR2032]|   |  - I2C Bus (SDA/SCL)<-- Zeitdaten (Uhrzeit)    |
| (Hält Zeit auch    |   |  - Touch Interrupt  <-- Display-Tap Event      |
|  ohne Zündung)     |   |  - Framebuffer PSRAM<-- Rendert RPM / Uhr      |
+--------------------+   +------------------------------------------------+
          | (I2C-Bus)             |
          +-----------------------+
                                  | (MIPI DSI / RGB Data + SPI Init)
                                  v
+-------------------------------------------------------------------------+
|                        DISPLAY (TTC248XRS-01)                           |
|               2.48" / 480x480 (ST7701S + Touch-Controller)              |
+-------------------------------------------------------------------------+
```

## Ursprünglich angegebene Pin-Belegung (⚠️ in Sprint 1 als teilweise fabriziert identifiziert)

> Die folgende Tabelle stammt aus dem ursprünglichen Gemini-Chat und wird hier nur zu Dokumentationszwecken (Nachvollziehbarkeit der Projekthistorie) archiviert. Sie wurde in Sprint 1 kritisch geprüft und als nicht datenblattkonform bewertet (u. a. angebliches SPI/RGB-Interface, obwohl das reale TTC248XRS-01-Datenblatt MIPI-DSI und "Without CTP" ausweist). Nicht als technische Grundlage verwenden.

### Display & Touch (TTC248XRS-01 / ST7701S) — unverifiziert/fabriziert

| Display Pin | Signal-Funktion | ESP32-S3 GPIO | Beschreibung |
| --- | --- | --- | --- |
| LEDA / LEDK | Backlight VCC / GND | 5V / GND | Hintergrundbeleuchtung |
| VCI / IOVCC | Logik-Spannung | 3.3V / GND | Logik-Versorgung Display |
| RESET | Display Reset | GPIO 4 | Hardware-Reset für ST7701S |
| CS / SCLK / SDA | 3-Wire SPI | GPIO 5, 6, 7 | Befehls-Schnittstelle |
| PCLK / DE / HS / VS | RGB Control Clocks | GPIO 1, 2, 3, 41 | Pixel Clock, Data Enable, H-Sync, V-Sync |
| D0 ... D15 | RGB565 Data Bus | GPIO 8–15, 17–24 | 16 Parallel-Datenleitungen |
| TP_SDA / TP_SCL | Touch I2C Bus | GPIO 38, 39 | Touch-Datenverbindung (FT6336 / CST816) |
| TP_INT / TP_RST | Touch Interrupt/Reset | GPIO 40, 42 | Touch-Ereignis-Erkennung |

### Sensoren & Peripherie — unverifiziert/fabriziert

| Baugruppe | Signal-Funktion | ESP32-S3 GPIO | Beschreibung |
| --- | --- | --- | --- |
| Kurbelwellensensor | RPM Pulse Input | GPIO 35 | Interrupt-Pin |
| DS3231 RTC Module | I2C Bus (SDA/SCL) | GPIO 36, 37 | Hardware-Uhrzeit |

## Grafiken (PNG-Assets)

Für den 480x480-Simulator wurden 5 PNG-Grafiken benötigt (480x480 Pixel, transparenter Hintergrund, PNG-24):

1. `rpm_dial.png` — Zifferblatt Drehzahlmesser (Skala 0–6 bzw. 0–9, Beschriftung "rpm x 1000", roter Warnbereich)
2. `clock_dial.png` — Zifferblatt Analoguhr (Ziffern 12, 3, 6, 9)
3. `rpm_needle.png` — rote Drehzahl-Nadel, Drehpunkt exakt bei (240, 240)
4. `hour_needle.png` — Stundenzeiger, Drehpunkt (240, 240)
5. `min_needle.png` — Minutenzeiger, Drehpunkt (240, 240)

Diese fünf finalen PNG-Dateien liegen im Projekt vor (siehe Ordner `02_Bilder_und_Referenzdateien` in diesem Paket) und wurden über iterative SVG-Verfeinerung erarbeitet (siehe `MotorradDrehzahlanzeige_mit_ESP32_TEXTAUSZUG.md` für den vollständigen SVG-Entwicklungsdialog inkl. aller Korrekturschleifen zu Winkel-/Bogenberechnung, Farbverläufen im roten/rosa Warnband, Schriftgrößen etc.).

## Referenz-C++-Code (Ausgangsidee, LVGL v8, Arduino-Style)

Kompletter erster Entwurf für Screen-Switching per Touch, RPM-Zeigerberechnung, Shift-Light-Blinken ab 8900 RPM, und RTC-Anbindung. Aus Platzgründen hier gekürzt — der vollständige Code (inkl. `build_gui()`, `update_display_logic()`, `setup()`/`loop()`) findet sich im Begleitdokument `MotorradDrehzahlanzeige_mit_ESP32_TEXTAUSZUG.md`, das denselben Code in seiner finalen, im Chat konsolidierten Fassung enthält.

Kernideen:
- `enum ViewState { VIEW_RPM, VIEW_CLOCK }` zur Screen-Verwaltung
- `screen_click_event_cb()`: Touch-Tap wechselt zwischen RPM- und Uhr-Ansicht (`lv_scr_load_anim`, Fade-Übergang)
- `rpm_isr()`: Interrupt-Service-Routine zählt Kurbelwellenimpulse
- RPM-Zeigerwinkel-Formel: `angle = start_angle + (current_rpm * total_sweep / 9000)`, mit `lv_img_set_angle()` in Zehntel-Grad
- Shift-Light: Hintergrund blinkt bei ≥8900 RPM mit 5 Hz (100 ms an / 100 ms aus)
- RTC-Update: Stunden-/Minutenzeigerwinkel aus `DateTime`-Objekt berechnet, nur wenn Uhr-Screen aktiv

## Simulator-Feinabstimmung (Linux Mint, später gewählt)

Der Nutzer entschied sich im Chatverlauf explizit für Linux Mint statt WSL ("WSL ist mir doch etwas zu 'unfertig'"). Anleitung:

```bash
sudo apt update && sudo apt install -y build-essential cmake git libsdl2-dev
git clone --recursive https://github.com/lvgl/lv_port_pc_vscode.git
cd lv_port_pc_vscode
code .
# in main/src/main.c: MY_DISP_HOR_RES / MY_DISP_VER_RES auf 480 setzen
mkdir build && cd build
cmake ..
make -j$(nproc)
./main
```
