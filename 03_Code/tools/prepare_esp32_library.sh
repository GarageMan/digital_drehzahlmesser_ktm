#!/usr/bin/env bash
# Packt src/core/ (ui.c/ui.h/vehicle_data.h) + assets/generated/ als Arduino-
# Bibliothek "ktm_ui", damit ein ESP32-Arduino-Sketch (z.B. ktm_bringup.ino)
# per "#include <ui.h>" darauf zugreifen kann, OHNE den Code zu duplizieren.
#
# Grund fuer den Umweg ueber eine Arduino-Bibliothek statt eines einfachen
# Datei-Kopierens in den Sketch-Ordner: Arduino IDE kompiliert nur .c/.cpp-
# Dateien, die entweder direkt im Sketch-Ordner liegen oder Teil einer
# ordnungsgemaess strukturierten Bibliothek unter Arduino/libraries sind -
# beliebige relative Include-Pfade ("../../core/ui.h") wie im CMake-Build
# des PC-Simulators funktionieren dort nicht. Das offizielle Elecrow-
# Beispielprojekt macht es genauso: seine SquareLine-generierte UI liegt
# ebenfalls als separat zu installierende Bibliothek vor (siehe
# example/Arduino/libraries/UI im Elecrow-Repo).
#
# core/ui.c bleibt dabei die einzige Quelle der Wahrheit (PC-Simulator und
# ESP32 nutzen denselben Code, siehe README.md "Architektur") - dieses
# Skript kopiert nur, es aendert nichts inhaltlich.
set -euo pipefail
cd "$(dirname "$0")/.."

LIB_DIR="src/platform/esp32/libraries/ktm_ui"
mkdir -p "$LIB_DIR/src"

cp src/core/ui.c src/core/ui.h src/core/vehicle_data.h "$LIB_DIR/src/"
cp assets/generated/img_rpm_dial.c   assets/generated/img_rpm_dial.h   "$LIB_DIR/src/"
cp assets/generated/img_rpm_needle.c assets/generated/img_rpm_needle.h "$LIB_DIR/src/"
cp assets/generated/img_clock_dial.c assets/generated/img_clock_dial.h "$LIB_DIR/src/"
cp assets/generated/img_hour_needle.c assets/generated/img_hour_needle.h "$LIB_DIR/src/"
cp assets/generated/img_min_needle.c  assets/generated/img_min_needle.h "$LIB_DIR/src/"

cat > "$LIB_DIR/library.properties" << 'EOF'
name=ktm_ui
version=0.1.0
author=Digitaler Drehzahlmesser KTM Projekt
maintainer=Digitaler Drehzahlmesser KTM Projekt
sentence=Plattformneutrale LVGL-v9-UI (Drehzahlmesser + Analoguhr) fuer den digitalen KTM-Drehzahlmesser.
paragraph=Generierte Kopie von src/core/ (ui.c/ui.h/vehicle_data.h) und assets/generated/ - nicht von Hand bearbeiten, sondern tools/prepare_esp32_library.sh erneut laufen lassen.
category=Display
url=https://github.com/GarageMan/digital_drehzahlmesser_ktm
architectures=esp32
EOF

echo ""
echo "Fertig. '$LIB_DIR' enthaelt jetzt eine installierbare Arduino-Bibliothek."
echo "Naechster Schritt: den Ordner 'ktm_ui' (aus $LIB_DIR) in den Arduino-"
echo "libraries-Ordner kopieren (Sketchbook-Speicherort siehe Arduino IDE ->"
echo "Voreinstellungen), dann RotaryScreen_2_1 einmal unveraendert testen"
echo "(Checkpoint), danach src/platform/esp32/ktm_bringup/ktm_bringup.ino"
echo "oeffnen und hochladen."
