#!/usr/bin/env bash
# Holt die C-Abhaengigkeit (LVGL v9), die absichtlich NICHT im Repo/Zip
# mitgeliefert wird (>30MB Fremdcode). Version exakt gepinnt auf 9.1.0 -
# das ist die Version, die nachweislich im offiziellen Elecrow-Beispiel
# RotaryScreen_2_1 (example/Arduino/libraries/lvgl/library.json) steckt und
# mit der der reale Hardware-Bring-up-Test (Sprint 3, 22.08.) erfolgreich
# lief. lv_drivers wird NICHT mehr gebraucht: v9 bringt seinen SDL-Treiber
# (src/drivers/sdl/) selbst mit, kein separates Repo mehr noetig.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p lib
if [ ! -d lib/lvgl ]; then
  git clone --branch v9.1.0 --depth 1 https://github.com/lvgl/lvgl.git lib/lvgl
else
  echo "lib/lvgl existiert bereits, ueberspringe."
fi

echo ""
echo "Fertig. lv_conf.h liegt bereits fertig konfiguriert in lib/ (480x480,"
echo "LV_COLOR_DEPTH=16/RGB565, LV_USE_SDL=1) - nicht ueberschreiben."
echo "Weiter mit: mkdir build && cd build && cmake .. && make -j\$(nproc) && ./sim"
