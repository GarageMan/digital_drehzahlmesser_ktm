#!/usr/bin/env python3
"""
PC-Telemetrie-Sender fuer den CrowPanel-Bring-up-Test
(src/platform/esp32/bringup_pc_telemetry/bringup_pc_telemetry.ino).

Liest CPU-Temperatur/-Last, RAM-Auslastung und (falls verfuegbar) GPU-
Temperatur vom PC und schickt sie einmal pro Sekunde als einfache Textzeile
ueber die serielle USB-Verbindung an den ESP32:

    CPU:<temp_c>;GPU:<temp_c>;LOAD:<cpu_percent>;RAM:<ram_percent>\n

Aufruf:
    python3 pc_telemetry_sender.py [SERIELLER_PORT]

    # Linux-Beispiel:
    python3 pc_telemetry_sender.py /dev/ttyACM0
    # Windows-Beispiel:
    python3 pc_telemetry_sender.py COM5

Voraussetzungen:
    pip install pyserial psutil
    # optional, nur fuer NVIDIA-GPU-Temperatur:
    pip install pynvml

Hinweise zur Plattform-/Hersteller-Abhaengigkeit (bewusst nicht "geraten",
sondern hier offen benannt):

  - GPU-Temperatur: Es gibt kein einheitliches Cross-Vendor-API.
      * NVIDIA (Linux/Windows): funktioniert ueber pynvml, siehe
        read_gpu_temp_nvidia() unten.
      * AMD/Intel: hier NICHT implementiert. Unter Windows liesse sich das
        z.B. ueber die Web-API von LibreHardwareMonitor nachruesten, unter
        Linux z.B. ueber `rocm-smi` (AMD) oder `sensors` (lm-sensors).
      * Ohne passenden Weg wird -1.0 gesendet - das Display zeigt dann
        "n/a" auf dem GPU-Screen.

  - CPU-Temperatur ueber psutil.sensors_temperatures():
      * Linux: funktioniert in der Regel direkt (Sensor-Namen wie
        "coretemp"/"k10temp"/"cpu_thermal" werden unten bevorzugt gesucht,
        sonst wird der erste verfuegbare Sensor genommen).
      * Windows/macOS: psutil bietet hier haeufig KEINE Werte (API existiert
        z.T. nicht). Workaround unter Windows z.B. ueber
        LibreHardwareMonitor (eigene Bibliothek/Web-API, hier nicht
        eingebunden). In diesem Fall wird -1.0 gesendet.
"""

import sys
import time

try:
    import serial
except ImportError:
    print("Fehlt: pyserial. Installieren mit: pip install pyserial")
    sys.exit(1)

try:
    import psutil
except ImportError:
    print("Fehlt: psutil. Installieren mit: pip install psutil")
    sys.exit(1)

DEFAULT_SERIAL_PORT = "/dev/ttyACM0"  # unter Windows z.B. "COM5"
BAUD_RATE = 115200
SEND_INTERVAL_S = 1.0


def read_cpu_temp_c():
    """Best-effort CPU-Temperatur in Grad Celsius, -1.0 wenn nicht verfuegbar."""
    sensors_fn = getattr(psutil, "sensors_temperatures", None)
    if sensors_fn is None:
        return -1.0  # z.B. macOS: psutil kennt diese Funktion dort gar nicht
    try:
        temps = sensors_fn()
    except Exception:
        return -1.0
    if not temps:
        return -1.0
    for key in ("coretemp", "k10temp", "cpu_thermal", "zenpower"):
        if key in temps and temps[key]:
            return float(temps[key][0].current)
    first_key = next(iter(temps))
    if temps[first_key]:
        return float(temps[first_key][0].current)
    return -1.0


def read_gpu_temp_nvidia():
    """NVIDIA-GPU-Temperatur via pynvml. -1.0, wenn pynvml fehlt oder keine
    NVIDIA-GPU gefunden wird (dann bewusst kein Fallback auf andere
    Hersteller - siehe Hinweis im Modul-Docstring)."""
    try:
        import pynvml
        pynvml.nvmlInit()
        handle = pynvml.nvmlDeviceGetHandleByIndex(0)
        temp = pynvml.nvmlDeviceGetTemperature(handle, pynvml.NVML_TEMPERATURE_GPU)
        pynvml.nvmlShutdown()
        return float(temp)
    except Exception:
        return -1.0


def main():
    port = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_SERIAL_PORT
    print(f"Verbinde mit {port} @ {BAUD_RATE} Baud ...")
    try:
        ser = serial.Serial(port, BAUD_RATE, timeout=1)
    except serial.SerialException as exc:
        print(f"Konnte seriellen Port {port} nicht oeffnen: {exc}")
        print("Tipp: Port-Namen als Argument angeben, z.B. 'python3 pc_telemetry_sender.py COM5'")
        sys.exit(1)

    with ser:
        time.sleep(2.0)  # ESP32 bootet nach dem Oeffnen des seriellen Ports i.d.R. neu
        print("Verbunden. Sende Telemetrie einmal pro Sekunde (Strg+C zum Beenden) ...")
        # Erster Aufruf von cpu_percent() liefert per Definition 0.0 zurueck
        # (Referenzmessung) - direkt danach schon mal "verbrauchen".
        psutil.cpu_percent(interval=None)
        try:
            while True:
                cpu_temp = read_cpu_temp_c()
                gpu_temp = read_gpu_temp_nvidia()
                cpu_load = psutil.cpu_percent(interval=None)
                ram_pct = psutil.virtual_memory().percent

                line = f"CPU:{cpu_temp:.1f};GPU:{gpu_temp:.1f};LOAD:{cpu_load:.1f};RAM:{ram_pct:.1f}\n"
                ser.write(line.encode("ascii"))
                print(line.strip())
                time.sleep(SEND_INTERVAL_S)
        except KeyboardInterrupt:
            print("\nBeendet.")


if __name__ == "__main__":
    main()
