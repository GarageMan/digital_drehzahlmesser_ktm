# Bring-up-Test #1: PC-Telemetrie auf dem echten CrowPanel

Ziel: Das reale CrowPanel-2.1"-Board (ESP32-S3R8) zum ersten Mal in Betrieb
nehmen - unabhaengig von RPM/CAN/Motorrad. Zeigt eine simple, per Touch-Tap
**und** per Encoder (Drehen + Druecken) umschaltbare Anzeige von
PC-Kennzahlen (CPU-/GPU-Temperatur, Last, RAM), die per USB-Kabel vom PC
zum Board gestreamt werden. Testet damit genau die Mechanik, die du pruefen
wolltest: Touchscreen und den Dreh-/Druckmechanismus des Encoders.

Nicht Teil dieses Tests: CAN-Bus, Motorrad-Sensoren, die finale
Drehzahlmesser-UI (die liegt in `src/core/ui.c` und wird erst in einem
spaeteren Schritt mit echten Sensor-/CAN-Daten verbunden).

## 1. Einmalige Einrichtung (Arduino IDE)

1. **Board-Unterstuetzung**: ESP32-Arduino-Core installieren (Board Manager
   URL `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`),
   Version **2.0.14 fixieren** (siehe Sprint 2 - neuere Versionen koennten mit
   dem Elecrow-Beispielcode inkompatibel sein).
2. **Board auswaehlen**: passendes ESP32-S3-Board mit PSRAM (laut
   Elecrow-Doku "ESP32S3 Dev Module" o.ae.), dabei:
   - **PSRAM: OPI PSRAM** aktivieren (das Board hat 8MB PSRAM)
   - **Partition Scheme: "Huge APP (3MB No OTA/1MB SPIFFS)"** oder aehnlich
     grosszuegig - ein Elecrow-Forumsnutzer bekam mit dem Standardschema
     "Sketch too large (189%)", das Beispielprojekt braucht mehr Platz
     wegen LVGL + SquareLine-UI-Daten.
3. **Bibliothek LVGL Version 8.3.11** ueber den Library Manager installieren
   (exakte Version, laut Elecrow-Doku bestaetigt - passt zur `release/v8.3`,
   die auch der PC-Simulator in diesem Projekt nutzt).
4. **Offizielles Beispielprojekt besorgen**: GitHub-Repo
   [Elecrow-RD/CrowPanel-2.1inch-HMI-ESP32-Rotary-Display-480-480-IPS-Round-Touch-Knob-Screen](https://github.com/Elecrow-RD/CrowPanel-2.1inch-HMI-ESP32-Rotary-Display-480-480-IPS-Round-Touch-Knob-Screen),
   Branch `master`, Ordner `example/` (Arduino-Sketch heisst
   `RotaryScreen_2_1`). Dort ggf. weitere fuer den Sketch benoetigte
   Zusatzbibliotheken (Touch-/Panel-Treiber) laut dortiger Anleitung
   installieren.

## 2. Diesen Bring-up-Sketch zusammensetzen

`bringup_pc_telemetry.ino` in diesem Ordner enthaelt bereits die komplette
Anwendungslogik (Screens, Touch-Umschaltung, serielles Protokoll,
Encoder-Anbindungspunkte) - **aber bewusst keine** Display-/Touch-/
PCF8574-Initialisierung (siehe Kommentar oben in der Datei: das haette in
Sprint 1 schon einmal zu einer falschen, erfundenen Pin-Tabelle gefuehrt).

1. `RotaryScreen_2_1.ino` aus dem offiziellen Beispiel oeffnen.
2. Aus dessen `setup()`-Funktion den kompletten Block kopieren, der:
   - `Wire.begin(...)` mit den I2C-Pins aufruft,
   - den PCF8574-IO-Expander initialisiert (Touch-Reset/-Interrupt,
     LCD-Power/-Reset togglen),
   - das Display-Panel initialisiert (ST7701 RGB-Timing) und bei LVGL
     registriert,
   - den Touch-Treiber (CST8xx) bei LVGL registriert,
   - die Hintergrundbeleuchtung (PWM auf Pin 6) einschaltet.
3. Diesen Block an der mit `TODO` markierten Stelle in `setup()` von
   `bringup_pc_telemetry.ino` einfuegen (VOR `build_screens()`).
4. Aus dem Original-Sketch die Funktionen zur Encoder-Auswertung
   (Quadratur-Dekodierung auf Pin 42/4, Taster-Lesen ueber PCF8574 Bit 5)
   uebernehmen und in `poll_encoder_rotation()` / `poll_encoder_button()`
   einhaengen (dort `next_screen()`/`prev_screen()` aufrufen).
5. Kompilieren, hochladen: Board per USB-Kabel an den PC (Buchse
   "USB-5V-IN"), Boot-Taste gedrueckt halten, USB-Kabel einstecken/verbinden,
   dann in der Arduino IDE hochladen.

## 3. PC-Telemetrie senden

```bash
cd tools/pc_telemetry_sender
pip install -r requirements.txt
python3 pc_telemetry_sender.py /dev/ttyACM0   # oder z.B. COM5 unter Windows
```

Das Skript schickt einmal pro Sekunde eine Zeile wie
`CPU:47.2;GPU:63.5;LOAD:22.0;RAM:71.4` ueber den seriellen USB-Port. Der
serielle Port darf nicht gleichzeitig vom Arduino-IDE-Seriellen-Monitor
belegt sein (nur eine Anwendung kann den Port gleichzeitig oeffnen).

## 4. Was du damit testen kannst

- **Touch**: Auf das Display tippen -> Screen wechselt (CPU-Temp ->
  GPU-Temp -> Last/RAM -> wieder von vorn).
- **Encoder drehen**: sobald Schritt 2.4 ausgefuellt ist, wechselt Drehen
  ebenfalls den Screen.
- **Encoder druecken**: testet den Druckmechanismus, den du als "wacklig"
  beschrieben hast - unabhaengig von der spaeteren Entscheidung, ob der
  Encoder am Motorrad ueberhaupt zum Einsatz kommt.
- **Serielle Datenuebernahme**: Werte aktualisieren sich etwa sekuendlich;
  bleibt der PC-Sender laenger als 3 Sekunden stumm (z.B. Skript beendet),
  zeigt das Display wieder "--" statt eines eingefrorenen alten Werts.

## 5. Offene Punkte

- Die exakten Funktions-/Variablennamen im offiziellen Beispiel sind hier
  nicht aufgefuehrt (nicht oeffentlich als Rohtext zugaenglich verifiziert) -
  beim Kopieren in Schritt 2 ggf. Namen anpassen, nicht raten.
- UART1-GPIOs (fuer den spaeteren CAN-Transceiver) sind separat bei Elecrow
  angefragt, siehe `Sprint3_Code_Grundgeruest.md` Abschnitt 6/7 im Projekt -
  fuer diesen Bring-up-Test nicht erforderlich.
