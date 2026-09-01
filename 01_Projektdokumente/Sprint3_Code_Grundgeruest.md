# Sprint 3 – Code-Grundgerüst & Feature-Roadmap

Projekt: Digitaler Drehzahlmesser · Stand: 07.08.2026 (Nachträge 11.08.2026)
Bezug: Sprint1_Machbarkeitsanalyse, Sprint2_Erweiterung_Machbarkeitsanalyse, Sprint2_Pruefung_externe_Analyse
Code-Archiv wurde an den Nutzer gesendet (ktm-drehzahlmesser-sprint3.zip) und ist buildbar/getestet (siehe Screenshots im Archiv unter docs/screenshots/).

## 1. Was gebaut wurde

Ein plattformneutrales C-Codegerüst (LVGL v8), das komplett am PC im SDL2-Simulator läuft, ohne dass die ESP32-Hardware vorliegt:

- Datenmodell (`vehicle_data.h`) mit allen Roadmap-Kanälen (RPM, Gang, Öl-/Wasser-/Lufttemperatur, Bordnetzspannung, Backup-Akku), nicht nur Release-1-Umfang.
- UI-Code (`ui.c`) identisch für PC-Simulator und späteren ESP32-Build – Drehzahlmesser (0–9500 U/min, kalibriert auf die vorhandenen PNG-Zifferblätter, Shift-Light-Blinken ab 8900 U/min) und Analoguhr, Umschaltung per Klick/Touch.
- Simulierter Datenprovider (`sim_provider.c`): erzeugt RPM-Sweep, Gangwechsel, realistisch ansteigende Temperaturen, Bordnetzspannung inkl. simulierbarem Anlasser-Spannungseinbruch, Backup-Akku-Lade-/Entladelogik – alle 500ms auf der Konsole geloggt.
- Eigener PNG→LVGL-Konverter (`tools/png_to_lvgl.py`), keine Abhängigkeit mehr von einem externen Online-Tool.
- Erfolgreich gebaut und headless (Xvfb) gerendert; Nadel-Geometrie visuell gegen die Zifferblatt-Beschriftung verifiziert (0 und 9 exakt getroffen).
- Kein ESP32-Code mit erfundener Pinbelegung – `main.cpp.stub` verweist bewusst auf das offizielle Elecrow-CrowPanel-Beispielprojekt statt Pins zu raten (Lehre aus Sprint 1).
- `docs/CAN_BUS_NOTES.md`: Zusammenfassung der bereits von github.com/blalor/ktm-can dekodierten CAN-IDs als Grundlage für eine spätere Ausbaustufe.

## 2. Backup-Akku für den ESP32 (Bordnetz-Überbrückung)

Empfehlung: ein fertiges "UPS"-Modul mit Lade-, Schutz- und Power-Path-Funktion zwischen Bordnetz-Wandler (5V) und CrowPanel-USB-Eingang.

- **IP5306-basiertes 5V/2A-USB-Lade-/UPS-Modul** (häufig als generisches Hobbyisten-Board erhältlich): Laden + Überladeschutz (4,2V) + Tiefentladeschutz (2,9V) + Überstromschutz + echtes Power-Path (Last bleibt am Eingang, Akku übernimmt nahtlos bei Ausfall). Einfachste, günstigste Option.
- **TI BQ24075/BQ24074** als "sauberere" Alternative, falls ein eigenes Platinen-Design statt eines fertigen Moduls gewünscht ist: dokumentierte Power-Path-Charger-IC, ebenfalls mit OVP/Strombegrenzung.
- Wichtiger Hinweis: Beide sind Consumer-Bauteile, nicht automotive-qualifiziert. Sie schützen den Akku, nicht den Buck-Converter vor 12V-Störungen – daher weiterhin sauberer, separater Step-Down-Wandler mit eigener Verpolungs-/Spannungsspitzenschutzbeschaltung davor (siehe Sprint 1).

## 3. Feature-Tabelle

| Datenpunkt | Release | Quelle | Aufwand | Hinweis |
|---|---|---|---|---|
| Drehzahl | 1 | Analog (Kurbelwellensensor + Optokoppler) | gering | Sprint 1/2; Sensortyp/Impulszahl weiter zu verifizieren |
| Analoguhr | 1 | DS3231 RTC | gering | Sprint 1 |
| Backup-Akku ESP32 | 1 (Hardware) | IP5306- oder BQ24075-Modul | gering–mittel | siehe Abschnitt 2 |
| Ganganzeige | 2 | Analog (Gangsensor, ADC) **oder** CAN 0x129/0x540 | gering (Sensor, ab ~2019) / mittel (CAN) | ktm-can liefert Gang bereits dekodiert |
| Kühlwassertemperatur | 2 | Analog (Parallelabgriff Serienfühler) **oder** CAN 0x540 | gering–mittel | ktm-can liefert Kühlwassertemp. bereits dekodiert |
| Motoröltemperatur | 2 | Nachrüst-NTC (M10×1, Ölfilterdeckel) | mittel (mechanisch) | kein Werkssensor; 690-spezifische Adapterpassung offen |
| Bordnetzspannung | 2 | Spannungsteiler an 12V | trivial | – |
| Lufttemperatur (Ansaug) | 2/3 | eigener NTC im Luftstrom | gering | kein bestätigter Werks-IAT-Abgriff auffindbar |
| Geschwindigkeit | 3 | nur CAN 0x12B | mittel (CAN nötig) | kein Analogabgriff bekannt |
| Schräglage (Lean Angle) | 3, optional | nur CAN 0x12B | mittel (CAN) | Nice-to-have, ktm-can liefert es bereits |
| Bremsdruck vorne | optional | CAN 0x290 | mittel (CAN) | eher fürs Logging als fürs Display |

Quelle CAN-IDs: [github.com/blalor/ktm-can](https://github.com/blalor/ktm-can) (siehe docs/CAN_BUS_NOTES.md im Code-Archiv).

## 4. Wie es weitergeht

1. CrowPanel-Board bestellen (Sprint 2), ESP32-Arduino-Core v2.0.14 fixieren.
2. Sobald Board da: Elecrow-Beispielprojekt als Basis für `platform/esp32/`, `main.cpp.stub` ausfüllen, `core/ui.c` unverändert einbinden.
3. KTM-Werkstatthandbuch: Kurbelwellensensor (Typ/Impulszahl) und Gangsensor-Steckerbelegung am eigenen Fahrzeug (Bj. 2021) verifizieren.
4. Öltemperaturfühler-Adapter (M10×1) bei Berotec explizit für 690er-Baureihe anfragen.
5. Release 1 auf Basis der realen Sensoren fertigstellen (RPM + Uhr + Backup-Akku), danach Release 2 (Gang, Temperaturen) angehen.
6. CAN-Bus (ktm-can-Repo als Startpunkt) als spätere, optionale Ausbaustufe.
7. Lizenzentscheidung (MIT/GPLv3) treffen, Repository-Grundgerüst für Open-Source-Veröffentlichung anlegen.

## 5. Anschluss-Frage: CrowPanel-Steckverbinder & CAN-Ankopplung (Nachtrag 07.08.2026)

Klarstellung, da die Frage aufkam, "wie das Display an einen ESP32-S3 angeschlossen wird": Beim CrowPanel ist der ESP32-S3R8 bereits fest auf der Display-Platine verlötet – es muss nichts an einen separaten ESP32-S3 "angeschlossen" werden. Die winzigen Stecker auf der Platine sind **Erweiterungsports für zusätzliche Peripherie** (Sensoren etc.), nicht die Display-Anbindung.

**Steckverbinder laut Elecrow-Doku:** 2x UART (UART0, UART1) + 1x I2C, jeweils **ZX-MX 1.25mm, 4-polig** (Bauform aus der JST-MX/PicoBlade-1.25-Familie) – tatsächlich sehr klein. Dazu ein 12-poliger FPC-Stecker, der ausschließlich Stromversorgung/Programmierung bedient. I2C ist zudem intern bereits für Touch-Controller und IO-Expander genutzt (Adresse 0x21) – dort ist kein "freier" Bus.

**Krimpen vermeiden:** Vorkonfektionierte "JST MX 1.25mm 4-Pin"-Kabel (mit Buchse/Stecker auf der einen, offenen Drahtenden auf der anderen Seite) sind als fertige Sets bei Amazon/eBay verfügbar (Stichwort "JST MX 1.25mm pre-crimped cable"). Das offene Ende lässt sich normal anlöten – kein eigenes Krimpen nötig.

**CAN-Ankopplung – vermutlich einfacher als gedacht:** Der ESP32-S3 hat einen eingebauten CAN-Controller (TWAI, offiziell von Espressif dokumentiert), dessen Pins per Software auf praktisch beliebige freie GPIOs gelegt werden können. Das spricht dafür, **keinen SPI-basierten CAN-Adapter (z.B. MCP2515)** zu verwenden – dafür sind ohnehin keine SPI-Leitungen extern zugänglich –, sondern einen kleinen CAN-**Transceiver**-Breakout (z.B. SN65HVD230, üblicherweise mit normalen 2,54mm-Pinheadern) über die UART1-Buchse anzuschließen und die zugehörigen GPIOs in der Firmware als TWAI RX/TX zu konfigurieren. UART0 eher meiden (vermutlich an die Programmier-/Konsolenschnittstelle gekoppelt). Offen: die exakten GPIO-Nummern von UART1 sind aus der öffentlichen Doku nicht ablesbar – vor Bestellung/Verdrahtung bei Elecrow-Support erfragen oder im GitHub-Repo (`factory_sourcecode`) nachsehen, nicht raten (siehe Sprint 1 zur Pin-Erfindungs-Problematik).

**Teensy 4.1 / Adafruit Feather RP2040 CAN als Alternative?** Beide sind technisch potente, gut dokumentierte CAN-Plattformen (Teensy 4.1: 3x natives CAN via FlexCAN). Ein Wechsel würde aber bedeuten, die bereits geloeste CrowPanel-Integration (Display+Touch+ESP32-S3 aus einer Hand, Sprint 2) aufzugeben und wieder ein rohes Display-Panel anzusteuern – das öffnet exakt die MIPI/RGB- und Touch-Fragen aus Sprint 1 erneut. Empfehlung: beim CrowPanel bleiben, CAN über dessen eingebautes TWAI + kleinen Transceiver lösen.

## 6. Steckverbinder-Details vertieft & SN65HVD230-Verdrahtung (Nachtrag 11.08.2026)

Anlass: Nutzer hat das offizielle Rückseiten-Foto/-Diagramm des CrowPanel 2.1" beigefügt (Quelle: elecrow.com Produktseite/Wiki, GitHub Elecrow-RD-Repo) und vier konkrete Fragen gestellt. Recherchequellen zusätzlich zu Abschnitt 5: Elecrow-Wiki-Seite, makerguides.com "Getting started with CrowPanel 2.1inch-HMI ESP32 Rotary Display", Elecrow-Support-Forum (Thread #27965, ein anderer Nutzer mit derselben GPIO-Frage).

**a) PC-Verbindung: USB IN(5V) genügt, FPC ist NICHT für Endnutzer gedacht.**
Bestätigt über zwei unabhängige Quellen (Elecrow-Wiki, makerguides.com): Das mitgelieferte USB-Kabel wird direkt in die mit "USB-5V-IN" beschriftete Buchse gesteckt; diese bedient sowohl Stromversorgung als auch Programmierung ("supports both power supply and program burning"). Der genaue Steckertyp (USB-C oder Micro-USB) ließ sich aus den öffentlich zugänglichen Quellen nicht zweifelsfrei klären – es ist aber sicher ein normaler USB-Steckverbinder mit passendem Kabel, kein Lötanschluss. Flash-Vorgang laut Anleitung: Boot-Taste gedrückt halten, dann per USB mit dem PC verbinden.
Der 12-polige FPC-Stecker ist dagegen laut Elecrow-Wiki explizit als "Power supply burning port" für die **Werksfertigung** (Bestückungstest/Erstprogrammierung am Band) beschriftet – kein für Endnutzer vorgesehener Anschluss und ohnehin nur mit einem speziellen FPC-Zugentlastungsstecker (kein Löten möglich) nutzbar. Für den Eigenbau ist die FPC-Buchse zu ignorieren.

**b) I2C-Buchse: für zusätzliche I2C-Sensoren, teilt sich den Bus mit Touch/IO-Expander.**
Bestätigt (Elecrow-Wiki): interner I2C-Bus liegt auf GPIO38 (SDA) / GPIO39 (SCL) und bedient bereits den kapazitiven Touch-Controller (CST8xx, Adresse 0x15) sowie den PCF8574-IO-Expander (Adresse 0x21, für Encoder-Taste/Touch-Reset). Der externe I2C-Steckverbinder ist – nach allem, was die Quellen hergeben – derselbe Bus, nur herausgeführt. Das heißt: Man kann zusätzliche I2C-Peripherie (z.B. einen zweiten Temperatursensor, ein weiteres Breakout) dort anschließen, muss aber auf freie, nicht mit 0x15/0x21 kollidierende I2C-Adressen achten. Ein Elecrow-Support-Forumsnutzer mit identischer Fragestellung (GPS-Modul + Schrittmotortreiber anschließen) bekam die Antwort "auf dem Panel ist nur ein I2C-Port frei" und wurde für Details per E-Mail an den Support verwiesen – das bestätigt indirekt, dass I2C der einzige öffentlich unklar dokumentierte, aber grundsätzlich nutzbare freie Bus ist.

**c) SN65HVD230 (Waveshare) an UART1 – Grundschaltung, aber zwei offene Punkte vor dem Löten.**
Der SN65HVD230 ist ein reiner Transceiver (Bindeglied zwischen 3,3V-Logikpegeln und dem differenziellen CAN-Bus), kein eigener CAN-Controller – das übernimmt das eingebaute TWAI-Peripheriegerät im ESP32-S3. Grundsätzliche Verdrahtung (4 Leitungen):

| SN65HVD230-Pin | Ziel |
|---|---|
| VCC | 3,3V (siehe Warnung unten) |
| GND | GND |
| TXD | ESP32-S3 TWAI-TX-GPIO |
| RXD | ESP32-S3 TWAI-RX-GPIO |
| CANH / CANL | zum Fahrzeug-CAN-Bus (Twisted Pair, z.B. am 6-poligen Sumitomo-Diagnosestecker – exakte Pinbelegung dort separat über das ktm-can-Repo klären, nicht Teil dieser Antwort) |

Kein zusätzlicher 120-Ohm-Abschlusswiderstand nötig, solange nur ein Abgriff (Tap) am bereits terminierten Fahrzeugbus erfolgt, kein Bus-Enderpunkt.

Zwei Punkte, die vor dem Löten zu klären sind (bewusst nicht geraten, siehe Sprint-1-Lehre):
1. **Exakte GPIO-Nummern von UART1 (TX/RX) sind weiterhin nicht öffentlich dokumentiert** (auch der Elecrow-Support-Thread verweist nur auf eine private E-Mail-Antwort). Empfehlung: dieselbe Frage wie der andere Forumsnutzer beim Elecrow-Support stellen (Vorschlag für eine Anfrage kann bei Bedarf formuliert werden) – oder nach Erhalt des Boards die Pins mit einem Multimeter/Kontinuitätstest gegen das ESP32-S3-Datenblatt zurückverfolgen.
2. **Spannungspegel der UART1-Buchse (VCC-Pin) ist nicht bestätigt 3,3V.** Der SN65HVD230 verträgt maximal ca. 3,6V an VCC – eine 5V-Versorgung würde ihn zerstören. Da die interne Logik des ESP32-S3 mit 3,3V arbeitet, ist 3,3V auf der UART1-Buchse wahrscheinlich, aber nicht bestätigt (denkbar wäre auch eine bewusste 5V-Versorgungsader für externe 5V-Module, getrennt von den 3,3V-Signalpegeln). Vor dem Anschluss des Transceivers: Board an USB anschließen, Spannung zwischen VCC- und GND-Pin der UART1-Buchse mit einem Multimeter messen. Falls 5V anliegen, VCC des SN65HVD230 stattdessen von einem 3,3V-Pin des Boards (z.B. vom I2C-Stecker, falls dort 3,3V bestätigt ist, oder einem etwaigen 3V3-Pad) abgreifen, nicht von der UART1-Buchse.

**d) Stromversorgung Labor vs. Motorrad – ein durchgängiges 5V-Konzept.**
Im Lötlabor: PC-USB oder Labornetzteil direkt in die USB-5V-IN-Buchse – unkritisch, wie vom Nutzer schon angenommen. Am Motorrad läuft dieselbe Buchse (bzw. bei Bedarf ein aufgetrenntes/direkt verlötetes USB-Kabel) letztlich an derselben 5V-Schiene, nur dass die 5V dort nicht vom PC, sondern von einem eigenen 12V→5V-Abwärtswandler (Buck-Converter) am Bordnetz kommen, wie bereits in Abschnitt 2 und in Sprint 1 beschrieben. Das dort empfohlene IP5306- oder BQ24075-basierte Akku-/UPS-Modul wird zwischen diesen Buck-Converter und die USB-5V-IN-Buchse des CrowPanels geschaltet: Solange das Motorrad läuft, speist der Wandler direkt durch (Power-Path) und lädt nebenbei den Puffer-Akku; bricht die Bordspannung kurz ein (Anlasser, Wackelkontakt), übernimmt der Akku nahtlos, ohne dass der ESP32/das Display neu startet. Damit ist die Frage aus Abschnitt 2 (Backup-Akku) und die aktuelle Frage (Motorrad-Stromversorgung) dieselbe Schaltung, nur an zwei verschiedenen Stellen der Roadmap gestellt.

**Offen für die nächste Runde:** exakte UART1-GPIOs (Support-Anfrage vorformulieren?), VCC-Pegel der UART1-Buchse messtechnisch bestätigen, sowie – separat – die Pinbelegung des KTM-Diagnosesteckers für CANH/CANL aus dem ktm-can-Repo herausziehen.

## 7. Elecrow-Support-Anfrage, Verbindungsskizze & erster Bring-up-Test (Nachtrag 11.08.2026)

**Elecrow-Support-Anfrage raus.** Der Nutzer hat eine E-Mail an den Elecrow-Support geschickt mit der Bitte um: exakte UART0/UART1-GPIO-Nummern, Bestätigung ob UART0 intern mit der USB-Programmier-/Konsolenschnittstelle geteilt wird, den Spannungspegel (3,3V vs. 5V) auf den VCC-Pins von UART0/UART1/I2C, sowie ob diese GPIOs softwareseitig frei auf andere Peripherie (z.B. TWAI/CAN) umgelegt werden können. Antwort steht noch aus – bis dahin gelten die Platzhalter/offenen Punkte aus Abschnitt 6 weiter.

**Zusätzliche Recherche-Funde (Quellen: Elecrow-Support-Forum-Threads #27965 und #27974, ein per raw.githubusercontent.com abrufbares Beispiel-Sketch-File aus dem offiziellen Elecrow-RD-Repo, makerguides.com):**

- Das offizielle Beispielprojekt für dieses Board heißt **`RotaryScreen_2_1`** (Ordner `example/` im Repo, Branch **`master`**, nicht `main` – das erklärt frühere erfolglose Abruf-Versuche), erstellt mit LVGL + SquareLine Studio, Arduino IDE 1.8.19.
- **Bestätigte, aus dem echten Beispiel-Code stammende Pins/Adressen** (nicht erfunden, direkt aus dem Sketch): Backlight PWM = GPIO6 (Kanal 0, 5kHz, 8-bit), I2C SDA/SCL = GPIO38/39, Encoder A/B = GPIO42/4, Encoder-Taster = PCF8574-Bit P5 (nicht direkt ein GPIO), Touch-Controller CST8xx an I2C-Adresse 0x15, PCF8574-IO-Expander an 0x21 mit Bit-Belegung P0=Touch-Reset, P2=Touch-Interrupt, P3=LCD-Power, P4=LCD-Reset, P5=Encoder-Taster. Zusätzlich: **GPIO43 ist im Original-Beispiel als "Breathing-LED" belegt** – also beim eigenen Board-Layout nicht als "frei" annehmen.
- **LVGL-Version 8.3.11** wird laut Elecrow-Doku für dieses Board vorausgesetzt – passt zur `release/v8.3`, die auch der PC-Simulator in diesem Projekt nutzt (siehe Abschnitt 1).
- Ein Forumsnutzer hatte mit dem Standard-Partitionsschema einen Compile-Fehler ("Sketch too large, 189%") – Empfehlung dort: Partition Scheme auf ein großzügigeres Schema (z.B. "Huge APP") umstellen.
- UART0/UART1-GPIOs kamen im untersuchten Beispiel-Sketch nicht vor (das Beispiel nutzt WLAN/UDP statt UART-Erweiterung) – bestätigen daher **nicht**, sind weiterhin offen (siehe Support-Anfrage oben).

**Verbindungsskizze erstellt** (`docs/verbindungsskizze.html` im Code-Archiv, als Diagramm auch direkt an den Nutzer gesendet/persistiert): zeigt drei Phasen – (1) PC-Programmierung über USB-5V-IN, (2) SN65HVD230-Anbindung über UART1 zum KTM-Diagnosestecker, (3) spätere Bordnetz-Stromversorgung über Step-Down-Wandler + Akku-Puffer-Modul in dieselbe USB-5V-IN-Buchse. Offene Punkte (UART1-GPIOs, VCC-Pegel) sind darin explizit als "offen" markiert, nicht als Fakten dargestellt.

**Erster Bring-up-Test für das reale Board** (`src/platform/esp32/bringup_pc_telemetry/` im Code-Archiv): Da das CrowPanel-Board beim Nutzer eingetroffen ist, wurde ein einfacher, von RPM/CAN/Motorrad unabhängiger Test zusammengestellt, um Touchscreen und den Dreh-/Druckmechanismus des eingebauten Encoders zu prüfen (Anlass: Nutzer äußerte Zweifel an der Stabilität des Encoder-Sockels für den Motorradeinsatz). Der Sketch (`bringup_pc_telemetry.ino`) zeigt umschaltbar (Touch-Tap, Encoder-Drehen, Encoder-Taste) PC-Kennzahlen (CPU-/GPU-Temperatur, CPU-Last, RAM-Auslastung), die ein beiliegendes Python-Skript (`tools/pc_telemetry_sender/pc_telemetry_sender.py`) per USB-Seriell an das Board sendet. Bewusst **nicht** enthalten: die Display-Panel-/Touch-/PCF8574-Initialisierung – dafür verweist der Sketch (wie schon `main.cpp.stub`) auf das 1:1 aus `RotaryScreen_2_1.ino` zu übernehmende Original, um keine Pin-/Init-Daten zu erfinden. Das Python-Skript ist bewusst transparent über Grenzen: GPU-Temperatur nur für NVIDIA (via `pynvml`) implementiert, CPU-Temperatur über `psutil` funktioniert zuverlässig primär unter Linux.

**Einordnung des Encoder-Zweifels:** Die Architektur trennt UI-Umschaltung (Touch **und** Encoder, beide unabhängig implementiert) bereits jetzt sauber, sodass eine spätere Entscheidung "Encoder am Motorrad nicht verbauen, nur Touch nutzen" keine Restrukturierung erfordert – nur das Weglassen der Encoder-Abfrage in der finalen Firmware.

**Offen für die nächste Runde:** Antwort des Elecrow-Supports zu UART0/UART1; danach Abschnitt 6 präzisieren und `can_provider`-Stub/Bring-up-Sketch mit den bestätigten Pins ergänzen. Ergebnis des ersten Bring-up-Tests (funktioniert Touch/Encoder am realen Board wie erwartet?) durch den Nutzer zurückmelden lassen.

## 8. Erstinbetriebnahme-Anleitung & USB-CAN-Adapter als PC-seitiges Verifikationswerkzeug (Nachtrag 11.08.2026)

**Schritt-für-Schritt-Anleitung zur Erstprogrammierung erstellt** (nicht als Datei, sondern im Chat, da Nutzer explizit einen begleiteten Ablauf wollte): Arduino-IDE-Einrichtung (Board-Paket-URL, ESP32-Core v2.0.14, Board "ESP32S3 Dev Module", Partition Scheme "Huge APP", PSRAM "OPI PSRAM", USB CDC On Boot "Enabled"), danach zwingend zuerst das offizielle `RotaryScreen_2_1`-Beispiel unverändert testen (Checkpoint, um Board-/Treiberprobleme von eigenem Code zu trennen), erst danach den eigenen Bring-up-Sketch zusammenbauen. Nutzer hat reine Python-, keine Arduino-/ESP32-Erfahrung – Anleitung entsprechend ohne vorausgesetztes Embedded-Wissen formuliert.

**Frage: Kann der vorhandene "Jhoinrch RH02 USB-to-CAN"-Adapter den SN65HVD230 vorerst ersetzen?**
Antwort: Nicht 1:1 austauschbar, da unterschiedliche Rollen – aber sehr sinnvoll als eigenständiger, vorgezogener Test- und Verifikationsschritt.

- **RH02 = PC-seitiges CAN-Interface.** Recherche bestätigt: basiert auf der offenen CANable-Hardware/Firmware ("candleLight" als Standardfirmware, umflashbar auf "slcan"), keine unseriöse Billig-Ware. Steckt per USB im PC, liest/schreibt CAN-Daten dort – z.B. mit `python-can` (passt zum reinen Python-Skillset des Nutzers). Unter Linux nativ als SocketCAN-Gerät nutzbar (kein Treiber nötig); unter Windows entweder Hersteller-Tools oder Umflashen auf slcan-Firmware nötig.
- **SN65HVD230 = fest mit dem ESP32-S3/CrowPanel verdrahteter Transceiver.** Ergibt nur zusammen mit dem eingebauten TWAI-Controller des ESP32 Sinn – das ist die Lösung fürs fertige Geraet am Lenker, nicht für PC-Tests.
- **Empfehlung:** RH02 jetzt schon nutzen, um die von ktm-can dekodierten CAN-IDs (0x120, 0x129, 0x540 etc.) direkt am eigenen Fahrzeug über Python zu verifizieren – unabhängig vom ESP32/CrowPanel, ein eigenständiger Meilenstein vor dem eigentlichen Hardware-Löten.
- Praktische Hinweise mitgegeben: Bitrate nicht raten, sondern aus dem ktm-can-Repo übernehmen; auf dem RH02 (CANable-Familie) eine ggf. vorhandene 120-Ohm-Terminierungsbrücke/-Schalter prüfen und deaktivieren (nur Tap, kein Busende); CANH/CANL/GND-Verkabelung zum 6-poligen KTM-Diagnosestecker wird so oder so benötigt, unabhängig vom gewählten Adapter.
- **Bonus-Idee (noch nicht umgesetzt):** PC liest echte CAN-Daten via RH02 und `python-can`, ein Python-Skript (nach demselben Muster wie `pc_telemetry_sender.py`) leitet RPM/Gang/Temperatur per USB-Seriell ans CrowPanel weiter – so liessen sich echte Fahrzeugdaten auf dem Display zeigen, bevor der SN65HVD230 überhaupt verlötet ist. Auf Wunsch des Nutzers als nächster Schritt umsetzbar.

**Nutzer-Rückfrage: Ist ESP32-Core v2.0.14 nicht viel zu alt (aktuell wäre 3.3.11)?**
Berechtigter Einwand, mit Primärquelle gegengeprüft statt nur wiederholt. Direktes Zitat aus dem Getting-Started-Guide für exakt dieses Board (makerguides.com, "Getting started with CrowPanel 2.1inch-HMI ESP32 Rotary Display"):

> "For the CrowPanel 2.1inch Display that you will need to install a specific version (2.0.14) of the ESP32 core. [...] Note that you can install versions up to 2.0.17 but not 3.x. The libraries, which we will install in the later section will not work with the 3.x ESP32 core."

Einordnung: Das ist keine allgemeine Empfehlung, "alte Software" zu nutzen, sondern eine **für dieses spezifische Board dokumentierte Inkompatibilität** – die vom Beispielprojekt genutzten Display-/RGB-Panel-Bibliotheken sind (Stand der Quelle) nicht mit dem 3.x-Core (neueres ESP-IDF, geänderte Peripherie-APIs) kompatibel. Erlaubter Bereich laut Quelle: 2.0.14 bis 2.0.17. Empfehlung angepasst: **2.0.17** wählen (neueste noch bestätigt kompatible Version, mehr Bugfixes als 2.0.14) statt exakt 2.0.14; bei Problemen auf 2.0.14 zurückfallen, da das die vom Guide-Autor selbst getestete Version ist.

**Offen für die nächste Runde:** Feedback zum ersten Arduino-IDE-Durchlauf (lief das offizielle Beispiel mit Core 2.0.17? Kompilierfehler?), Entscheidung ob die RH02-basierte CAN-Verifikation als nächster Schritt umgesetzt werden soll, weiterhin Antwort von Elecrow zu UART0/UART1 ausstehend.

## 9. LVGL-Versionswechsel v8.3 → v9.1 & eigener KTM-Bring-up-Sketch (Nachtrag 01.09.2026)

**Anlass:** Fortsetzung nach dem erfolgreichen Bring-up-Meilenstein (Abschnitt 8, 22.08.) mit dem Ziel, den eigenen KTM-690-Bring-up-Sketch (RPM-Anzeige mit Nadel-Sweep, Uhr-Ansicht, Touch-Umschaltung) auf Basis des jetzt funktionierenden `RotaryScreen_2_1`-Grundgerüsts aufzusetzen (siehe Abschnitt 9 der vorigen Fassung dieses Dokuments).

**Fund vor dem Schreiben des Sketches:** Abgleich mit dem tatsächlich aktuellen offiziellen Elecrow-GitHub-Repo (nicht nur der Doku) zeigte, dass `RotaryScreen_2_1.ino` – derselbe Sketch, mit dem der Bring-up-Test am 22.08. erfolgreich lief – nachweislich **LVGL 9.1.0** nutzt (`example/Arduino/libraries/lvgl/library.json`, reine v9-API: `lv_display_create`, `lv_indev_create`, `lv_screen_active()` usw.), nicht die in Sprint 3 (Abschnitt 7) dokumentierte 8.3.11. Das Elecrow-Repo wurde offenbar zwischen der damaligen Recherche und heute aktualisiert. `core/ui.c` und der PC-Simulator waren bewusst gegen die v8-API geschrieben – das Kernprinzip "1:1 Code-Übernahme PC→ESP32" stimmte damit nicht mehr.

**Entscheidung (Nutzer):** Vollständige Migration auf LVGL v9, statt nur den neuen ESP32-Sketch separat in v9 zu schreiben – damit bleibt `core/ui.c` weiterhin die einzige Quelle der Wahrheit für beide Plattformen.

**Durchgeführt:**
- `core/ui.c`/`ui.h` auf v9-API portiert (`lv_image_*` statt `lv_img_*`, `lv_screen_active()`/`lv_screen_load_anim()` statt `lv_scr_act()`/`lv_scr_load_anim()`, `lv_obj_remove_flag()` statt `lv_obj_clear_flag()`, `LV_SCR_LOAD_ANIM_FADE_IN` statt `_FADE_ON`). Rotationswinkel-Konvention (0,1°-Schritte, 0..3600, im Uhrzeigersinn) ist identisch geblieben.
- `ui.h` zusätzlich mit `extern "C"`-Absicherung versehen: `ui.c` wird als reines C kompiliert, ein ESP32-Arduino-Sketch aber immer als C++ – ohne diese Absicherung hätte das einen Linker-Fehler durch C++-Name-Mangling gegeben (im PC-Simulator, reines C, nie aufgefallen).
- `tools/png_to_lvgl.py` neu geschrieben: v9-Bildheader (`lv_image_dsc_t`/`lv_image_header_t` mit `magic`/`stride`-Feldern statt der alten v8-Struktur), Pixel-Byte-Reihenfolge (BGRA) unverändert. Nebeneffekt: PIL/Pillow-Abhängigkeit entfernt, PNG-Decoding jetzt mit einem minimalen, selbst geschriebenen Decoder auf Basis von `zlib` (Python-Standardbibliothek) – byteweise gegen die vorherigen PIL-erzeugten Arrays verifiziert, identisch für alle 5 Assets.
- `lib/lv_conf.h` durch das echte v9.1.0-Template ersetzt (Inhalt aktiviert, `LV_USE_SDL` auf 1, sonst Defaults – u.a. `LV_COLOR_DEPTH=16`/RGB565 statt vorher 32, passend zum echten Display). `lib/lv_drv_conf.h` entfällt ersatzlos (v9 bringt seinen SDL-Treiber selbst mit, kein separates `lv_drivers`-Repo mehr).
- `tools/setup_deps.sh` klont jetzt Tag `v9.1.0` (statt Branch `release/v8.3`), ohne `lv_drivers`.
- `CMakeLists.txt` und `src/platform/sim/main.c` an die v9-SDL-API angepasst (`lv_sdl_window_create()`/`lv_sdl_mouse_create()` ersetzen das manuelle Draw-Buffer-/Flush-Setup von v8).
- **Wichtige Einschränkung:** Die Migration wurde anhand des tatsächlichen v9.1.0-Quellcodes (aus dem Elecrow-Repo mitgeliefert) Header für Header gegengeprüft, konnte aber in der Entwicklungsumgebung, in der sie entstand, nicht tatsächlich kompiliert werden (kein cmake/gcc/SDL2-devel installiert, keine Root-Rechte zur Nachinstallation). **Der erste `cmake .. && make`-Lauf beim Nutzer ist damit der erste echte Kompiliertest dieser Migration.**

**Eigener Bring-up-Sketch:** `src/platform/esp32/ktm_bringup/ktm_bringup.ino` – Display-/Touch-/PCF8574-Init 1:1 aus `RotaryScreen_2_1.ino` übernommen (nicht erfunden), SquareLine-Menü-Teil durch `core/ui.c` ersetzt, dazu ein einfacher `vehicle_data_provider_t` mit automatischem RPM-Sweep + laufender Uhr (keine echten Sensoren – reine Anzeige-/Touch-Verifikation auf echter Hardware). Da Arduino-Sketches keine beliebigen relativen Include-Pfade erlauben, wird `core/ui.c` über eine per `tools/prepare_esp32_library.sh` generierte Arduino-Bibliothek (`libraries/ktm_ui/`) eingebunden statt kopiert – Quelle der Wahrheit bleibt `src/core/`. Bewusst **nicht** enthalten: Encoder-Bedienung (Pins bestätigt, aber für "Touch-Umschaltung" laut Aufgabenstellung nicht nötig) und jede Sensor-/CAN-Anbindung. Details/Testanleitung siehe `src/platform/esp32/ktm_bringup/README.md`. Ebenfalls **nicht** aktualisiert: der ältere `bringup_pc_telemetry`-Test (verweist weiterhin auf Core 2.0.14/LVGL 8.3.11 – eigenständiger, bereits abgeschlossener Diagnoseschritt vor diesem Sketch, nicht Teil dieser Migration).

**Offen für die nächste Runde:** Ergebnis des ersten Kompilier-/Flash-Durchlaufs von PC-Simulator und `ktm_bringup.ino` beim Nutzer zurückmelden lassen. Falls erfolgreich: Entscheidung, ob als Nächstes Encoder-Bedienung ergänzt wird oder direkt echte Sensor-/CAN-Anbindung (Abschnitt 6-8) angegangen wird.
