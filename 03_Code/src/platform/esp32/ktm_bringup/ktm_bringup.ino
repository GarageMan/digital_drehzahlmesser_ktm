/*
 * KTM-Bring-up-Sketch: Drehzahlmesser + Analoguhr auf dem echten CrowPanel
 * (ESP32-S3R8), touch-umschaltbar. Nadel-Sweep und Uhr sind simuliert
 * (siehe sweep_provider_* unten) - Ziel ist die Anzeige-/Touch-Verifikation
 * mit dem ECHTEN core/ui.c auf echter Hardware, nicht das Auslesen echter
 * Sensoren/CAN-Daten (das ist ein spaeterer Schritt, siehe
 * Sprint3_Code_Grundgeruest.md Punkt 9 "Offene Punkte").
 *
 * Voraussetzung: RotaryScreen_2_1.ino (offizielles Elecrow-Beispiel) wurde
 * bereits unveraendert erfolgreich getestet (Checkpoint, siehe Sprint 3,
 * Nachtrag 22.08.) - Arduino-IDE-Einrichtung (Board-Paket, ESP32-Core
 * 2.0.17, Partition "Huge APP", PSRAM "OPI PSRAM", Bibliotheken
 * Arduino_GFX_Library/Adafruit_CST8XX/PCF8574/lvgl 9.1.0) ist identisch.
 * Zusaetzlich noetig: `bash tools/prepare_esp32_library.sh` einmal laufen
 * lassen und den entstandenen Ordner "ktm_ui" in den Arduino-libraries-
 * Ordner kopieren (siehe README.md in diesem Ordner).
 *
 * Display-/Touch-/PCF8574-Init-Block unten ist 1:1 aus RotaryScreen_2_1.ino
 * uebernommen (Elecrow-RD/CrowPanel-2.1inch-HMI-ESP32-Rotary-Display-480-
 * 480-IPS-Round-Touch-Knob-Screen, Branch master) - bewusst NICHT selbst
 * erfunden (siehe Sprint-1-Lehre zu erfundenen Pin-Tabellen). Nur der
 * SquareLine-generierte Menu-Teil (ui_init(), Volume/Temp/Light-Screens,
 * Encoder-Navigation) wurde weggelassen und durch core/ui.c (ktm_ui-
 * Bibliothek) ersetzt.
 *
 * Bewusst NICHT Teil dieses Bring-up-Tests (naechste Schritte, nicht hier):
 *   - Encoder-Drehen/-Taste (Pins 42/4 bzw. PCF8574 P5 sind bestaetigt,
 *     siehe Sprint 3 Abschnitt 6/7, aber fuer "Touch-Umschaltung" laut
 *     Aufgabenstellung nicht erforderlich - spart Komplexitaet/Fehlerflaeche
 *     fuer diesen ersten Durchlauf)
 *   - UART1/CAN (Pegelwandler-Frage aus Sprint 3 Nachtrag 22.08. weiterhin
 *     offen, hier nicht relevant)
 *   - echte RPM-/Uhr-Sensoren (Kurbelwellensensor, DS3231-RTC)
 */
#include <lvgl.h>
#include <Arduino_GFX_Library.h>
#include <Adafruit_CST8XX.h>
#include "PCF8574.h"
#include "ui.h"            /* aus der ktm_ui-Bibliothek, siehe oben */
#include "vehicle_data.h"

/*---------------------------------------------------------------
 * Bestaetigte Hardware-Anbindung (1:1 aus RotaryScreen_2_1.ino, siehe
 * Sprint3_Code_Grundgeruest.md Abschnitt 7 fuer die Herkunft der Werte).
 *--------------------------------------------------------------*/
#define I2C_SDA_PIN 38
#define I2C_SCL_PIN 39
PCF8574 pcf8574(0x21);

#define SCREEN_BACKLIGHT_PIN 6
const int pwmFreq = 5000;
const int pwmResolution = 8;

#define I2C_TOUCH_ADDR 0x15
Adafruit_CST8XX tsPanel = Adafruit_CST8XX();
static const uint16_t screenWidth = 480;
static const uint16_t screenHeight = 480;

static uint8_t *buf1 = NULL;
static uint8_t *buf2 = NULL;

/*---------------------------------------------------------------
 * RGB-Display-Treiber (ST7701, 480x480) - Werte 1:1 aus RotaryScreen_2_1.ino
 *--------------------------------------------------------------*/
Arduino_DataBus *panelInitBus = new Arduino_SWSPI(
  GFX_NOT_DEFINED /* DC: ST7701 nutzt 9-bit SPI */, 16 /* CS */,
  2 /* SCK */, 1 /* SDA */, GFX_NOT_DEFINED /* MISO */);

Arduino_ESP32RGBPanel *rgbPanel = new Arduino_ESP32RGBPanel(
  40 /* DE */, 7 /* VSYNC */, 15 /* HSYNC */, 41 /* PCLK */,
  46 /* R0 */, 3 /* R1 */, 8 /* R2 */, 18 /* R3 */, 17 /* R4 */,
  14 /* G0 */, 13 /* G1 */, 12 /* G2 */, 11 /* G3 */, 10 /* G4 */, 9 /* G5 */,
  5 /* B0 */, 45 /* B1 */, 48 /* B2 */, 47 /* B3 */, 21 /* B4 */,
  1 /* hsync polarity */, 10 /* front porch */, 4 /* pulse width */, 20 /* back porch */,
  1 /* vsync polarity */, 10 /* front porch */, 4 /* pulse width */, 20 /* back porch */,
  0 /* pclk active neg */, 12000000 /* pixel clock */, false /* native endian */,
  0 /* de idle high */, 0 /* pclk idle high */,
  480 * 20 /* zwei interne DMA-Bounce-Buffer, je 20 Zeilen */);

Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
  480 /* width */, 480 /* height */, rgbPanel, 0 /* rotation */, true /* auto flush */,
  panelInitBus, GFX_NOT_DEFINED /* RST */,
  st7701_type5_init_operations, sizeof(st7701_type5_init_operations));

/**
 * Flush-Callback: RGB565 von LVGL -> Panel. Der R/B-Tausch ist eine
 * bestaetigte, hardwarespezifische Korrektur fuer dieses ST7701-Board
 * (siehe RotaryScreen_2_1.ino meint dazu: "ESP-IDF 5's RGB path on this
 * ST7701 board presents the red and blue 5-bit fields in the opposite
 * order from the legacy working driver") - nicht selbst hergeleitet,
 * sondern aus dem offiziellen Beispiel uebernommen.
 */
void my_disp_flush(lv_display_t *display, const lv_area_t *area, uint8_t *px_map) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  uint16_t *pixels = (uint16_t *)px_map;
  const uint32_t pixelCount = w * h;
  for (uint32_t i = 0; i < pixelCount; ++i) {
    const uint16_t c = pixels[i];
    pixels[i] = (c & 0x07E0) | ((c & 0x001F) << 11) | ((c & 0xF800) >> 11);
  }
  gfx->draw16bitRGBBitmap(area->x1, area->y1, pixels, w, h);
  lv_display_flush_ready(display);
}

/**
 * Touch-Callback: reine Punkt-Abfrage fuer LVGL, ohne die Wisch-Gesten-
 * Logik des offiziellen Beispiels (die galt dem dortigen SquareLine-Menue,
 * core/ui.c braucht nur einen einfachen Tap/Klick auf den Vollbild-Screen).
 * Der Y-Versatz "-20" ist eine bestaetigte Kalibrierung dieses Touch-
 * Panels (siehe RotaryScreen_2_1.ino), keine Erfindung.
 */
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  (void)indev;
  if (tsPanel.touched()) {
    CST_TS_Point p = tsPanel.getPoint(0);
    data->point.x = constrain(p.x, 0, screenWidth - 1);
    data->point.y = constrain(p.y - 20, 0, screenHeight - 1);
    data->state = LV_INDEV_STATE_PR;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

void initBacklight() {
  ledcAttach(SCREEN_BACKLIGHT_PIN, pwmFreq, pwmResolution);
  ledcWrite(SCREEN_BACKLIGHT_PIN, 204); /* ~80% */
}

/*---------------------------------------------------------------
 * vehicle_data_provider_t: automatischer RPM-Sweep + laufende Uhr, rein
 * zur Anzeige-Verifikation ohne echte Sensoren (Interface siehe
 * core/vehicle_data.h). PC-Gegenstueck mit Tastatursteuerung:
 * src/platform/sim/sim_provider.c.
 *--------------------------------------------------------------*/
static vehicle_data_t g_data;
static bool g_rpm_rising = true;
static uint32_t g_clock_ms_acc = 0;

static void sweep_provider_init(void) {
  g_data.rpm = 0;
  g_data.rtc_hour = 10;
  g_data.rtc_minute = 32;
  g_data.rpm_source = DATA_SRC_SIM;
}

static void sweep_provider_update(uint32_t elapsed_ms) {
  if (elapsed_ms == 0) elapsed_ms = 1;

  /* ca. 3000 RPM/s, wie im PC-Simulator */
  int32_t delta = (int32_t)(3000.0f * (elapsed_ms / 1000.0f));
  if (delta < 1) delta = 1;
  if (g_rpm_rising) {
    int32_t r = (int32_t)g_data.rpm + delta;
    if (r >= 9500) { r = 9500; g_rpm_rising = false; }
    g_data.rpm = (uint16_t)r;
  } else {
    int32_t r = (int32_t)g_data.rpm - delta;
    if (r <= 0) { r = 0; g_rpm_rising = true; }
    g_data.rpm = (uint16_t)r;
  }

  /* 60x Realzeit, wie im PC-Simulator, damit der Minutenzeiger sichtbar laeuft */
  g_clock_ms_acc += elapsed_ms * 60;
  if (g_clock_ms_acc >= 60000) {
    g_clock_ms_acc -= 60000;
    g_data.rtc_minute++;
    if (g_data.rtc_minute >= 60) {
      g_data.rtc_minute = 0;
      g_data.rtc_hour = (g_data.rtc_hour + 1) % 24;
    }
  }
}

static vehicle_data_t sweep_provider_get(void) {
  return g_data;
}

static const vehicle_data_provider_t sweep_provider = {
  sweep_provider_init,
  sweep_provider_update,
  sweep_provider_get,
};

/*---------------------------------------------------------------
 * setup() / loop()
 *--------------------------------------------------------------*/
uint32_t g_last_ms = 0;

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("[BOOT] KTM-Bring-up: Drehzahlmesser + Uhr (simulierte Werte)");

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  pcf8574.pinMode(P0, OUTPUT);        // tp RST
  pcf8574.pinMode(P2, OUTPUT);        // tp INT
  pcf8574.pinMode(P3, OUTPUT);        // lcd power
  pcf8574.pinMode(P4, OUTPUT);        // lcd reset

  Serial.print("Init pcf8574...\n");
  if (pcf8574.begin()) {
    Serial.println("pcf8574 OK");
  } else {
    Serial.println("pcf8574 KO");
  }

  pcf8574.digitalWrite(P3, HIGH);
  delay(100);

  /* lcd reset */
  pcf8574.digitalWrite(P4, HIGH);
  delay(100);
  pcf8574.digitalWrite(P4, LOW);
  delay(120);
  pcf8574.digitalWrite(P4, HIGH);
  delay(120);

  /* tp RST */
  pcf8574.digitalWrite(P0, HIGH);
  delay(100);
  pcf8574.digitalWrite(P0, LOW);
  delay(120);
  pcf8574.digitalWrite(P0, HIGH);
  delay(120);
  /* tp INT */
  pcf8574.digitalWrite(P2, HIGH);
  delay(120);

  gfx->begin();
  /* BGR-Bit in MADCTL setzen, damit Farben auf diesem Panel stimmen
   * (bestaetigt aus RotaryScreen_2_1.ino) */
  panelInitBus->beginWrite();
  panelInitBus->writeCommand(0x36);
  panelInitBus->write(0x08);
  panelInitBus->endWrite();
  gfx->fillScreen(0x0000);

  if (!tsPanel.begin(&Wire, I2C_TOUCH_ADDR)) {
    Serial.println("No touchscreen found");
  } else {
    Serial.println("Touchscreen found");
  }

  lv_init();
  lv_tick_set_cb(millis);

  size_t buffer_size = sizeof(uint16_t) * screenWidth * screenHeight;
  buf1 = (uint8_t *)heap_caps_malloc(buffer_size, MALLOC_CAP_SPIRAM);
  buf2 = (uint8_t *)heap_caps_malloc(buffer_size, MALLOC_CAP_SPIRAM);
  if (!buf1 || !buf2) {
    Serial.println("[FATAL] LVGL framebuffer allocation failed");
    while (true) delay(1000);
  }

  lv_display_t *display = lv_display_create(screenWidth, screenHeight);
  lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
  lv_display_set_flush_cb(display, my_disp_flush);
  lv_display_set_buffers(display, buf1, buf2, buffer_size, LV_DISPLAY_RENDER_MODE_PARTIAL);

  lv_indev_t *touch_indev = lv_indev_create();
  lv_indev_set_type(touch_indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(touch_indev, my_touchpad_read);
  lv_indev_set_display(touch_indev, display);

  ui_build(); /* aus core/ui.c (ktm_ui-Bibliothek) - identisch zum PC-Simulator */

  delay(200);
  initBacklight();
  pcf8574.digitalWrite(P3, LOW);

  sweep_provider.init();

  g_last_ms = millis();
  Serial.println("Setup abgeschlossen. Tap auf das Display schaltet Drehzahlmesser<->Uhr um.");
}

void loop() {
  uint32_t now_ms = millis();
  uint32_t elapsed_ms = now_ms - g_last_ms;
  g_last_ms = now_ms;

  lv_timer_handler();
  sweep_provider.update(elapsed_ms);

  vehicle_data_t data = sweep_provider.get();
  ui_tick(&data, elapsed_ms);

  delay(5);
}
