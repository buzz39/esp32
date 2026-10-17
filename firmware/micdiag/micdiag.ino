/*
 * micdiag - finds out WHY the INMP441 gives noise instead of sound.
 *
 * HUM CONTINUOUSLY near the mic while it runs (about 2 seconds).
 * It records twice: SCK/WS as wired, then SCK/WS swapped in software.
 * For each recording it tries bit alignments -2..+2 and measures
 * "smoothness" (corr). Real sound: corr 0.6-1.0. Random noise: ~0.
 *
 * Board: ESP32C3 Dev Module, USB CDC On Boot = Enabled, core 3.x.
 * Press Enter in the Serial Monitor to run again.
 */
#include <Arduino.h>
#include <driver/i2s_std.h>

#define PIN_SCK  0
#define PIN_WS   1
#define PIN_SD   3
#define RATE     16000
#define FRAMES   4000          // 0.25 s of stereo frames

static int32_t *cap = nullptr;  // FRAMES * 2 words (A,B,A,B,...)

static bool capture(int bclk, int ws) {
  i2s_chan_handle_t rx = nullptr;
  i2s_chan_config_t ch = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
  ch.dma_desc_num  = 8;
  ch.dma_frame_num = 256;
  if (i2s_new_channel(&ch, nullptr, &rx) != ESP_OK) return false;

  i2s_std_config_t cfg = {};
  cfg.clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(RATE);
  cfg.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT,
                                                    I2S_SLOT_MODE_STEREO);
  cfg.slot_cfg.slot_mask = I2S_STD_SLOT_BOTH;
  cfg.gpio_cfg.mclk = I2S_GPIO_UNUSED;
  cfg.gpio_cfg.bclk = (gpio_num_t)bclk;
  cfg.gpio_cfg.ws   = (gpio_num_t)ws;
  cfg.gpio_cfg.dout = I2S_GPIO_UNUSED;
  cfg.gpio_cfg.din  = (gpio_num_t)PIN_SD;
  if (i2s_channel_init_std_mode(rx, &cfg) != ESP_OK) { i2s_del_channel(rx); return false; }
  i2s_channel_enable(rx);

  static int32_t warm[512];
  size_t br = 0;
  uint32_t t0 = millis();
  while (millis() - t0 < 300)       // mic start-up time
    i2s_channel_read(rx, warm, sizeof(warm), &br, pdMS_TO_TICKS(100));

  size_t want = (size_t)FRAMES * 2 * 4, got = 0;
  while (got < want) {
    size_t n = 0;
    if (i2s_channel_read(rx, (uint8_t *)cap + got, want - got, &n,
                         pdMS_TO_TICKS(500)) != ESP_OK) break;
    if (n == 0) break;
    got += n;
  }
  i2s_channel_disable(rx);
  i2s_del_channel(rx);
  return got == want;
}

// rebuild slot-A sample i assuming data is k bits early (k>0) or late (k<0)
static int32_t recon(int i, int k) {
  uint32_t a = (uint32_t)cap[2 * i];
  if (k == 0) return (int32_t)a;
  if (k < 0)  return (int32_t)(a << (-k));
  uint32_t low = (uint32_t)cap[2 * i - 1] & ((1u << k) - 1u);   // previous slot B
  return (int32_t)((low << (32 - k)) | (a >> k));
}

static float analyse(const char *label) {
  int32_t bmin = 0, bmax = 0;
  for (int i = 0; i < FRAMES; i++) {
    if (cap[2 * i + 1] < bmin) bmin = cap[2 * i + 1];
    if (cap[2 * i + 1] > bmax) bmax = cap[2 * i + 1];
  }
  Serial.printf("\n[%s]  slot B min/max %ld / %ld\n", label, (long)bmin, (long)bmax);
  Serial.print("  first slot A words: ");
  for (int i = 1; i <= 6; i++) Serial.printf("%08lX ", (unsigned long)(uint32_t)cap[2 * i]);
  Serial.println();

  float best = -2;
  for (int k = -2; k <= 2; k++) {
    double mean = 0;
    int n = 0;
    for (int i = 1; i < FRAMES; i++) { mean += (double)(recon(i, k) >> 8); n++; }
    mean /= n;
    double s2 = 0, cross = 0, prev = 0;
    bool first = true;
    for (int i = 1; i < FRAMES; i++) {
      double d = (double)(recon(i, k) >> 8) - mean;
      s2 += d * d;
      if (!first) cross += d * prev;
      prev = d; first = false;
    }
    float corr = s2 > 0 ? (float)(cross / s2) : 0;
    float rms  = (float)sqrt(s2 / n);
    Serial.printf("  align %+d : rms %10.1f   corr %6.3f %s\n", k, rms, corr,
                  corr > 0.6 ? "<-- looks like SOUND" : "");
    if (corr > best) best = corr;
  }
  return best;
}

static void runDiag() {
  Serial.println(F("\n=========== mic diagnosis ==========="));
  Serial.println(F(">>> START HUMMING NOW, keep humming until it says DONE <<<"));
  delay(1500);

  if (!capture(PIN_SCK, PIN_WS)) { Serial.println(F("capture 1 failed")); return; }
  float b1 = analyse("as wired: SCK=pin0, WS=pin1");
  if (!capture(PIN_WS, PIN_SCK)) { Serial.println(F("capture 2 failed")); return; }
  float b2 = analyse("swapped:  SCK=pin1, WS=pin0");

  Serial.println(F("\n>>> DONE - you can stop humming <<<"));
  if (b1 > 0.6 && b1 >= b2)
    Serial.println(F("VERDICT: wiring OK, sound found as wired (see the align line marked)."));
  else if (b2 > 0.6)
    Serial.println(F("VERDICT: sound found only with SCK/WS SWAPPED - the two wires are crossed."));
  else
    Serial.println(F("VERDICT: no setting looks like sound - physical problem (contact, interference, or mic)."));
  Serial.println(F("Paste everything from '=== mic diagnosis ===' down."));
  Serial.println(F("Press Enter to run again."));
}

void setup() {
  Serial.begin(115200);
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 30000) delay(50);
  delay(800);
  cap = (int32_t *)malloc((size_t)FRAMES * 2 * 4);
  if (!cap) { Serial.println(F("out of memory")); return; }
  runDiag();
}

void loop() {
  if (Serial.available()) {
    while (Serial.available()) Serial.read();
    if (cap) runDiag();
  }
  delay(50);
}
