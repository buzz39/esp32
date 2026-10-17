/*
 * ramrec - record 5 s from the INMP441 into RAM and play it on your phone.
 * No SD card needed.
 *
 * 1. Upload. The board starts a WiFi hotspot:
 *      name: NeoClone-test     password: neoclone1
 * 2. Phone: join that WiFi (ignore "no internet"), open  http://192.168.4.1
 * 3. Press the breadboard button (or tap "Record" on the page), talk for 5 s
 *    from about 20-30 cm away. LED is on while recording.
 * 4. Refresh the page and press play.
 *
 * Board: ESP32C3 Dev Module, USB CDC On Boot = Enabled, core 3.x.
 */
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <driver/i2s_std.h>
#include <esp_heap_caps.h>

#define PIN_SCK   0
#define PIN_WS    1
#define PIN_SD    3
#define PIN_BTN   2
#define PIN_LED   8          // active LOW on the Super Mini

#define RATE      16000
#define SECONDS   5
#define MAXSAMP   (RATE * SECONDS)
#define GAIN_DIV  128.0f     // 24-bit -> 16-bit is /256; /128 = 2x louder

#define AP_SSID   "NeoClone-test"
#define AP_PASS   "neoclone1"

static int16_t *audio = nullptr;
static size_t recorded = 0;
static size_t nsamp = 0;           // how many samples fit in RAM
static i2s_chan_handle_t rx = nullptr;
static String lastInfo = "nothing recorded yet";
WebServer server(80);

static void led(bool on) { digitalWrite(PIN_LED, on ? LOW : HIGH); }

static bool micInit() {
  i2s_chan_config_t ch = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
  ch.dma_desc_num  = 8;
  ch.dma_frame_num = 256;
  if (i2s_new_channel(&ch, nullptr, &rx) != ESP_OK) return false;
  i2s_std_config_t cfg = {};
  cfg.clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(RATE);
  cfg.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT,
                                                    I2S_SLOT_MODE_STEREO);
  cfg.slot_cfg.slot_mask = I2S_STD_SLOT_BOTH;   // mic is in slot A (L/R = GND)
  cfg.gpio_cfg.mclk = I2S_GPIO_UNUSED;
  cfg.gpio_cfg.bclk = (gpio_num_t)PIN_SCK;
  cfg.gpio_cfg.ws   = (gpio_num_t)PIN_WS;
  cfg.gpio_cfg.dout = I2S_GPIO_UNUSED;
  cfg.gpio_cfg.din  = (gpio_num_t)PIN_SD;
  if (i2s_channel_init_std_mode(rx, &cfg) != ESP_OK) return false;
  return i2s_channel_enable(rx) == ESP_OK;
}

static void recordClip() {
  led(true);
  Serial.printf("recording %.1f s - talk now...\n", (float)nsamp / RATE);
  static int32_t buf[512];                  // 256 stereo frames
  size_t br = 0;
  for (int i = 0; i < 6; i++)               // drop stale buffers
    i2s_channel_read(rx, buf, sizeof(buf), &br, pdMS_TO_TICKS(100));

  float px = 0, py = 0;                     // DC-removal filter state
  size_t n = 0;
  double sq = 0;
  int peak = 0, clipped = 0;
  while (n < nsamp) {
    if (i2s_channel_read(rx, buf, sizeof(buf), &br, pdMS_TO_TICKS(300)) != ESP_OK || br == 0) break;
    int frames = br / 8;
    for (int i = 0; i < frames && n < nsamp; i++) {
      float x = (float)(buf[2 * i] >> 8);   // slot A, 24-bit sample
      float y = x - px + 0.995f * py;       // remove DC / slow drift
      px = x; py = y;
      float o = y / GAIN_DIV;
      if (o > 32767)  { o = 32767;  clipped++; }
      if (o < -32768) { o = -32768; clipped++; }
      int16_t s = (int16_t)o;
      audio[n++] = s;
      sq += (double)s * s;
      if (abs(s) > peak) peak = abs(s);
    }
  }
  recorded = n;
  led(false);
  float rms = n ? sqrt(sq / n) : 0;
  lastInfo = String(n) + " samples, rms " + String(rms, 0) + ", peak " + String(peak) +
             ", clipped " + String(n ? 100.0f * clipped / n : 0, 1) + "%";
  Serial.println("done: " + lastInfo);
}

static void handleRoot() {
  String h = F("<!doctype html><meta name=viewport content='width=device-width'>"
               "<body style='font-family:sans-serif;padding:16px'>"
               "<h2>NeoClone mic test</h2>");
  h += "<p>" + lastInfo + "</p>";
  h += "<p><a href='/record' style='font-size:20px'>&#9679; Record " + String((float)nsamp / RATE, 1) + " s</a></p>";
  if (recorded) h += F("<audio controls src='/rec.wav'></audio>"
                       "<p><a href='/rec.wav' download='test.wav'>Download test.wav</a></p>");
  h += F("<p style='color:#888'>Or press the breadboard button, then refresh.</p></body>");
  server.send(200, "text/html", h);
}

static void handleRecord() {
  recordClip();
  server.sendHeader("Location", "/");
  server.send(303);
}

static void put32(uint8_t *p, uint32_t v) { p[0]=v; p[1]=v>>8; p[2]=v>>16; p[3]=v>>24; }
static void put16(uint8_t *p, uint16_t v) { p[0]=v; p[1]=v>>8; }

static void handleWav() {
  if (!recorded) { server.send(404, "text/plain", "record first"); return; }
  uint32_t dataBytes = recorded * 2;
  uint8_t hdr[44];
  memcpy(hdr, "RIFF", 4);      put32(hdr + 4, 36 + dataBytes);
  memcpy(hdr + 8, "WAVEfmt ", 8); put32(hdr + 16, 16);
  put16(hdr + 20, 1);          put16(hdr + 22, 1);            // PCM, mono
  put32(hdr + 24, RATE);       put32(hdr + 28, RATE * 2);
  put16(hdr + 32, 2);          put16(hdr + 34, 16);
  memcpy(hdr + 36, "data", 4); put32(hdr + 40, dataBytes);

  server.setContentLength(44 + dataBytes);
  server.send(200, "audio/wav", "");
  server.sendContent((const char *)hdr, 44);
  const char *p = (const char *)audio;
  for (uint32_t off = 0; off < dataBytes; off += 4096) {
    uint32_t len = min((uint32_t)4096, dataBytes - off);
    server.sendContent(p + off, len);
  }
}

void setup() {
  Serial.begin(115200);
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 5000) delay(50);   // don't block if no PC
  pinMode(PIN_LED, OUTPUT); led(false);
  pinMode(PIN_BTN, INPUT_PULLUP);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);   // Super Mini boards often need this to be visible
  if (!micInit()) { Serial.println(F("mic init failed")); return; }

  // grab the biggest free block left, keep 24 KB spare for WiFi/web traffic
  size_t largest = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
  size_t want = largest > 24 * 1024 ? largest - 24 * 1024 : 0;
  nsamp = min((size_t)MAXSAMP, want / sizeof(int16_t));
  audio = nsamp ? (int16_t *)malloc(nsamp * sizeof(int16_t)) : nullptr;
  Serial.printf("free heap %u, largest block %u -> %.1f s of audio\n",
                (unsigned)ESP.getFreeHeap(), (unsigned)largest, (float)nsamp / RATE);
  if (!audio || nsamp < RATE) { Serial.println(F("out of memory")); return; }

  server.on("/", handleRoot);
  server.on("/record", handleRecord);
  server.on("/rec.wav", handleWav);
  server.begin();

  Serial.println(F("\n=== ramrec ready ==="));
  Serial.println(F("Phone: join WiFi 'NeoClone-test' (password neoclone1)"));
  Serial.println(F("Then open http://192.168.4.1"));
  Serial.println(F("Press the breadboard button to record 5 s."));
}

void loop() {
  server.handleClient();
  if (digitalRead(PIN_BTN) == LOW) {
    delay(30);
    if (digitalRead(PIN_BTN) == LOW) {
      recordClip();
      while (digitalRead(PIN_BTN) == LOW) delay(10);
    }
  }
}
