# Status — working breadboard prototype (18 Oct 2026)

**Click → record → WAV on SD card → hold 3 s → download on phone over WiFi. Verified working.**

## Verified wiring (USB at top, ESP32 straddling the groove, rows 1–8)

ESP32 right side (top→bottom): 5V, G, 3.3, 4, 3, 2, 1, 0
ESP32 left side  (top→bottom): 5, 6, 7, 8, 9, 10, 20, 21

| Part | Pin | ESP32 pin | Row / side |
|---|---|---|---|
| Button | leg 1 / diagonal leg | G / 2 | 2 R / 6 R |
| INMP441 | VDD | 3.3 | 3 R |
| INMP441 | GND, L/R | G | 2 R |
| INMP441 | SD | 3 | 5 R |
| INMP441 | WS | 1 | 7 R |
| INMP441 | SCK | 0 | 8 R |
| SD module | VCC | **5V** (module has its own regulator) | 1 R |
| SD module | GND | G | 2 R |
| SD module | MISO | 4 | 4 R |
| SD module | MOSI | 5 | 1 L |
| SD module | SCK | 6 | 2 L |
| SD module | CS | 7 | 3 L |

Mic sits across the centre groove (SD/VDD/GND one side, SCK/WS/L/R the other) — never in one half, its 2×3 pins would short in pairs.

## Firmware

- `firmware/pendant_recorder/` — main recorder. Stereo I2S read, mic in slot A, 24-bit → DC filter → `/GAIN_DIV` (128). Header patched every 5 s (crash-safe). LED active LOW. `USE_LIGHT_SLEEP 0` for bench work (sleep kills USB serial).
- `firmware/hwtest/` — full hardware self-test (press Enter in Serial Monitor to re-run).
- `firmware/wiretest/` — mic-row continuity probe (mic removed).
- `firmware/micdiag/` — tells real sound from noise (corr ≈ 1 = sound).
- `firmware/ramrec/` — 2.8 s RAM recording served over WiFi, no SD needed.

Measured: normal speech at 20–30 cm → rms ~2000, peak ~7000, 0 % clipping.

## Lessons from bring-up

1. Board was sold **without headers soldered** (BOUGHT.md was wrong) — resting on pins = no contact.
2. Arduino IDE: **Tools → USB CDC On Boot → Enabled**, or nothing prints.
3. Super Mini drops the USB link on reset — sketches now wait for the Serial Monitor.
4. Button: use diagonally opposite legs.
5. Mic all-zeros / noise was always wiring (WS in row 3, SCK in row 7). `wiretest` found it in 2 minutes.
6. A stray test jumper left in a mic row made the board reboot when I2S started.
7. Mic RMS alone can't tell sound from noise; adjacent-sample correlation can.
8. Super Mini WiFi: set `WIFI_POWER_8_5dBm` after `softAP`.
9. 5 s of audio doesn't fit in one RAM block with WiFi on (largest ≈ 114 KB).

## Next

- [ ] Phase 2 power: TP4056 (USB-C, with protection) + LiPo → 5V pin; then re-enable light sleep and measure idle current
- [ ] Re-test that SD module works at LiPo voltage (3.7–4.2 V into its regulator)
- [ ] Solder permanent build, fit in Vicks-jar pendant, mic port facing out + foam windscreen
- [ ] Phone/PC transcription: `tools/transcribe.py`
- [ ] Change `AP_PASS`
