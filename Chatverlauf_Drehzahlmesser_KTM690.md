# Chatprotokoll: Digitaler Drehzahlmesser für KTM 690 SMC R

Projekt-Chat mit Claude – Export vom 07.08.2026
(Reine Textsicherung des Gesprächsverlaufs zur lokalen Aufbewahrung, falls der Sync zwischen den Claude-Clients Lücken hat.)

---

## Nutzer:
Bitte prüfe die Machbarkeit meiner Projektidee eines digitalen Drehzahlmessers für eine KTM 690 SMCR (Bj2021)

## Claude:
Kurz vorweg: Die bisherige Analyse in diesem Projekt bezieht sich auf eine KTM 690 Enduro R (Bj. 2021) – du fragst jetzt nach der 690 SMC R. Beide nutzen denselben LC4-Einzylindermotor, daher dürften Kurbelwellensensor und Signal identisch sein, aber das wäre vor dem Eingriff zu verifizieren.

Gesamtergebnis: Konzeptionell machbar, aber noch nicht bestellreif.

Was gut ist:
- Die 5 PNG-Grafiken (Zifferblätter, Zeiger) sind korrekt vorbereitet (480×480, richtiger Drehpunkt)
- PC-Simulation vor Hardwarekauf (LVGL-Simulator) ist sinnvoll und möglich
- Schutzkonzept fürs Bordnetz (Step-Down-Wandler, Diode+Kondensator) ist fachlich richtig
- RTC-Konzept (DS3231) für die Uhr-Funktion ist Standard

Zwei kritische Blocker vor dem Kauf:

1. Display-Interface stimmt nicht. Das Datenblatt des TTC248XRS-01 weist ein MIPI-DSI-Interface aus (2-Lane, D0P/D0N etc.), nicht den in der bisherigen Doku angenommenen 16-Bit-Parallelbus. Der empfohlene ESP32-S3 hat aber keine MIPI-DSI-Hardware – das ist nur beim ESP32-P4 vorhanden. → Vor Kauf klären: gibt es eine RGB-Parallel-Variante des Displays, oder muss auf ESP32-P4 gewechselt werden?

2. Touch unbestätigt. Datenblatt sagt „Without CTP" (ohne Touchscreen), obwohl dein Bedienkonzept (Tippen zum Umschalten Drehzahlmesser/Uhr) Touch voraussetzt. → Beim Verkäufer schriftlich bestätigen lassen, sonst Taster als Fallback einplanen.

Weitere Punkte:
- Backlight braucht ~19–20V (nicht 5V wie in der Doku angenommen) → Boost-Konverter nötig
- Kurbelwellensignal (Sensortyp, Impulszahl/Umdrehung) und RPM-Formel im Beispielcode sind unverifizierte Annahmen

Nächste Schritte: Interface-Variante und Touch-Bestückung beim Displayverkäufer klären, Controller danach final festlegen, KTM-Werkstatthandbuch für Sensordetails prüfen, parallel den LVGL-Simulator mit vorhandenen Assets testen.

Soll ich die vorhandene Sprint-1-Analyse aktualisieren (SMC R statt Enduro R) und/oder das Datenblatt nochmal gezielt nach einer RGB-Variante des Displays durchsuchen?

---

## Nutzer:
Die Baureihe der KTM ist egal, seitens Motor und Elektrik sind sie identisch.

Wie unterscheidet sich der S3 vom P4, bzw. welcher ist der stärkere?

Das Display muss eine Touch-Funktion haben. Siehst du Alternativem in der Baugröße 2 bis 2,8 Inches?

## Claude:
Gut, dann bleibt die bisherige Analyse für Motor/Sensorik unverändert gültig.

**1. ESP32-S3 vs. ESP32-P4 – welcher ist stärker?**

Der P4 ist deutlich stärker, aber beide haben unterschiedliche Stärken:

| | ESP32-S3 | ESP32-P4 |
|---|---|---|
| CPU | 240 MHz, Dual-Core | 400 MHz, Dual-Core (RISC-V), ~2,5× schneller |
| Grafik | Nur Software-Rendering (CPU zeichnet jeden Pixel) | Hardware-2D-Beschleuniger (PPA) – entlastet CPU |
| Display-Interface | SPI/QSPI, paralleles RGB/8080 | MIPI-DSI (nativ für moderne Rund-/Touchdisplays) |
| WLAN/Bluetooth | Eingebaut | Fehlt komplett – bräuchte externen Chip (z. B. ESP32-C6) |
| Empfehlung für 480×480 | Läuft gut, bei komplexen Animationen ggf. Ruckler | Läuft butterweich, aber "overkill" für dieses Projekt |

Für deine Anwendung (ein rundes 480×480-Zifferblatt mit zwei drehenden Zeigern, Touch-Umschaltung) ist die reine Rechenleistung des P4 nicht zwingend nötig – LVGL mit einfachen Nadel-Rotationen läuft auf dem S3 auch bei 480×480 flüssig genug. Der P4 wäre nur nötig, wenn du am MIPI-Display TTC248XRS-01 festhältst. Da der S3 kein WLAN/Bluetooth extra braucht (falls du das später für OTA-Updates willst), ist er unterm Strich das praktischere Bauteil – sofern das Display eine SPI/RGB-Variante hat.

**2. Alternative Touch-Displays, 2–2,8", SPI/RGB (kein MIPI):**

- Adafruit TL021WVC02CT-B1323 – 2,1", 480×480, RGB666-TTL-Interface, kapazitiver Touch. War schon in eurer ursprünglichen Doku als Alternative genannt und passt am besten: gleiche Auflösung wie das TTC248XRS-01, aber mit bestätigtem Touch und einem Interface, das der ESP32-S3 direkt kann.
- VIEWE / Elecrow "CrowPanel" 2,1" Knob Display – 480×480, IPS, rund, Touch + Drehknopf, mit ESP32-S3 bereits integriert (fertiges Board, kein separater Displaykauf/Verkabelung nötig). Der Zusatz-Drehknopf wäre für dich unnötig, aber praktisch als All-in-one-Lösung.

Beide sind SPI/RGB-basiert und vom S3 direkt ansteuerbar – kein Wechsel auf den P4 nötig. Ich würde vor dem Kauf trotzdem das aktuelle Datenblatt der Adafruit-Platine gegenprüfen, um die 19–20V-Backlight-Frage (Punkt 4.3 aus der Analyse) zu klären.

Soll ich die Sprint-1-Analyse mit diesen beiden Alternativen und der S3/P4-Entscheidung aktualisieren?

---

## Nutzer:
Es wird der Elecrow CrowPanel 2,1" HMI-ESP32-Rotary-Display, IPS, ESP32-S3R8 als Basis. Die Anbindung des Drehzahlsignals (via Optokoppler) sollte keine Herausforderung darstellen. Interessant ist, was noch alles für Daten aus der KTM ausgelesen werden könnten (Gangsnzeige, Öltemperatur über entsprechenden Fühler, etc.)

## Claude:
Guter Punkt – die 690er (Bj. 2021, Euro5) hat einiges an Sensorik, aus der sich zusätzliche Anzeigewerte gewinnen lassen. Kurzer Überblick nach Aufwand sortiert:

**Direkt am Diagnosestecker (CAN-Bus) – mittlerer Aufwand:**
- Unter der Sitzbank sitzt ein 6-poliger Sumitomo-Diagnosestecker mit 12V, Masse, CAN-High, CAN-Low. Ab Euro4/5-Modellen (deine 2021er ist Euro5) liegt hier echter CAN-Verkehr an.
- Ein CAN-Transceiver (z. B. MCP2515 oder TJA1050) am ESP32 könnte diesen Bus passiv mitlesen.
- Haken: KTM veröffentlicht keine offizielle DBC-Datei (Signalzuordnung). Die IDs müssten per Reverse-Engineering (z. B. mit TuneECU als Vergleich oder Tools wie SavvyCAN) entschlüsselt werden – machbar, aber ein eigenes Teilprojekt.
- Darüber liefen dann potenziell: Kühlwassertemperatur, Batteriespannung, Geschwindigkeit, evtl. Fehlercodes.

**Gangsensor – geringer bis mittlerer Aufwand:**
- Die 690er ab ca. 2016/2017 hat einen vollständigen Gangsensor (4- oder 6-poliger Stecker an der Schaltwalze), der pro Gang einen unterschiedlichen, festen Spannungswert ausgibt.
- Das ließe sich mit einem simplen ADC-Pin am ESP32 auslesen – kein Optokoppler nötig, da es ein sauberes Analogsignal ist (ggf. Spannungsteiler zur Pegelanpassung).
- Kalibrierung nötig: Spannungswerte den Gängen 1–6 zuordnen (einmalig, am Ständer).

**Öltemperatur – zusätzlicher Sensor nötig:**
- Die 690 LC4 hat werksseitig keinen Öltemperatursensor (nur einen Kühlwassertemperaturfühler). Ein Ölthermometer wird von Motorradfahrern seit Jahren als Nachrüstlösung diskutiert.
- Übliche Lösung: Einschraubfühler (z. B. NTC, M10x1-Gewinde) anstelle des Ölmessstabs bzw. am Ölkreislauf/Ölfilterdeckel einschrauben. Ein NTC lässt sich direkt per Spannungsteiler an einem ADC-Pin des ESP32 auswerten – elektrisch simpel, mechanisch der aufwändigere Teil (passende Bohrung/Adapter finden).

**Weitere realistische Ergänzungen mit eigenen Sensoren:**
- Kühlwassertemperatur: Originalfühler am Kühler – Signal ist analog, evtl. parallel abgreifbar (Innenwiderstand des ESP32-ADC beachten, ggf. Impedanzwandler).
- Batteriespannung: trivial per Spannungsteiler direkt vom Bordnetz.
- Ganganzeige-Fertiglösung als Referenz: Es gibt bereits kommerzielle Zusatzmodule (z. B. "CUMPAN"), die zeigen, dass das Gangsignal grundsätzlich sauber auswertbar ist – gute Bestätigung, dass dein Ansatz funktioniert.

Kurzfazit: Gangsensor und zusätzliche Temperaturfühler sind mit überschaubarem Aufwand in dein bestehendes Konzept (ESP32-S3, LVGL, mehrere ADC-Kanäle) integrierbar. Der CAN-Bus wäre der interessanteste, aber auch aufwändigste Zusatzbaustein, da die Signalzuordnung nicht dokumentiert ist.

Soll ich das als Ergänzung zur Sprint-1-Analyse dokumentieren (inkl. konkreter BOM-Vorschläge für Gangsensor-Anbindung und Öltemperaturfühler)?

---

## Nutzer:
Auf alle Fälle dokumentieren. Was Cumpan kann, kann ich alle Male - und das ganze dann als Open Source veröffentlichen.
Das wird ein spannendes Projekt!

## Claude:
[Erstellung des Word-Dokuments "Sprint2_Erweiterung_Machbarkeitsanalyse.docx" – vollständiger Inhalt siehe separate Datei im ZIP-Archiv]

Kurzzusammenfassung der Antwort:
Das ist Sprint 2 eurer Machbarkeitsanalyse, dokumentiert als Word-Datei. Inhalt im Überblick:
- Hardware-Entscheidung (CrowPanel 2,1", ESP32-S3R8) und wie sie die beiden Blocker aus Sprint 1 löst
- S3-vs-P4-Einordnung als Kurzbegründung der Wahl
- Ganganzeige – werksseitiger Gangsensor, ADC-Auswertung, Kalibrierung nötig
- Öltemperatur – kein Werkssensor vorhanden, Nachrüst-NTC nötig (mechanisch der aufwändigere Teil)
- CAN-Bus als spannender, aber eigenständiger Ausbauschritt für später (keine offizielle KTM-Dokumentation, Reverse-Engineering nötig)
- Open-Source-Vorbereitung: Lizenzwahl, Repo-Struktur, und ein Sicherheitshinweis-Absatz für Nachnutzer (Eingriffe in Fahrzeugelektrik, Gewährleistung, Zulassungsrelevanz)
- Nächste Schritte für Sprint 3

---

## Nutzer:
Bitte stelle den gesamten Chat inkl. dem Inhalt des Word-Dokuments in einer ZIP-Datei zum Download bereit. Irgend etwas mit dem Sync zwischen dem Claude Client auf dem iPhone (hier) und meinem Windows PC ist off, also nicht in-Sync (hier fehlen einige Chats und ich befürchte, dass dieser Chat hier verloren gehen könnte…)

## Claude:
[siehe ZIP-Archiv mit diesem Protokoll und dem Word-Dokument]

---

**Hinweis:** Dieses Textprotokoll ist eine manuelle Rekonstruktion des sichtbaren Gesprächsverlaufs aus diesem Chatfenster zum Zeitpunkt der Erstellung (07.08.2026), keine automatische Exportfunktion von Anthropic. Es enthält keine Bilder/Grafiken aus dem Projekt (Zifferblätter, Zeigergrafiken) und keine Tool-Zwischenschritte (z. B. Websuchen), nur die für den Nutzer sichtbaren Frage-/Antwort-Inhalte.
