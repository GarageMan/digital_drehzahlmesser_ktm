> **Hinweis:** Diese Datei ist eine Text-Extraktion aus dem Original-Dokument "TTC248XRS01.pdf" im Projekt (offizielles Hersteller-Datenblatt, Tailor Pixels Technology Co., Ltd., Rev. 1.0, 21.05.2024). Das Original-PDF konnte über die verfügbaren Werkzeuge nicht als Binärdatei abgerufen werden (nur der extrahierte Textinhalt war zugänglich). Insbesondere die mechanische Zeichnung (Kapitel 8) enthält im Original technische Vektorgrafiken, die sich nicht sinnvoll als Text darstellen lassen — hier daher nur die Kerndaten. Das Original mit vollständiger Formatierung und den Diagrammen kann direkt aus dem Projekt in der Claude-App heruntergeladen werden.
>
> **Wichtiger Kontext:** Dieses Datenblatt ist die primäre, verlässliche technische Quelle für das Display TTC248XRS-01 und war die Grundlage für die kritischen Befunde aus Sprint 1 (u. a. MIPI-DSI-Interface statt des ursprünglich angenommenen SPI/RGB, kein integrierter Touchscreen ("Without CTP"), 19,2V Backlight-Spannung statt einfacher 5V-Versorgung).

---

# TTC248XRS-01 — 2.48" TFT Display Module (480×480) — Datenblatt (Auszug)

Hersteller: Tailor Pixels Technology Co., Ltd. | www.tailorpixels.com | Rev. 1.0, 21. Mai 2024

## 1. General Specifications

| Kategorie | Feature | Spezifikation |
| --- | --- | --- |
| Display Spec. | LCD type | 2.48 inch |
| | Resolution (H×V) | 480(RGB)×480 |
| | Technology Type | a-Si TFT |
| | Pixel Configuration | R.G.B. Vertical Stripe |
| | Display Mode | Normally Black |
| | Surface Treatment | Glare |
| | Viewing Direction | ALL |
| Mechanical | Outline Dimensions (W×H×T, mm) | 65.87 × 69.04 × 1.85 |
| | Active Area (mm) | 62.93 × 62.93 |
| | **Mit/Ohne Touchscreen** | **Without CTP** (kein integrierter Touch!) |
| | Match Connector | 0.5 pitch, 30 pin |
| | Backlight Type | White LED |
| Electrical | **Interface** | **MIPI** (DSI, nicht SPI/RGB!) |
| | Number of colors | 16.7M |
| | Driver IC | ST7701S |

Hinweis im Original: "Viewing direction for best image quality is different from TFT definition. There is a 180 degree shift."

## 2. Pin Assignment (30-Pin, 0.5mm Pitch FPC)

| Nr. | Pin Name | Beschreibung |
| --- | --- | --- |
| 1 | LEDA | LED Anode |
| 2 | LEDK1 | LED Kathode |
| 3 | LEDK2 | LED Kathode |
| 4 | VCI | Power Supply 3.3V |
| 5 | IOVCC | Power Supply 3.3V |
| 6 | RESET | LCM Reset-Signal |
| 7 | TE | Tearing Effect Output |
| 8 | PWM | PWM-Frequenzausgang für LCD-Treiber-Steuerung |
| 9 | GND | Ground |
| 10 | D0P | DSI-D0+ Datensignal |
| 11 | D0N | DSI-D0- Datensignal |
| 12 | GND | Ground |
| 13 | D1P | DSI-D1+ Datensignal |
| 14 | D1N | DSI-D1- Datensignal |
| 15 | GND | Ground |
| 16 | CLKP | DSI-Clock+ |
| 17 | CLKN | DSI-Clock- |
| 18 | GND | Ground |
| 19–20 | NC | Nicht belegt |
| 21 | GND | Ground |
| 22–23 | NC | Nicht belegt |
| 24 | GND | Ground |
| 25 | TP_INT | Touch Interrupt |
| 26 | TP_SDA | Touch I²C Data |
| 27 | TP_SCL | Touch I²C Clock |
| 28 | TP_RESET | Touch Reset |
| 29 | TP_VCI | Touch Power Supply |
| 30 | TP_IOVCC | Touch Power Supply |

**Wichtig:** Obwohl das Panel selbst laut Feature-Tabelle "Without CTP" (kein Touch) ist, sind Touch-bezogene Pins (TP_*) im Pinout vorgesehen — je nach Bestückungsvariante. Vor Kaufentscheidung beim Hersteller/Händler klären, ob die konkret erworbene Variante tatsächlich Touch unterstützt.

## 3. Absolute Maximum Ratings (GND=0V, Ta=25°C)

| Item | Symbol | Value | Unit |
| --- | --- | --- | --- |
| Power supply voltage for logic | VDD | 0.3–3.3 | V |
| Input voltage | Vin | VDD+0.3 | V |
| Operating temperature | Topr | -20 bis 70 | °C |
| Storage temperature | Tstg | -30 bis 80 | °C |

## 4. Electrical Characteristics

### 4.1 Driving TFT LCD Panel (GND=0V, Ta=25°C)

| Item | Symbol | Min | Typ | Max | Unit | Bedingung |
| --- | --- | --- | --- | --- | --- | --- |
| Operating voltage | VDD | 2.5 | 2.8 | 3.3 | V | – |
| Supply current | IDD | – | 50 | – | mA | VDD=2.8V, Ta=25°C |
| Input High Voltage | VIH | 0.8·VDD | – | VDD | V | – |
| Input Low Voltage | VIL | 0 | – | 0.2·VDD | V | – |
| Input leakage current | IIL | -1.0 | – | 1.0 | µA | VIN=VDD oder VSS |

Hinweis: Spannungen über den o. g. Werten können das Modul beschädigen.

### 4.2 Driving Backlight (Ta=25°C)

| Item | Symbol | Min | Typ | Max | Unit | Bemerkung |
| --- | --- | --- | --- | --- | --- | --- |
| Forward Current | IF | – | 20 | – | mA | – |
| **Forward Voltage** | **VF** | – | **19.2** | – | **V** | – |
| Connection mode | P | – | 6S | – | – | LED-Anzahl / 6 Stück |

**Kritisch:** Die Backlight-Versorgungsspannung beträgt **19,2V (typ.)**, nicht die üblichen 3,3V/5V der Logikversorgung — dies erfordert einen separaten Spannungswandler/Boost-Konverter für die Hintergrundbeleuchtung und war einer der zentralen Befunde in Sprint 1 gegen die Eignung dieses konkreten Displays für eine einfache ESP32-Direktanbindung.

## 5. Interface Timing

Enthält im Original Zeitdiagramme für System Bus Read/Write Characteristics sowie Power ON/OFF Timing (T1 < 1ms für Power-ON-Sequenz, T2 > 200ms zwischen den Stufen Display ON/Backlight ON/Normal Operation). Für exakte Timing-Werte siehe Original-PDF.

## 6. Optical Characteristics (Ta=25°C)

| Item | Symbol | Min | Typ | Max | Unit |
| --- | --- | --- | --- | --- | --- |
| View Angles (T/B/L/R) | θ | 80 | 85 | – | Grad |
| Contrast Ratio | CR | 1000 | 1200 | – | – |
| Response Time (Ton/Toff) | T | – | 35 | 40 | ms |
| Uniformity | U | 70 | 80 | – | % |
| NTSC | – | – | 60–65 | – | % |
| Luminance | L | – | 500 | – | cd/m² |

Testbedingungen: VF=19,2V, IF=20mA, Umgebungstemperatur 25°C.

## 7. Environmental / Reliability Test

| Item | Bedingung | Dauer (h) |
| --- | --- | --- |
| High temp. Storage | 80°C | 96 |
| High temp. Operating | 70°C | 96 |
| Low temp. Storage | -30°C | 96 |
| Low temp. Operating | -20°C | 96 |
| Humidity | 40°C / 90% RH | 96 |
| Thermal Shock (Non-operation) | -20°C ↔ 70°C, 10 Zyklen | – |

Bewertungskriterium jeweils: keine funktionalen oder optischen Auffälligkeiten.

## 8. Mechanical Drawing (Kerndaten, Diagramm nicht textuell darstellbar)

- LCM Outline Dimensions: 65,87 × 69,04 mm
- Active Area: 62,93 × 62,93 mm
- Gesamtdicke: 1,85 mm (± 0,2 mm)
- LCD Driver IC: ST7701S, Treiberspannung 3,3V
- Interface-Auswahl laut Zeichnungsnotiz: "3SPI + RGB (3-wire SPI)" — diese Notiz auf der mechanischen Zeichnung steht im Widerspruch zur "Interface: MIPI"-Angabe in Kapitel 1 der Feature-Tabelle; dieser interne Widerspruch im Datenblatt selbst wurde in Sprint 1 explizit vermerkt und als Grund für zusätzliche Vorsicht/Nachfrage beim Verkäufer benannt.
- Betriebstemperatur: -20°C bis +70°C, Lagertemperatur: -30°C bis +80°C
- LED-Schaltung: IF=20mA, VF=19,2V (typ.)
- RoHS/HSF-konform

## 9. Precautions For Use of LCD Modules (Zusammenfassung)

- Display-Panel besteht aus Glas — keine mechanischen Stöße (z. B. Fallenlassen)
- Bei Beschädigung: Flüssigkristallsubstanz nicht in Mund/Augen, bei Hautkontakt mit Seife abwaschen
- Keine übermäßige Kraft auf Displayoberfläche (Farbtonveränderung möglich)
- Polarizer ist weich und leicht zerkratzbar — vorsichtig handhaben
- Reinigung: trockenes weiches Tuch, bei Bedarf mit Isopropylalkohol oder Ethylalkohol angefeuchtet; kein Wasser, Keton oder aromatische Lösungsmittel
- Modul nicht zerlegen
- Bei ausgeschalteter Logikversorgung keine Eingangssignale anlegen
- ESD-Schutz: geerdeter Arbeitsplatz, geerdete Lötwerkzeuge, keine Montage unter trockenen Bedingungen
- Lagerung: kein direktes Sonnenlicht, kein Fluoreszenzlicht, innerhalb der angegebenen Lager-Temperaturbereiche, keine säure-/laugen-/schadgashaltige Umgebung
- Transport: kein Herunterfallen, keine starken Erschütterungen, kein übermäßiger Druck, Feuchtigkeit oder Sonneneinstrahlung
