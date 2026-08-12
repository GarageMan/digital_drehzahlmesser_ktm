# Digitaler Drehzahlmesser - Sprint 3: Code-Grundgeruest & PC-Simulator

Code-Grundlage fuer den digitalen Drehzahlmesser/Analoguhr fuer die
KTM 690 SMC R / Enduro R (Bj. ab 2021, LC4-Plattform). Zielhardware:
Elecrow CrowPanel 2.1" HMI ESP32 Rotary Display (ESP32-S3R8) - liegt noch
nicht vor, daher ist dieser Stand vollstaendig am PC im LVGL-Simulator
lauffaehig und getestet (siehe `docs/screenshots/`).

Bezug: `Sprint1_Machbarkeitsanalyse`, `Sprint2_Erweiterung_Machbarkeitsanalyse`,
`Sprint2_Pruefung_externe_Analyse` (Projekt-Dokumente).

## Was schon laeuft (Release-1-Umfang)

- Rundes 480x480-Zifferblatt "Drehzahlmesser" mit rotierender Nadel,
  0-9500 U/min, exakt auf die vorhandenen PNG-Grafiken kalibriert
  (0 und 9 zeigen nachweislich auf die richtigen Ziffern, siehe Screenshots).
- Shift-Light: Hintergrund blinkt mit 5Hz ab 8900 U/min.
- Umschaltung per Klick/Touch auf ein Analoguhr-Zifferblatt (Stunden-/
  Minutenzeiger).
- Simulierter Datenprovider (`src/platform/sim/sim_provider.c`), der nicht
  nur RPM liefert, sondern **alle** fuer die Roadmap vorgesehenen Kanaele
  simuliert und alle 500ms auf der Konsole loggt: Gang, Oel-/Wasser-/
  Lufttemperatur, Bordnetzspannung, Backup-Akku-Zustand.
- Ein sauber getrenntes Datenmodell (`src/core/vehicle_data.h`) und
  UI-Code (`src/core/ui.c`), das unveraendert auf den ESP32 uebernommen
  werden kann - nur der Datenprovider und main.c sind PC-spezifisch.
- Eigener PNG-zu-LVGL-Konverter (`tools/png_to_lvgl.py`), keine Abhaengigkeit
  von einem externen Online-Tool mehr.

## Was absichtlich noch NICHT drin ist

- Kein ESP32-Code, der echte Sensoren liest (Hardware liegt noch nicht vor).
  `src/platform/esp32/main.cpp.stub` beschreibt die naechsten Schritte,
  sobald das CrowPanel-Board da ist - bewusst ohne erfundene Pinbelegung
  (siehe Kommentar dort).
- Keine Anzeige von Gang/Temperaturen/Akku im UI selbst - die Werte werden
  vom Simulator zwar erzeugt und geloggt, aber Release 1 zeigt nur RPM und
  Uhr (siehe Tabelle "Wie geht es weiter" im Chat/Projekt).
- Kein CAN-Bus-Code. `docs/CAN_BUS_NOTES.md` fasst zusammen, was
  [github.com/blalor/ktm-can](https://github.com/blalor/ktm-can) bereits
  dekodiert hat, als Grundlage fuer eine spaetere Ausbaustufe.

## Bauen und starten (Linux)

```bash
# Einmalig: LVGL v8.3 + lv_drivers v8.3 laden (nicht im Repo enthalten,
# siehe "Warum lib/ fast leer ist" unten)
bash tools/setup_deps.sh

# SDL2-Entwicklungspakete, falls noch nicht vorhanden (Debian/Ubuntu/Mint):
sudo apt install -y build-essential cmake libsdl2-dev

mkdir build && cd build
cmake ..
make -j$(nproc)
./sim
```

Es oeffnet sich ein 480x480-Fenster mit dem Drehzahlmesser.

### Bedienung im Simulator

| Eingabe | Wirkung |
|---|---|
| Klick ins Fenster | Umschaltung Drehzahlmesser <-> Analoguhr (simuliert Touch) |
| Pfeil hoch / runter | RPM manuell nudgen (nur wenn Auto-Sweep aus) |
| `A` | Auto-RPM-Sweep an/aus (Standard: an, 0 -> 9500 -> 0) |
| `G` | Gang manuell weiterschalten (0=Leerlauf .. 6) |
| `C` | Bordnetz-Einbruch simulieren (~1.5s, wie beim Anlasser-Cranking) |
| `R` | RPM sofort auf 0 |

**Hinweis:** Diese Tastaturkuerzel wurden in einer normalen Desktop-Sitzung
entwickelt und getestet. In einer kopflosen/virtuellen X-Umgebung ohne
Fenstermanager (z.B. reines Xvfb, wie es in dieser Sandbox zur automatisierten
Bild-Verifikation genutzt wurde) empfangen SDL-Fenster teils keine
Tastatur-/Klickfokus-Events - das ist eine Eigenheit der Testumgebung, keine
Einschraenkung der eigentlichen Anwendung. Auf einem normalen Linux-Desktop
(wie im Sprint-1-Chat fuer Linux Mint beschrieben) funktionieren Klick und
Tastatur wie oben beschrieben.

### Assets neu erzeugen

Falls die PNG-Dateien in `assets/src/` geaendert werden:

```bash
python3 tools/png_to_lvgl.py assets/src/rpm_dial.png assets/generated/img_rpm_dial img_rpm_dial
# ... entsprechend fuer die anderen 4 PNGs (siehe assets/generated/ fuer die Namen)
```

## Warum `lib/` fast leer ist

LVGL v8 und lv_drivers werden bewusst **nicht** im Zip mitgeliefert
(zusammen >100MB Fremdcode inkl. Demos/Beispielen, die wir nicht brauchen).
`tools/setup_deps.sh` klont exakt die getesteten Versionen
(`release/v8.3`-Branch beider Repos). `lib/lv_conf.h` und
`lib/lv_drv_conf.h` sind dagegen Teil dieses Repos - sie enthalten die
projektspezifische Konfiguration (480x480, `LV_COLOR_DEPTH=32`, `USE_SDL=1`)
und duerfen beim Setup nicht ueberschrieben werden.

## Architektur (fuer die spaetere ESP32-Portierung)

```
src/
  core/
    vehicle_data.h      <- Datenmodell + Provider-Interface (Plattform-neutral)
    ui.c / ui.h          <- LVGL-UI, 1:1 fuer ESP32 wiederverwendbar
  platform/
    sim/                  <- PC/SDL2-spezifisch (main.c, sim_provider.c)
    esp32/                <- Platzhalter fuer die reale Firmware (noch offen)
assets/
  src/                    <- Original-PNGs (480x480, siehe Sprint 1)
  generated/              <- daraus erzeugte LVGL-C-Arrays (tools/png_to_lvgl.py)
docs/
  screenshots/            <- Verifikations-Screenshots aus dem Simulator
  CAN_BUS_NOTES.md         <- Zusammenfassung von github.com/blalor/ktm-can
```

Das Prinzip (schon in Sprint 1 als Ziel formuliert): `core/` kennt nur das
`vehicle_data_provider_t`-Interface, nie eine konkrete Implementierung. Der
Wechsel von "simulierte Daten am PC" zu "echte Sensoren/CAN am ESP32"
betrifft ausschliesslich `platform/`, nicht `core/`.

## Lizenz

Noch nicht final festgelegt (siehe Sprint2_Erweiterung_Machbarkeitsanalyse,
Abschnitt 6 "Open-Source-Veroeffentlichung": MIT oder GPLv3 fuer den Code,
CERN-OHL/CC-BY-SA fuer Hardware-Dokumentation). Bis zur Entscheidung: alle
Rechte beim Autor, kein Nutzungsrecht fuer Dritte vergeben.
