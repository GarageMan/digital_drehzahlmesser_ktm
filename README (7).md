# Digitaler Drehzahlmesser – Gesamtpaket (Stand: 12.08.2026)

Dieses Paket enthält alle bisher im Projekt "Digitaler Drehzahlmesser" (KTM 690 Enduro R/SMC R, Bj. 2021) erzeugten Dokumente, Bilder und den aktuellen Code-Stand, gebündelt zum Offline-Zugriff.

## Ordnerstruktur

### 01_Projektdokumente/
Die Sprint-Dokumente und der Chatverlauf:

- `Sprint1_Machbarkeitsanalyse.docx` / `.pdf` — Original-Dateien (echte Word-/PDF-Binärdatei, im Projekt erzeugt)
- `Sprint2_Pruefung_externe_Analyse.docx` / `.pdf` — Original-Dateien (echte Word-/PDF-Binärdatei, im Projekt erzeugt)
- `Sprint2_Erweiterung_Machbarkeitsanalyse_TEXTAUSZUG.md` — **Text-Extrakt** (siehe Hinweis unten)
- `Sprint3_Code_Grundgeruest.md` — aktueller Stand aller Sprint-3-Notizen (Backup-Akku, Feature-Tabelle, Steckverbinder-Details, Elecrow-Support-Anfrage, Verbindungsskizze, Bring-up-Test, Erstinbetriebnahme-Anleitung, ESP32-Core-Versionsklärung)
- `Chatverlauf_Drehzahlmesser_KTM690.md` — manuell rekonstruierter früherer Chatverlauf (User-Original-Dokument)
- `Digitale_Drehzahlanzeige_Ausgangsidee_TEXTAUSZUG.md` — **Text-Extrakt** der ursprünglichen Projekt-Idee (siehe Hinweis unten)

### 02_Bilder_und_Referenzdateien/
- `hour_needle.png`, `clock_dial.png`, `min_needle.png`, `rpm_needle.png`, `rpm_dial.png` — Original-Bilddateien (Zifferblatt- und Zeiger-Grafiken für die LVGL-Anzeige, identisch mit den im Code-Projekt unter `03_Code/assets/src/` verwendeten Dateien)
- `MotorradDrehzahlanzeige_mit_ESP32_TEXTAUSZUG.md` — **Text-Extrakt** (siehe Hinweis unten)
- `TTC248XRS01_TEXTAUSZUG.md` — **Text-Extrakt** des offiziellen Hersteller-Datenblatts (siehe Hinweis unten)

### 03_Code/
Vollständige Kopie des aktuellen Code-Projekts `ktm-drehzahlmesser` (PC-Simulator, ESP32-Bring-up-Sketch, Python-Telemetrie-Tool, Verbindungsskizze, generierte LVGL-Assets). Ausgeschlossen wurden die großen Drittanbieter-Bibliotheken `lib/lvgl/` und `lib/lv_drivers/` (per `tools/setup_deps.sh` selbst nachladbar) sowie Build-Artefakte (`build/`, `__pycache__/`). Siehe `03_Code/README.md` für den Projektüberblick.

## Wichtiger Hinweis zu den "TEXTAUSZUG"-Dateien

Für vier der ursprünglich als Word-/PDF-Dokumente im Projekt abgelegten Dateien konnte über das verwendete Werkzeug nur der extrahierte **Textinhalt**, nicht die Original-Binärdatei mit Formatierung, abgerufen werden:

- `Sprint2_Erweiterung_Machbarkeitsanalyse.docx`
- `Digitale Drehzahlanzeige.docx`
- `MotorradDrehzahlanzeige mit ESP32.pdf`
- `TTC248XRS01.pdf`

Diese vier Dokumente sind daher in diesem Paket als reine `.md`-Textdateien (mit dem Namenszusatz `_TEXTAUSZUG`) enthalten — inhaltlich vollständig, aber ohne die ursprüngliche Formatierung, Tabellenlayouts oder eingebettete Diagramme/Zeichnungen. Wer die Originaldateien mit vollständiger Formatierung benötigt, findet sie unverändert im Projekt "Digitaler Drehzahlmesser" direkt in der Claude-App (Download dort möglich).

Für die übrigen zwei Dokumente (`Sprint1_Machbarkeitsanalyse`, `Sprint2_Pruefung_externe_Analyse`) sowie alle fünf Bilddateien liegen echte Original-Binärdateien in diesem Paket bei.

## Projektstatus (Kurzüberblick)

- **Hardware entschieden:** Elecrow CrowPanel 2,1" HMI ESP32 Rotary Display (ESP32-S3R8), physisch bereits vorhanden
- **Aktueller Schritt:** Erstinbetriebnahme über Arduino IDE, offizielles Elecrow-Beispielprojekt als Basis, eigener Bring-up-Test mit PC-Telemetrie (CPU-/GPU-Temperatur) zur Prüfung von Touch- und Encoder-Mechanik
- **Offen:** Antwort von Elecrow-Support zu den exakten UART1-GPIO-Pins (für spätere CAN-Anbindung), danach Fortsetzung der CAN-Bus-Integration (SN65HVD230-Transceiver bzw. interimsweise Tests mit dem bereits vorhandenen Jhoinrch RH02 USB-to-CAN-Adapter am PC)

Details siehe `01_Projektdokumente/Sprint3_Code_Grundgeruest.md`.
