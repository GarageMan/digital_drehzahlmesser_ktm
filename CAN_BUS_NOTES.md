# CAN-Bus als optionale Datenquelle (Sprint 3+, nicht Release 1)

Referenz: [github.com/blalor/ktm-can](https://github.com/blalor/ktm-can) - ein
unabhaengiges (nicht von KTM autorisiertes) Reverse-Engineering-Projekt am
6-poligen Sumitomo-Diagnosestecker einer realen KTM 690 Enduro R.

Diese Datei fasst nur zusammen, was das Repo aktuell an CAN-IDs dekodiert -
sie ist **kein** eigener Treiber und wird von diesem Projekt (noch) nicht
eingebunden. Bevor daraus Code wird: Board mit CAN-Transceiver (z.B.
MCP2515/TJA1050) am ESP32 vorhanden, Verifikation der Werte am eigenen
Fahrzeug.

## Bereits dekodierte CAN-IDs (Stand: Recherche 07.08.2026)

| CAN-ID | Enthaltene Signale |
|---|---|
| 0x120 | Drehzahl (RPM), Drosselklappenstellung, Killswitch, Gasmap/-status |
| 0x129 | Gangposition, Kupplungsschalter |
| 0x12A | Drosselklappe offen/geschlossen, angeforderte Gasmap |
| 0x12B | Vorder-/Hinterradgeschwindigkeit, Schraeglage (Lean Angle), Neigungswinkel |
| 0x290 | Bremsdruck vorne |
| 0x450 | Traktionskontroll-Taster |
| 0x540 | Drehzahl (langsamere Rate), Gang, Seitenstaenderstatus/-fehler, Kuehlwassertemperatur |

**Nicht** dekodiert bzw. im Repo nicht vorhanden: Batteriespannung,
Tankfuellstand, Fehlercodes. Einzelne Felder in 0x12A/0x450 sind laut Repo
unbestaetigt/teilweise.

## Warum das fuer dieses Projekt interessant ist

Falls spaeter ein CAN-Transceiver ergaenzt wird, koennten **RPM, Gang und
Kuehlwassertemperatur ueber einen einzigen Bus-Abgriff** kommen, statt ueber
drei separate Analog-/Sensor-Anbindungen (Kurbelwellensensor-Optokoppler,
Gangsensor-Spannungsteiler, Parallelabgriff am Kuehlwasserfuehler). Zusaetzlich
waeren Geschwindigkeit und Schraeglage nur ueber CAN ueberhaupt verfuegbar -
dafuer gibt es keinen einfachen Analogabgriff.

Das ist eine strategische Option fuer eine spaetere Ausbaustufe, kein
Ersatz fuer Release 1: Der Bus ist inoffiziell reverse-engineered, nicht von
KTM dokumentiert, und sollte vor produktivem Einsatz am eigenen Fahrzeug
gegengeprueft werden (siehe Sprint2_Pruefung_externe_Analyse).

## Geplante Provider-Erweiterung (noch nicht implementiert)

`vehicle_data.h` hat dafuer bereits `DATA_SRC_CAN` als moeglichen Wert von
`data_source_t` vorgesehen. Ein spaeterer `can_provider.c` wuerde denselben
`vehicle_data_provider_t`-Vertrag wie `sim_provider.c` implementieren und
liesse sich damit einbinden, ohne `ui.c` anzufassen.
