/*
 * NeoClone — cheap AI-pendant voice-memo recorder
 * ------------------------------------------------------------------
 * Hardware : ESP32-C3 Super Mini + INMP441 (I2S) + microSD (SPI)
 *            + one push button + status LED + TP4056 + LiPo 3.7V
 * Arduino  : esp32 core 3.x (tested on 3.3.12, ESP32-C3 Super Mini)
 * Board    : "ESP32C3 Dev Module", USB CDC On Boot = Enabled
 *
 * Button:
 *   click  (idle)          -> start recording
 *   click  (recording)     -> stop and save
 *   hold >= SYNC_HOLD_MS   -> stop recording, start WiFi sync hotspot
 *   click  (sync mode)     -> leave sync mode
 *
 * Sync (hold 3s):
 *   phone joins SSID "NeoClone" / pass "neoclone1"
 *   browse to http://192.168.4.1  -> list / download / delete WAVs
 *
 * Idle uses light sleep so a 500 mAh cell lasts weeks.
 *
 * Mic path verified on the bench (Oct 2026): stereo I2S read, mic in slot A
 * (L/R = GND), 24-bit sample, DC removed, /GAIN_DIV to 16-bit. Normal speech
 * at 20-30 cm gave rms ~2000 with no clipping at GAIN_DIV 128.
 */

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <WebServer.h>
#include <math.h>
#include <esp_sleep.h>

// ---------------- pins ----------------
#define PIN_I2S_BCLK 0
#define PIN_I2S_WS   1
#define PIN_I2S_DIN  3

#define PIN_SD_CS    7
#define PIN_SD_SCK   6
#define PIN_SD_MOSI  5
#define PIN_SD_MISO  4

#define PIN_BTN      2    // button to GND, internal pull-up
#define PIN_LED      8    // onboard LED, active LOW on the Super Mini

// ---------------- config ----------------
#define SAMPLE_RATE      16000
#define GAIN_DIV         128.0f // 24-bit -> 16-bit is /256; 128 = 2x. Lower = louder
#define I2S_BUF_FRAMES   256

#define USE_LIGHT_SLEEP  0      // 1 = weeks of standby (breaks USB serial), 0 = bench mode
#define SYNC_HOLD_MS     3000   // hold this long for the WiFi hotspot
#define SYNC_TIMEOUT_MS  300000 // leave sync after 5 min of no activity

#define AP_SSID          "NeoClone"
#define AP_PASS          "neoclone1"  // CHANGE THIS

// =====================================================================
//  microphone driver — one wrapper, two implementations
//  build with esp32 core 3.x -> new i2s_std driver
//  build with esp32 core 2.x -> legacy i2s driver
// =====================================================================
#if ESP_ARDUINO_VERSION_MAJOR < 3
#error "Needs esp32 Arduino core 3.x (Boards Manager -> esp32 by Espressif)"
#endif
#include <driver/i2s_std.h>

static i2s_chan_handle_t s_rx = nullptr;

// Read BOTH slots: on this board/mic combo that is the proven working setup.
// The mic (L/R = GND) is in slot A; slot B is ignored.
static bool micInit() {
  i2s_chan_config_t ch = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
  ch.dma_desc_num  = 8;
  ch.dma_frame_num = I2S_BUF_FRAMES;
  if (i2s_new_channel(&ch, nullptr, &s_rx) != ESP_OK) return false;

  i2s_std_config_t cfg = {};
  cfg.clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE);
  cfg.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT,
                                                    I2S_SLOT_MODE_STEREO);
  cfg.slot_cfg.slot_mask = I2S_STD_SLOT_BOTH;
  cfg.gpio_cfg.mclk = I2S_GPIO_UNUSED;
  cfg.gpio_cfg.bclk = (gpio_num_t)PIN_I2S_BCLK;
  cfg.gpio_cfg.ws   = (gpio_num_t)PIN_I2S_WS;
  cfg.gpio_cfg.dout = I2S_GPIO_UNUSED;
  cfg.gpio_cfg.din  = (gpio_num_t)PIN_I2S_DIN;

  return i2s_channel_init_std_mode(s_rx, &cfg) == ESP_OK;
}
static void micStart() { i2s_channel_enable(s_rx); }
static void micStop()  { i2s_channel_disable(s_rx); }
static size_t micRead(void *buf, size_t len) {
  size_t br = 0;
  if (i2s_channel_read(s_rx, buf, len, &br, pdMS_TO_TICKS(200)) != ESP_OK) return 0;
  return br;
}

static void led(bool on) { digitalWrite(PIN_LED, on ? LOW : HIGH); }

// ---------------- state ----------------
enum Mode { MODE_IDLE, MODE_REC, MODE_SYNC };

static Mode      g_mode       = MODE_IDLE;
static File      g_file;
static uint32_t  g_dataBytes  = 0;
static uint32_t  g_fileIndex  = 0;
static int32_t   g_i2sBuf[I2S_BUF_FRAMES * 2];   // stereo frames: A,B,A,B...
static float     g_dcX = 0, g_dcY = 0;           // DC-removal filter state
static uint32_t  g_lastFlush = 0;
static WebServer g_server(80);
static uint32_t  g_lastSyncActivity = 0;

// ---------------------------------------------------------------- SD helpers

static void findNextIndex() {
  char name[32];
  for (uint32_t i = 1; i <= 9999; i++) {
    snprintf(name, sizeof(name), "/REC_%04lu.WAV", (unsigned long)i);
    if (!SD.exists(name)) { g_fileIndex = i; return; }
  }
  g_fileIndex = 1;
}

static void writeWavHeader(File &f, uint32_t dataBytes) {
  uint8_t h[44];
  uint32_t riffSize   = 36 + dataBytes;
  uint32_t byteRate   = SAMPLE_RATE * 2;
  uint16_t blockAlign = 2;
  uint32_t fmtSize    = 16;
  uint16_t audioFmt   = 1;
  uint16_t channels   = 1;
  uint32_t rate       = SAMPLE_RATE;
  uint16_t bits       = 16;

  memcpy(h + 0,  "RIFF", 4);
  memcpy(h + 4,  &riffSize, 4);
  memcpy(h + 8,  "WAVE", 4);
  memcpy(h + 12, "fmt ", 4);
  memcpy(h + 16, &fmtSize, 4);
  memcpy(h + 20, &audioFmt, 2);
  memcpy(h + 22, &channels, 2);
  memcpy(h + 24, &rate, 4);
  memcpy(h + 28, &byteRate, 4);
  memcpy(h + 32, &blockAlign, 2);
  memcpy(h + 34, &bits, 2);
  memcpy(h + 36, "data", 4);
  memcpy(h + 40, &dataBytes, 4);
  f.write(h, 44);
}

// ---------------------------------------------------------------- recording

static bool startRecording() {
  findNextIndex();
  char name[32];
  snprintf(name, sizeof(name), "/REC_%04lu.WAV", (unsigned long)g_fileIndex);
  g_file = SD.open(name, FILE_WRITE);
  if (!g_file) {
    Serial.printf("!! cannot open %s\n", name);
    return false;
  }
  writeWavHeader(g_file, 0);   // placeholder, patched on stop
  g_dataBytes = 0;

  micStart();
  for (int i = 0; i < 6; i++)            // mic start-up (~100 ms) + stale DMA
    micRead(g_i2sBuf, sizeof(g_i2sBuf));
  g_dcX = g_dcY = 0;
  g_lastFlush = millis();

  g_mode = MODE_REC;
  led(true);
  Serial.printf("recording -> %s\n", name);
  return true;
}

static void stopRecording() {
  if (g_mode != MODE_REC) return;
  micStop();

  g_file.seek(0);
  writeWavHeader(g_file, g_dataBytes);   // patch real sizes
  g_file.flush();
  g_file.close();
  led(false);
  g_mode = MODE_IDLE;
  Serial.printf("saved: %lu bytes (~%lu s)\n",
                (unsigned long)g_dataBytes,
                (unsigned long)(g_dataBytes / (SAMPLE_RATE * 2)));
}

// Drain the I2S DMA into the open file. Call often while recording.
static void pumpAudio() {
  if (g_mode != MODE_REC) return;

  size_t bytesRead = micRead(g_i2sBuf, sizeof(g_i2sBuf));
  if (bytesRead == 0) return;

  size_t frames = bytesRead / 8;                 // 2 slots x 4 bytes
  static int16_t out[I2S_BUF_FRAMES];
  for (size_t i = 0; i < frames; i++) {
    float x = (float)(g_i2sBuf[2 * i] >> 8);     // slot A, 24-bit sample
    float y = x - g_dcX + 0.995f * g_dcY;        // remove DC / slow drift
    g_dcX = x; g_dcY = y;
    float o = y / GAIN_DIV;
    if (o > 32767)  o = 32767;
    if (o < -32768) o = -32768;
    out[i] = (int16_t)o;
  }
  g_file.write((const uint8_t *)out, frames * 2);
  g_dataBytes += frames * 2;

  // every 5 s, patch the header and flush so a crash or dead battery
  // still leaves a playable file
  if (millis() - g_lastFlush > 5000) {
    uint32_t pos = g_file.position();
    g_file.seek(0);
    writeWavHeader(g_file, g_dataBytes);
    g_file.seek(pos);
    g_file.flush();
    g_lastFlush = millis();
  }
}

// ---------------------------------------------------------------- sync server

// The page the phone sees. Kept inline so there's nothing extra to flash.
static const char SYNC_PAGE[] PROGMEM = R"HTML(
<!doctype html><html><head>
<meta name=viewport content="width=device-width,initial-scale=1">
<title>NeoClone</title>
<style>
 body{font-family:system-ui,sans-serif;background:#14161a;color:#e8eaed;margin:0;padding:18px}
 h1{font-size:19px} a{color:#8ab4f8}
 .row{display:flex;justify-content:space-between;align-items:center;
      background:#1e2126;border-radius:10px;padding:12px;margin:8px 0}
 .row span{font-size:14px}
 .btn{background:#2b6cb0;color:#fff;text-decoration:none;padding:8px 14px;
      border-radius:8px;font-size:14px}
 .del{background:#7f1d1d;margin-left:6px}
 .all{display:inline-block;margin:10px 0 18px;background:#276749;padding:11px 16px;
      border-radius:9px;color:#fff;text-decoration:none;font-weight:600}
 small{color:#9aa0a6}
</style></head><body>
<h1>NeoClone recordings</h1>
<a class=all href="/all">Get every recording</a>
<p><small>Join WiFi "NeoClone" first. Files stay on the device until you delete them.</small></p>
<div id=list></div>
<script>
async function refresh(){
  const r = await fetch('/list'); const files = await r.json();
  const el = document.getElementById('list');
  if(!files.length){ el.innerHTML = '<p><small>No recordings yet.</small></p>'; return; }
  el.innerHTML = files.map(f =>
    `<div class=row><span>${f.name}<br><small>${(f.size/1024).toFixed(0)} KB</small></span>
     <span><a class=btn href="/dl?f=${f.name}">Get</a>
     <a class="btn del" href="/del?f=${f.name}" onclick="return confirm('Delete ${f.name}?')">Del</a></span></div>`
  ).join('');
}
refresh();
</script></body></html>
)HTML";

static void handleRoot() {
  g_lastSyncActivity = millis();
  g_server.send_P(200, "text/html", SYNC_PAGE);
}

static void handleList() {
  g_lastSyncActivity = millis();
  String json = "[";
  File dir = SD.open("/");
  bool first = true;
  for (File f = dir.openNextFile(); f; f = dir.openNextFile()) {
    String nm = f.name();
    if (!nm.endsWith(".WAV") && !nm.endsWith(".wav")) { f.close(); continue; }
    if (!nm.startsWith("/")) nm = "/" + nm;
    if (!first) json += ",";
    first = false;
    json += "{\"name\":\"" + nm + "\",\"size\":" + String((uint32_t)f.size()) + "}";
    f.close();
  }
  dir.close();
  json += "]";
  g_server.send(200, "application/json", json);
}

static void handleDownload() {
  g_lastSyncActivity = millis();
  String name = g_server.arg("f");
  if (!name.startsWith("/")) name = "/" + name;
  if (!SD.exists(name)) { g_server.send(404, "text/plain", "not found"); return; }
  File f = SD.open(name, FILE_READ);
  if (!f) { g_server.send(500, "text/plain", "open failed"); return; }
  g_server.sendHeader("Content-Disposition", "attachment; filename=\"" + name.substring(1) + "\"");
  g_server.streamFile(f, "audio/wav");
  f.close();
}

static void handleDelete() {
  g_lastSyncActivity = millis();
  String name = g_server.arg("f");
  if (!name.startsWith("/")) name = "/" + name;
  bool ok = SD.remove(name);
  g_server.sendHeader("Location", "/");
  g_server.send(303, "text/plain", ok ? "deleted" : "failed");
}

// A real .zip would need a store-only encoder with two passes over the card.
// Instead we return a plain page of download links, which every phone can tap
// through (long-press -> "Download link" works too). Honest and simple.
static void handleAll() {
  g_lastSyncActivity = millis();
  String html = "<meta name=viewport content='width=device-width,initial-scale=1'>"
                "<body style='font-family:system-ui;background:#14161a;color:#e8eaed'>"
                "<h3>Tap each to download:</h3>";
  File dir = SD.open("/");
  for (File f = dir.openNextFile(); f; f = dir.openNextFile()) {
    String nm = f.name();
    if (nm.endsWith(".WAV") || nm.endsWith(".wav")) {
      if (!nm.startsWith("/")) nm = "/" + nm;
      html += "<p><a style='color:#8ab4f8' href=\"/dl?f=" + nm + "\">" + nm + "</a></p>";
    }
    f.close();
  }
  dir.close();
  g_server.send(200, "text/html", html);
}

static void enterSyncMode() {
  stopRecording();
  Serial.println("sync mode: joining WiFi...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);   // Super Mini: hotspot is often invisible at full power
  delay(200);
  Serial.printf("AP '%s' at http://%s\n", AP_SSID,
                WiFi.softAPIP().toString().c_str());

  g_server.begin();
  g_lastSyncActivity = millis();
  g_mode = MODE_SYNC;
}

static void exitSyncMode() {
  g_server.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  led(false);
  g_mode = MODE_IDLE;
  Serial.println("sync mode off");
}

static void serviceSync() {
  g_server.handleClient();
  if (millis() - g_lastSyncActivity > SYNC_TIMEOUT_MS) {
    Serial.println("sync timeout");
    exitSyncMode();
  }
  // fast blink while in sync mode
  led((millis() / 150) % 2);
}

// ---------------------------------------------------------------- power

static void enterIdleSleep() {
#if USE_LIGHT_SLEEP
  WiFi.mode(WIFI_OFF);
  gpio_wakeup_enable((gpio_num_t)PIN_BTN, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();
  esp_light_sleep_start();      // returns when the button is pressed
  // Debounce the wake source a little.
  delay(25);
#else
  delay(20);
#endif
}

// ---------------------------------------------------------------- main

// Simple click/hold detector. Returns: 0 none, 1 click, 2 hold.
static uint8_t readButton() {
  static bool     down = false;
  static uint32_t downAt = 0;
  bool pressed = (digitalRead(PIN_BTN) == LOW);

  if (pressed && !down) {
    down = true;
    downAt = millis();
  } else if (!pressed && down) {
    down = false;
    uint32_t held = millis() - downAt;
    if (held >= SYNC_HOLD_MS) return 2;   // hold
    if (held >= 40)           return 1;   // click (debounced)
  }
  return 0;
}

void setup() {
  Serial.begin(115200);
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 3000) delay(20);   // brief wait, never blocks on battery

  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  led(false);

  SPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
  if (!SD.begin(PIN_SD_CS, SPI, 4000000)) {
    Serial.println("!! SD mount failed (check wiring / FAT32).");
    for (bool on = false;; on = !on) { led(on); delay(80); }   // fast blink = SD problem
  }
  Serial.printf("SD ok, %llu MB\n",
                (unsigned long long)(SD.cardSize() / (1024ULL * 1024ULL)));

  if (!micInit()) {
    Serial.println("!! I2S mic init failed (check wiring).");
  }
  WiFi.mode(WIFI_OFF);

  // Routes are registered once; enterSyncMode() only brings the AP up.
  g_server.on("/",     HTTP_GET, handleRoot);
  g_server.on("/list", HTTP_GET, handleList);
  g_server.on("/dl",   HTTP_GET, handleDownload);
  g_server.on("/del",  HTTP_GET, handleDelete);
  g_server.on("/all",  HTTP_GET, handleAll);

  Serial.println("ready. click = record, hold 3s = sync.");
}

void loop() {
  uint8_t ev = readButton();

  if (ev == 2) {                       // hold -> sync (stops any recording)
    if (g_mode == MODE_SYNC) exitSyncMode();
    else                     enterSyncMode();
    return;
  }

  switch (g_mode) {
    case MODE_IDLE:
      if (ev == 1) startRecording();
#if USE_LIGHT_SLEEP
      else enterIdleSleep();
#endif
      break;

    case MODE_REC:
      if (ev == 1) stopRecording();
      else         pumpAudio();
      break;

    case MODE_SYNC:
      if (ev == 1) exitSyncMode();
      else         serviceSync();
      break;
  }
}
