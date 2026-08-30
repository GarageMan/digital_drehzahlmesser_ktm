#!/usr/bin/env bash
# Holt die C-Abhaengigkeiten (LVGL v8 + lv_drivers), die absichtlich NICHT
# im Repo/Zip mitgeliefert werden (zusammen >100MB Fremdcode). Passend zur
# Version, mit der dieses Skelett gebaut und getestet wurde.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p lib
if [ ! -d lib/lvgl ]; then
  git clone --branch release/v8.3 --depth 1 https://github.com/lvgl/lvgl.git lib/lvgl
else
  echo "lib/lvgl existiert bereits, ueberspringe."
fi

if [ ! -d lib/lv_drivers ]; then
  git clone --branch release/v8.3 --depth 1 https://github.com/lvgl/lv_drivers.git lib/lv_drivers
else
  echo "lib/lv_drivers existiert bereits, ueberspringe."
fi

echo ""
echo "Fertig. lv_conf.h und lv_drv_conf.h liegen bereits fertig konfiguriert"
echo "in lib/ (480x480, LV_COLOR_DEPTH=32, USE_SDL=1) - nicht ueberschreiben."
echo "Weiter mit: mkdir build && cd build && cmake .. && make -j\$(nproc) && ./sim"
