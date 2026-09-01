# KTM-Bring-up: Drehzahlmesser + Uhr auf echter Hardware

Zeigt `core/ui.c` (Drehzahlmesser mit Nadel-Sweep + Shift-Light, Analoguhr,
Touch-Umschaltung) auf dem echten CrowPanel-2.1"-Board (ESP32-S3R8) - mit
simulierten Werten (automatischer RPM-Sweep, laufende Uhr), nicht mit
echten Fahrzeugsensoren. Ziel: verifizieren, dass die Anzeige selbst auf
der realen Hardware korrekt aussieht, bevor echte Sensoren/CAN angebunden
werden (siehe Sprint3_Code_Grundgeruest.md, Punkt 9 "Offene Punkte").

## Voraussetzung: Checkpoint bereits erledigt

Dieser Sketch setzt voraus, dass das offizielle `RotaryScreen_2_1`-Beispiel
schon einmal unveraendert erfolgreich auf dem Board lief (siehe Sprint 3,
Nachtrag 22.08. - das war bereits erfolgreich). Insbesondere:

- Arduino-IDE-Board-Paket "esp32 by Espressif Systems", Version **2.0.17**
- Board "ESP32S3 Dev Module", **Partition Scheme: Huge APP**, **PSRAM: OPI PSRAM**
- Bibliotheken aus dem offiziellen Elecrow-Repo (Ordner `example/Arduino/libraries/`):
  `Arduino_GFX_Library` (GFX_Library_for_Arduino), `Adafruit_CST8XX_Library`,
  `Adafruit_BusIO`, `PCF8574_library-master`, und **lvgl in Version 9.1.0**
  (aus demselben Ordner - **nicht** die in Sprint 2 noch dokumentierte 8.3.11,
  siehe Sprint3_Code_Grundgeruest.md Abschnitt "LVGL-Versionswechsel")

Falls das noch nicht gemacht wurde: erst `RotaryScreen_2_1.ino` aus
<https://github.com/Elecrow-RD/CrowPanel-2.1inch-HMI-ESP32-Rotary-Display-480-480-IPS-Round-Touch-Knob-Screen>
(Branch `master`, Ordner `example/Arduino/RotaryScreen_2_1/`) testen.

## Zusaetzlich fuer diesen Sketch: eigene UI-Bibliothek installieren

`core/ui.c` wird nicht kopiert, sondern als Arduino-Bibliothek eingebunden
(Grund: Arduino IDE kompiliert nur Dateien im Sketch-Ordner selbst oder in
ordnungsgemaessen Bibliotheken - siehe Kommentar in
`tools/prepare_esp32_library.sh`).

1. Im Projekt-Wurzelverzeichnis (`03_Code/`):
   ```bash
   bash tools/prepare_esp32_library.sh
   ```
2. Den entstandenen Ordner `src/platform/esp32/libraries/ktm_ui` in den
   Arduino-`libraries`-Ordner kopieren (Speicherort siehe Arduino IDE ->
   Einstellungen -> "Sketchbook-Speicherort", darin der Unterordner
   `libraries`).
3. Arduino IDE neu starten (oder Bibliotheken neu laden), damit `ui.h` und
   `vehicle_data.h` gefunden werden.
4. Bei Aenderungen an `core/ui.c`/`vehicle_data.h`/den Bilddateien: Schritt 1
   und 2 wiederholen (die Bibliothek ist eine generierte Kopie, keine
   eigene Quelle - `03_Code/src/platform/esp32/libraries/` ist deshalb in
   `.gitignore`).

## Hochladen

Wie beim Checkpoint-Test: Boot-Taste gedrueckt halten, USB-Kabel in die
Buchse "USB-5V-IN" stecken/verbinden, in der Arduino IDE hochladen.

## Was du damit testen kannst

- **Drehzahlmesser-Screen**: Nadel schwingt automatisch 0 -> 9500 -> 0
  U/min (ca. 3000 U/min/s, wie im PC-Simulator). Ab 8900 U/min blinkt der
  Hintergrund rot (Shift-Light, 5Hz).
- **Touch**: Ein Tap auf das Display schaltet zwischen Drehzahlmesser und
  Analoguhr um (Uhr laeuft 60x Realzeit, wie im PC-Simulator, damit der
  Minutenzeiger sichtbar wandert, ohne eine Stunde zu warten).
- **Seriell-Log** (115200 Baud): Boot-Meldungen und PCF8574-/Touch-Init-
  Status, analog zum bisherigen Bring-up-Test.

## Bewusst NICHT Teil dieses Tests

- **Encoder** (Drehen/Taste): Pins sind bestaetigt (siehe
  Sprint3_Code_Grundgeruest.md Abschnitt 6/7), aber fuer "Touch-Umschaltung"
  laut Aufgabenstellung nicht noetig - spart Komplexitaet/Fehlerflaeche in
  diesem ersten Durchlauf. Naechster moeglicher Schritt, falls gewuenscht.
- **Echte Sensoren/CAN**: `sweep_provider_*` in `ktm_bringup.ino` liefert
  rein simulierte Werte. Der Wechsel zu echten Sensoren betrifft laut
  Architekturprinzip (README.md, Abschnitt "Architektur") nur einen neuen
  Provider - `core/ui.c` bleibt unveraendert.
- **UART1/CAN-Pegelwandler**: siehe Sprint 3, Nachtrag 22.08. (UART-Signale
  sind 5V, ESP32-S3 vertraegt nur 3,3V) - fuer diesen reinen Display-Test
  nicht relevant.

## Nicht getestet (ehrlich gesagt)

Dieser Sketch wurde anhand der bestaetigten Init-Sequenz aus
`RotaryScreen_2_1.ino` und der v9-API-Referenz (LVGL-9.1.0-Quellcode)
zusammengesetzt, konnte aber in der Entwicklungsumgebung, in der dieser
Sketch entstanden ist, **nicht selbst kompiliert/geflasht werden** (kein
ESP32-Toolchain dort verfuegbar). Der PC-Simulator (`03_Code/README.md`)
wurde dagegen soweit wie moeglich gegen die reale v9.1.0-API gegengeprueft,
ebenfalls aber nicht in dieser Umgebung tatsaechlich gebaut (kein
CMake/gcc/SDL2 dort installiert) - bitte beim ersten Durchlauf (PC-Sim wie
ESP32) auf Kompilierfehler gefasst sein und melden.
