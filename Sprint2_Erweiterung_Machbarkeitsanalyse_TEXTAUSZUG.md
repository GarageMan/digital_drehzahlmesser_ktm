> **Hinweis:** Diese Datei ist eine reine Text-Extraktion aus dem Original-Dokument "Sprint2_Erweiterung_Machbarkeitsanalyse.docx" im Projekt. Das Original-Word-Dokument mit seiner Formatierung konnte über die verfügbaren Werkzeuge nicht als Binärdatei abgerufen werden (nur der Textinhalt war zugänglich). Das Original mit vollständiger Formatierung kann direkt aus dem Projekt in der Claude-App heruntergeladen werden.

---

# Sprint 2 – Erweiterung der Machbarkeitsanalyse

## Digitaler Drehzahlmesser für KTM 690 (LC4-Plattform, Bj. 2021)

*Projekt: Digitaler Drehzahlmesser · Stand: 06.08.2026 · Bezug: Sprint1_Machbarkeitsanalyse.docx*

# 1. Zusammenfassung

In Sprint 2 wurde die Hardware-Frage aus Sprint 1 final geklärt und der Funktionsumfang des Projekts erweitert. Als Basis-Hardware wurde das

Elecrow CrowPanel 2,1" HMI ESP32 Rotary Display (ESP32-S3R8) festgelegt. Dieses Board löst beide in Sprint 1 identifizierten kritischen Blocker: Es nutzt ein SPI/RGB-Interface (kein MIPI, damit vom ESP32-S3 ansteuerbar) und bringt einen bestätigten, integrierten kapazitiven Touchscreen mit – zusätzlich einen Drehknopf, der als optionale Zweitbedienung nutzbar ist.

Die Anbindung des Drehzahlsignals über einen Optokoppler wurde als unkritisch eingestuft. Neu hinzugekommen ist der Wunsch, das Projekt um weitere aus dem Motorrad auslesbare Betriebsdaten zu erweitern (Ganganzeige, Öltemperatur, perspektivisch CAN-Bus-Daten) und es anschließend als Open-Source-Projekt zu veröffentlichen.

# 2. Hardware-Entscheidung: Elecrow CrowPanel 2,1"

- Display: 480×480 Pixel, rund, IPS

- Touch: kapazitiv, werksseitig bestätigt (löst Befund 4.2 aus Sprint 1)

- Zusatzbedienung: mechanischer Drehknopf (Rotary Encoder) – für dieses Projekt optional, z. B. als Fallback-Umschaltung Drehzahlmesser/Uhr

- Controller: ESP32-S3R8 fest verbaut auf dem Board – kein separater Verkabelungsaufwand Display↔Controller

- Interface: SPI/RGB-parallel – kompatibel zur Grafik-Hardware des ESP32-S3 (löst Befund 4.1 aus Sprint 1, kein MIPI-DSI/ESP32-P4 nötig)

Damit ist das Projekt aus Display-/Controllersicht bestellreif. Die in Sprint 1 unter 4.3 genannte Backlight-Spannungsfrage (19–20 V beim ursprünglich geplanten TTC248XRS-01) entfällt, da das CrowPanel-Board die Backlight-Ansteuerung bereits integriert hat – vor Bestellung dennoch einen Blick ins Datenblatt werfen, um die Versorgungsspannung des Gesamtboards (i. d. R. 5 V über USB-C) zu bestätigen.

# 3. ESP32-S3 vs. ESP32-P4 – Einordnung der Entscheidung

Der ESP32-S3 ist dem ESP32-P4 in reiner Rechen- und Grafikleistung unterlegen (240 MHz vs. 400 MHz, Software- statt Hardware-Grafikbeschleunigung), für ein 480×480-Zifferblatt mit rotierenden Nadelbildern über LVGL jedoch ausreichend dimensioniert. Der S3 bringt zudem WLAN/Bluetooth fest integriert mit (praktisch für spätere OTA-Updates oder Log-Auslese per App), während der P4 dafür einen externen Chip benötigen würde. Die Wahl des CrowPanel-Boards (S3-basiert) ist damit für den Funktionsumfang dieses Projekts stimmig.

# 4. Drehzahlsignal

Die Anbindung des Kurbelwellensignals über einen Optokoppler zur galvanischen Trennung und Pegelanpassung wurde als unproblematisch bestätigt. Offen bleibt weiterhin (siehe Sprint 1, Befund 4.4/4.5): Sensortyp (induktiv/Hall) und Impulszahl pro Umdrehung sind vor dem elektrischen Eingriff anhand des KTM-Werkstatthandbuchs zu verifizieren, ebenso die RPM-Berechnungsformel nach dem ersten Anschluss gegen eine Referenzdrehzahl zu kalibrieren.

# 5. Erweiterung: zusätzliche Fahrzeugdaten

Die LC4-Plattform (690 Enduro R / SMC R / Duke, Bj. ab ca. 2016) bietet mehrere Ansatzpunkte für zusätzliche Anzeigewerte. Die Herausforderungen sind jeweils primär mechanisch/elektrisch, nicht im Controller-Code:

## 5.1 Ganganzeige – geringer bis mittlerer Aufwand

- Die 690er verfügt ab Werk über einen vollständigen Gangsensor an der Schaltwalze (4- oder 6-poliger Stecker), der pro Gang einen festen, unterschiedlichen Spannungswert ausgibt.

- Auswertung über einen ADC-Pin des ESP32 möglich, ggf. mit Spannungsteiler zur Pegelanpassung.

- Einmalige Kalibrierung nötig: Spannungswerte den Gängen 1–6 zuordnen.

- Kommerzielle Zusatzmodule (z. B. "CUMPAN") bestätigen, dass dieses Signal grundsätzlich sauber auswertbar ist – guter Referenzpunkt für die eigene Kalibrierkurve.

## 5.2 Öltemperatur – zusätzlicher Sensor erforderlich

- Werksseitig ist an der LC4 kein Öltemperatursensor verbaut – nur ein Kühlwasserfühler.

- Nachrüstlösung über einen Einschraub-NTC-Fühler (z. B. M10×1-Gewinde), der anstelle des Ölmessstabs bzw. am Ölfilterdeckel/Ölkreislauf montiert wird – in der KTM-Community ein bekanntes, bereits mehrfach umgesetztes Vorhaben.

- Elektrisch unkritisch: NTC per Spannungsteiler direkt an einem ADC-Pin des ESP32 auswertbar. Der Aufwand liegt im mechanischen Teil (passende Bohrung/Adapter finden, Dichtheit sicherstellen).

## 5.3 CAN-Bus – interessant, aber eigenständiges Teilprojekt

- Unter der Sitzbank befindet sich ein 6-poliger Sumitomo-Diagnosestecker mit 12 V, Masse, CAN-High und CAN-Low. Ab Euro4/5-Modellen (die 2021er ist Euro5) liegt hier aktiver CAN-Verkehr an.

- Technisch anbindbar über einen CAN-Transceiver (z. B. MCP2515 oder TJA1050) am ESP32.

- Einschränkung: KTM veröffentlicht keine offizielle Signal-/DBC-Dokumentation. Die Zuordnung der CAN-IDs zu Messwerten (Kühlwassertemperatur, Geschwindigkeit, Batteriespannung, ggf. Fehlercodes) müsste per Reverse-Engineering erfolgen (Vergleichsmessungen, Tools wie SavvyCAN). Realistisch als späterer Ausbauschritt (Sprint 3+), nicht als Bestandteil der Erstversion.

## 5.4 Weitere einfache Ergänzungen

- Batteriespannung: trivial per Spannungsteiler direkt vom Bordnetz.

- Kühlwassertemperatur: Signal ggf. parallel am Originalfühler abgreifbar (Impedanz beachten, ggf. Pufferstufe).

# 6. Open-Source-Veröffentlichung

Für eine Veröffentlichung sind neben dem eigentlichen Code einige Punkte sinnvoll vorzubereiten:

- Lizenzwahl: z. B. MIT (permissiv, einfache Nachnutzung) oder GPLv3 (copyleft) für den Code; für Hardware-Dokumentation (Schaltpläne, BOM) bietet sich zusätzlich CERN-OHL oder CC-BY-SA an.

- Repository-Struktur: Firmware (ESP32/LVGL), KiCad- oder Fritzing-Schaltplan der Sensorbeschaltung (Optokoppler, Gangsensor-Spannungsteiler, NTC-Beschaltung), Stückliste (BOM) mit Bezugsquellen, PNG/SVG-Assets, Aufbauanleitung.

- Sicherheitshinweis für Nachnutzer: Eingriffe in die Fahrzeugelektrik (Kurbelwellensensor, Gangsensor, Diagnosestecker) und der Einbau eines Öltemperaturfühlers greifen in sicherheitsrelevante bzw. bauartgenehmigte Systeme ein. Ein Hinweis auf Eigenverantwortung, Gewährleistungsausschluss und – je nach Land – die Klärung der Zulassungsrelevanz (in Deutschland z. B. StVZO-Bezug bei Änderungen an fahrzeugsicherheitsrelevanter Elektrik) gehört in die README.

- Versionierung passend zu den Sprints (v0.1 Simulator, v0.2 Drehzahl+Uhr, v0.3 Gang, v0.4 Öltemperatur, …).

# 7. Nächste Schritte (Sprint 3)

- CrowPanel-Board bestellen, LVGL-Beispielprojekt des Herstellers als Ausgangsbasis prüfen.

- Vorhandene PNG-Assets und Zeiger-Rotationscode aus Sprint 1 auf das CrowPanel-Board portieren (PC-Simulation weiterhin vorgeschaltet).

- KTM-Werkstatthandbuch beschaffen: Sensortyp/Impulszahl Kurbelwelle, Steckerbelegung Gangsensor verifizieren.

- Gangsensor-Spannungswerte am stehenden Fahrzeug messen und Kalibriertabelle erstellen.

- Passenden Öltemperaturfühler (Gewinde/Einbauort) für die LC4 auswählen.

- Repository-Grundgerüst (Lizenz, README, Ordnerstruktur) für die Open-Source-Veröffentlichung anlegen.
