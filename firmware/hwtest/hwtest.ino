/*
  hwtest.ino - hardware check for the Neo-Sapien-clone pendant prototype
  ---------------------------------------------------------------
  Board : ESP32C3 Dev Module
  Core  : esp32 by Espressif  -- works on BOTH 2.0.x and 3.x
          (3.x uses the newer driver/i2s_std.h API, 2.x the legacy one;
           the version is detected automatically, nothing to edit)

  REQUIRED setting:  USB CDC On Boot = Enabled
                     without it, Serial output is invisible

  Upload this, open Serial Monitor at 115200, press the board's RESET button,
  and paste the whole output back.

  It tests, in order, and does NOT stop if one part fails:
    1. chip info
    2. LED on GPIO8
    3. push button on GPIO2
    4. INMP441 mic   (SCK=0  WS=1  SD=3  VDD=3V3  GND=GND  L/R=GND)
    5. microSD card  (CS=7  SCK=6  MOSI=5  MISO=4  VCC=3V3  GND=GND)

  Leaving a module unplugged is fine - it prints FAILED and moves on.
  That is the intended way to use it: wire one thing, run it, repeat.
*/

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

// ---------- pins ----------
#define PIN_LED    8      // onboard LED, active LOW on the Super Mini
#define PIN_BTN    2      // push-to-talk button, other leg to GND

#define PIN_I2S_BCLK  0   // -> mic SCK (bit clock)
#define PIN_I2S_WS    1   // -> mic WS  (word select)
#define PIN_I2S_DIN   3   // -> mic SD  (data out of the mic)

#define PIN_SD_CS     7
#define PIN_SD_SCK    6
#define PIN_SD_MOSI   5
#define PIN_SD_MISO   4

// ---------- config ----------
#define SAMPLE_RATE      16000
#define I2S_BUF_FRAMES   256
#define MIC_SECONDS      4
#define LED_ACTIVE_LOW   1     // set to 0 if your LED behaves the other way

struct ChStat { double sq8=0, sq11=0, sq12=0; int32_t mn=0, mx=0; int64_t n=0; };

// =====================================================================
//  microphone driver - one thin wrapper, two implementations
// =====================================================================
#if ESP_ARDUINO_VERSION_MAJOR >= 3
// ---- core 3.x : new I2S driver (ESP-IDF 5.x) ----
#include <driver/i2s_std.h>

static i2s_chan_handle_t s_rx = nullptr;

static bool micInit() {
  i2s_chan_config_t ch = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
  ch.dma_desc_num  = 8;
  ch.dma_frame_num = I2S_BUF_FRAMES;
  if (i2s_new_channel(&ch, nullptr, &s_rx) != ESP_OK) {
    Serial.println(F("  i2s_new_channel failed"));
    return false;
  }

  i2s_std_config_t cfg = {};
  cfg.clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE);
  cfg.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT,
                                                    I2S_SLOT_MODE_STEREO);
  cfg.slot_cfg.slot_mask = I2S_STD_SLOT_BOTH;      // read both, report each
  cfg.gpio_cfg.mclk = I2S_GPIO_UNUSED;
  cfg.gpio_cfg.bclk = (gpio_num_t)PIN_I2S_BCLK;
  cfg.gpio_cfg.ws   = (gpio_num_t)PIN_I2S_WS;
  cfg.gpio_cfg.dout = I2S_GPIO_UNUSED;
  cfg.gpio_cfg.din  = (gpio_num_t)PIN_I2S_DIN;

  if (i2s_channel_init_std_mode(s_rx, &cfg) != ESP_OK) {
    Serial.println(F("  i2s_channel_init_std_mode failed"));
    return false;
  }
  return true;
}
static void micStart() { i2s_channel_enable(s_rx); }
static void micStop()  { i2s_channel_disable(s_rx); }
static void micFree()  { if (s_rx) { i2s_del_channel(s_rx); s_rx = nullptr; } }
static size_t micRead(void *buf, size_t len) {
  size_t br = 0;
  if (i2s_channel_read(s_rx, buf, len, &br, pdMS_TO_TICKS(300)) != ESP_OK) return 0;
  return br;
}

#else
// ---- core 2.0.x : legacy I2S driver (ESP-IDF 4.4) ----
#include <driver/i2s.h>

static bool micInit() {
  i2s_config_t cfg = {};
  cfg.mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX);
  cfg.sample_rate          = SAMPLE_RATE;
  cfg.bits_per_sample      = I2S_BITS_PER_SAMPLE_32BIT;
  cfg.channel_format       = I2S_CHANNEL_FMT_RIGHT_LEFT;
  cfg.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  cfg.intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1;
  cfg.dma_buf_count        = 8;
  cfg.dma_buf_len          = I2S_BUF_FRAMES;
  cfg.use_apll             = false;
  if (i2s_driver_install(I2S_NUM_0, &cfg, 0, nullptr) != ESP_OK) {
    Serial.println(F("  i2s_driver_install failed"));
    return false;
  }

  i2s_pin_config_t pins = {};
  pins.bck_io_num   = PIN_I2S_BCLK;
  pins.ws_io_num    = PIN_I2S_WS;
  pins.data_out_num = I2S_PIN_NO_CHANGE;
  pins.data_in_num  = PIN_I2S_DIN;
  i2s_set_pin(I2S_NUM_0, &pins);
  i2s_stop(I2S_NUM_0);          // stay idle until we ask for audio
  return true;
}
static void micStart() { i2s_start(I2S_NUM_0); }
static void micStop()  { i2s_stop(I2S_NUM_0); }
static void micFree()  { i2s_driver_uninstall(I2S_NUM_0); }
static size_t micRead(void *buf, size_t len) {
  size_t br = 0;
  if (i2s_read(I2S_NUM_0, buf, len, &br, 300 / portTICK_PERIOD_MS) != ESP_OK) return 0;
  return br;
}
#endif

// =====================================================================

static void led(bool on) {
#if LED_ACTIVE_LOW
  digitalWrite(PIN_LED, on ? LOW : HIGH);
#else
  digitalWrite(PIN_LED, on ? HIGH : LOW);
#endif
}

// ------------------------------------------------------------------ 1. chip
static void testChip() {
  Serial.println(F("--- 1. chip ---"));
  Serial.printf("arduino core : %d.%d.%d\n",
                ESP_ARDUINO_VERSION_MAJOR, ESP_ARDUINO_VERSION_MINOR, ESP_ARDUINO_VERSION_PATCH);
  Serial.printf("model        : %s\n", ESP.getChipModel());
  Serial.printf("cores/rev    : %d / %d\n", ESP.getChipCores(), ESP.getChipRevision());
  Serial.printf("cpu MHz      : %u\n", getCpuFrequencyMhz());
  Serial.printf("flash KB     : %u\n", ESP.getFlashChipSize() / 1024);
  Serial.printf("free heap    : %u bytes\n", ESP.getFreeHeap());
  Serial.printf("mac          : %04X%08X\n",
                (uint16_t)(ESP.getEfuseMac() >> 32), (uint32_t)ESP.getEfuseMac());
  Serial.println();
}

// ------------------------------------------------------------------ 2. LED
static void testLed() {
  Serial.println(F("--- 2. LED on GPIO8 ---"));
  Serial.println(F("blinking 6 times - watch the board"));
  for (int i = 0; i < 6; i++) {
    led(true);  delay(150);
    led(false); delay(150);
  }
  Serial.println(F("if you saw nothing, set LED_ACTIVE_LOW to 0 and re-upload.\n"));
}

// ------------------------------------------------------------------ 3. button
static void testButton() {
  Serial.println(F("--- 3. button on GPIO2 ---"));
  Serial.println(F("press and hold the button for the next 6 seconds..."));
  pinMode(PIN_BTN, INPUT_PULLUP);
  bool sawPress = false;
  uint32_t t0 = millis();
  while (millis() - t0 < 6000) {
    if (digitalRead(PIN_BTN) == LOW) sawPress = true;
    delay(10);
  }
  Serial.println(sawPress
    ? F("PASS - button detected\n")
    : F("no press seen - either you did not press it, or it is not wired to GPIO2/GND\n"));
}

// ------------------------------------------------------------------ 4. mic

static void addSample(ChStat &c, int32_t v) {
  if (v > c.mx) c.mx = v;
  if (v < c.mn) c.mn = v;
  c.sq8  += (double)(v >> 8)  * (v >> 8);
  c.sq11 += (double)(v >> 11) * (v >> 11);
  c.sq12 += (double)(v >> 12) * (v >> 12);
  c.n++;
}

static void printCh(const char *name, const ChStat &c) {
  Serial.printf("%s raw min/max : %ld / %ld\n", name, (long)c.mn, (long)c.mx);
  if (c.n == 0) return;
  Serial.printf("%s RMS shift 8 : %.1f\n", name, sqrt(c.sq8  / c.n));
  Serial.printf("%s RMS shift 11: %.1f\n", name, sqrt(c.sq11 / c.n));
  Serial.printf("%s RMS shift 12: %.1f\n", name, sqrt(c.sq12 / c.n));
}

static void listenPhase(const char *title, uint32_t ms) {
  static int32_t buf[I2S_BUF_FRAMES * 2];
  ChStat raw, fix, B;
  uint32_t prevBlsb = 0;
  bool havePrev = false;
  uint32_t t0 = millis();
  while (millis() - t0 < ms) {
    size_t br = micRead(buf, sizeof(buf));
    if (br == 0) break;
    int cnt = br / 4;
    for (int i = 0; i + 1 < cnt; i += 2) {
      int32_t a = buf[i], b = buf[i + 1];
      addSample(raw, a);
      addSample(B, b);
      if (havePrev) {
        // data arrives 1 bit early: sign bit sits in the previous slot B's LSB
        int32_t c = (int32_t)((prevBlsb << 31) | ((uint32_t)a >> 1));
        addSample(fix, c);
      }
      prevBlsb = (uint32_t)b & 1u;
      havePrev = true;
    }
  }
  Serial.printf("[%s]\n", title);
  Serial.printf("  slot B min/max      : %ld / %ld\n", (long)B.mn, (long)B.mx);
  if (raw.n) Serial.printf("  RAW   RMS shift 11  : %.1f   (min/max %ld / %ld)\n",
                           sqrt(raw.sq11 / raw.n), (long)raw.mn, (long)raw.mx);
  if (fix.n) Serial.printf("  FIXED RMS shift 11  : %.1f   shift 12: %.1f   (min/max %ld / %ld)\n",
                           sqrt(fix.sq11 / fix.n), sqrt(fix.sq12 / fix.n), (long)fix.mn, (long)fix.mx);
}

static void testMic() {
  Serial.println(F("--- 4. INMP441 microphone ---"));

  pinMode(PIN_I2S_DIN, INPUT_PULLUP);   delay(5);
  int withUp = digitalRead(PIN_I2S_DIN);
  pinMode(PIN_I2S_DIN, INPUT_PULLDOWN); delay(5);
  int withDown = digitalRead(PIN_I2S_DIN);
  pinMode(PIN_I2S_DIN, INPUT);
  Serial.printf("SD pin check : pullup->%d pulldown->%d  ", withUp, withDown);
  if (withUp == 0 && withDown == 0)      Serial.println(F("(stuck LOW - SD wire touching GND?)"));
  else if (withUp == 1 && withDown == 1) Serial.println(F("(stuck HIGH - SD wire touching 3.3V?)"));
  else                                   Serial.println(F("(ok - not shorted)"));

  if (!micInit()) { Serial.println(F("mic setup failed - test skipped\n")); return; }
  micStart();
  static int32_t warm[I2S_BUF_FRAMES * 2];
  for (int i = 0; i < 8; i++) micRead(warm, sizeof(warm));   // mic start-up

  Serial.println(F(">>> STAY SILENT for 3 s ..."));
  delay(300);
  listenPhase("silence", 3000);
  Serial.println(F(">>> NOW HUM LOUDLY near the mic for 3 s ..."));
  delay(700);
  listenPhase("humming", 3000);

  micStop();
  micFree();
  Serial.println(F("Paste both [silence] and [humming] blocks."));
  Serial.println(F("Good result: FIXED is small in silence (~50-2000) and much bigger when humming."));
  Serial.println();
}

// ------------------------------------------------------------------ 5. SD
static void testSD() {
  Serial.println(F("--- 5. microSD card ---"));
  SPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);

  if (!SD.begin(PIN_SD_CS, SPI, 4000000)) {
    Serial.println(F("SD.begin FAILED."));
    Serial.println(F("  no card in the slot / wiring / card is exFAT not FAT32 /"));
    Serial.println(F("  card not formatted. Reformat as FAT32 on a PC.\n"));
    return;
  }
  uint8_t type = SD.cardType();
  if (type == CARD_NONE) { Serial.println(F("no card detected\n")); return; }

  const char *tn = (type == CARD_MMC) ? "MMC" : (type == CARD_SD) ? "SDSC" : "SDHC/SDXC";
  Serial.printf("card type  : %s\n", tn);
  Serial.printf("card size  : %llu MB\n", SD.cardSize() / (1024ULL * 1024ULL));
  Serial.printf("used       : %llu MB of %llu MB\n",
                SD.usedBytes() / (1024ULL * 1024ULL),
                SD.totalBytes() / (1024ULL * 1024ULL));

  File root = SD.open("/");
  if (root) {
    Serial.println(F("root contents:"));
    int shown = 0;
    for (File f = root.openNextFile(); f && shown < 12; f = root.openNextFile()) {
      Serial.printf("   %-28s %8u bytes\n", f.name(), (unsigned)f.size());
      shown++;
    }
    root.close();
    if (shown == 0) Serial.println(F("   (empty)"));
  }

  File w = SD.open("/hwtest.txt", FILE_WRITE);
  if (!w) { Serial.println(F("could not create /hwtest.txt - card write-protected?\n")); return; }
  w.printf("pendant hardware test, uptime %lu ms\n", (unsigned long)millis());
  w.close();

  if (SD.exists("/hwtest.txt")) {
    Serial.print(F("read back OK: "));
    File r = SD.open("/hwtest.txt");
    while (r.available()) Serial.write(r.read());
    r.close();
    Serial.println(F("PASS - card is readable and writable\n"));
  } else {
    Serial.println(F("wrote a file but cannot see it again - card is failing\n"));
  }
}

// ------------------------------------------------------------------ run
static void runAll() {
  Serial.println();
  Serial.println(F("=========================================="));
  Serial.println(F("  pendant prototype - hardware test"));
  Serial.println(F("=========================================="));
  Serial.println();

  testChip();
  testLed();
  testButton();
  testMic();
  SD.end();                    // safe to re-run the SD test
  testSD();

  Serial.println(F("=========================================="));
  Serial.println(F("  test finished. Paste everything above."));
  Serial.println(F("  Press Enter in the Serial Monitor to run again."));
  Serial.println(F("=========================================="));
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  led(false);

  // Wait (up to 30 s) until the Serial Monitor is actually connected, so the
  // report is never lost while the USB link reconnects after RESET.
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 30000) {
    led(true);  delay(100);    // fast flicker = waiting for Serial Monitor
    led(false); delay(100);
  }
  delay(1000);

  runAll();
}

void loop() {
  if (Serial.available()) {    // Enter in the Serial Monitor = run again
    while (Serial.available()) Serial.read();
    runAll();
  }
  led(true);  delay(50);       // slow heartbeat: board is alive
  led(false); delay(1950);
}
