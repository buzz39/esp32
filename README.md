# NeoClone — ₹500-ish DIY replica of Neo Sapien (Neo 1)

> **Picking this up cold, or on a different machine? Read [`CONTEXT.md`](CONTEXT.md) first.**
> It is the current state of play: what's built, what's verified, what's blocking, and what
> to do next. The rest of these files are reference material.

A pocket pendant. **Click the button, speak your thought, click to stop.** All the
AI (transcription + summary + memory) runs on your phone or PC. The pendant is just
a mic, a button, a battery and a bit of storage.

Real Neo 1 = ₹11,699. This does the same *job* for ~₹540–750.

```
   click -> speak -> click          hold 3s
        [INMP441 mic]              [WiFi hotspot]
             |                          |
        [ESP32-C3] --SPI--> [microSD]   |  phone joins "NeoClone"
             |                          v
        [button] [LED]          http://192.168.4.1  -> download WAVs
                                             |
                                             v
                          phone app: Whisper -> transcript -> summary
```

One button. No display. Phone does every operation.

## What it does / doesn't

| Neo 1 | NeoClone |
|---|---|
| Records conversations hands-free | Records when **you** press the button |
| On-device live transcription | No — transcribe on phone/PC after sync |
| Cloud AI summaries | Yes, free: phone Whisper app or local LLM |
| App + memory search | Yes — transcripts are just searchable text |
| Always-on all day | Not needed: only records while pressed, so it sleeps for weeks |
| 20 g pendant | ~35–45 g, slightly chunky |

Recording only on press is *better* than always-on here: hugely longer battery, no
accidental room recordings, no consent mess.

## Bill of materials (India prices, mid-2026)

| # | Part | Where | Price |
|---|---|---|---|
| 1 | ESP32-C3 Super Mini (USB-C) | Amazon.in / Robu.in | ₹200–280 |
| 2 | INMP441 I2S MEMS mic module | Robu.in / Amazon | ₹80–150 |
| 3 | TP4056 USB-C charging module | Robu.in | ₹25–40 |
| 4 | LiPo 3.7 V 500–1000 mAh | reclaimed, or ₹120–250 | ₹0–250 |
| 5 | microSD module (SPI) | Robu.in | ₹40–60 |
| 6 | microSD card 8–32 GB | reuse an old one | ₹0–200 |
| 7 | Momentary push button, wires, case, lanyard | local | ₹40–80 |

**Cheapest realistic total: ~₹540.** Reuse the battery and card from old junk to
get under ₹500.

> The ₹50 SD module is what makes this reliable. You *can* drop it and use the C3's
> ~4 MB flash instead, but that only holds ~90 s of audio, so a long thought is lost.
> Keep the card.

## Wiring

```
INMP441        ESP32-C3 Super Mini
  VDD   ------> 3V3
  GND   ------> GND
  L/R   ------> GND            (selects LEFT channel)
  WS    ------> GPIO1
  SCK   ------> GPIO0
  SD    ------> GPIO3

microSD (SPI)  ESP32-C3
  VCC   ------> 3V3
  GND   ------> GND
  CS    ------> GPIO7
  SCK   ------> GPIO6
  MOSI  ------> GPIO5
  MISO  ------> GPIO4

Button         GPIO2 ----[ button ]---- GND     (internal pull-up, no resistor)

Status LED     GPIO8  (the onboard LED on most Super Mini boards; if yours has
                       none, add an LED + 330R from GPIO10 to GND)

TP4056
  B+ / B-      <--> LiPo cell
  OUT+ / OUT-  --> ESP32-C3 5V pin & GND   (use OUT pads so you get protection)
```

All pins are `#define`s at the top of `pendant_recorder.ino` — change to match
what you actually wire.

> GPIO2, GPIO8, GPIO9 are ESP32-C3 strapping pins. Don't **hold the button while
> powering on** (release it during boot) and you're fine.

## Button map

| Action | Result |
|---|---|
| Click (from idle) | Start recording. LED solid on. |
| Click (while recording) | Stop and save. LED off. |
| Hold ≥ 3 s (any time) | Stop recording if active, then start the WiFi sync hotspot. LED fast-blinks. |
| Click (while in sync mode) | Leave sync, back to idle. |

Auto-exit from sync after 5 minutes of no downloads.

## Sync to the phone — three ways

### 1. WiFi hotspot (recommended, works on every phone)
Hold the button 3 s. Phone → WiFi settings → join:

- SSID **`NeoClone`**
- password **`neoclone1`**

Then open **`http://192.168.4.1`** in any browser. You get a list of recordings with
**Get** and **Del** buttons, plus a **Get every recording** page. No app to install.
(The ESP32-C3 will warn "no internet" — ignore it, it's a local hotspot.)

### 2. Wire (USB)
Copy the card's WAVs over USB-C with a card reader, or — if you want a cable-only
workflow — the C3 can be flashed as a USB mass-storage device with Adafruit TinyUSB
(`USBMSC`). That makes plugging into the phone show a removable drive. It's an extra
fiddly step and conflicts with the serial monitor, so WiFi is the default. Ask and
I'll write that firmware variant.

### 3. Bluetooth (BLE)
The C3 has **BLE only — no Bluetooth Classic**, so there's no "send file" pairing.
BLE file transfer needs a custom app, and 16 kHz audio streaming over BLE barely
fits. Not worth it when WiFi works in 2 seconds. Skipped on purpose.

## Phone pipeline (all free)

After downloading the WAVs to your phone:

1. **Transcribe** — install an offline on-device Whisper app:
   - **Transcribro** (F-Droid) — pick a WAV, get text.
   - or any "Whisper" app on Play Store that accepts audio files.
2. **Summarise** — paste/ share the transcript into the **Gemini** or **ChatGPT**
   app: *"Summarise this and list action items."*
3. **Memory** — save the notes as text in a folder. Search it with your phone's
   Files search, or throw the folder into any notes app with global search.

### PC pipeline (if you'd rather batch it)
Copy the card to your PC and run the included script — fully offline, no uploads:

```powershell
pip install faster-whisper requests
python tools/transcribe.py D:\REC_*.WAV --out notes\
```

Each recording becomes `notes\REC_0001.md` with a timed transcript plus a summary
(via Ollama if running, else transcript only). Then `grep -ri "that thing I forgot" notes\`.

## Build steps

1. **Run the hardware test first.** Arduino IDE → Boards Manager → install *esp32 by
   Espressif* (2.0.x or 3.x, both work — the sketch detects which and picks the matching
   I2S driver). Board: **ESP32C3 Dev Module**, **USB CDC On Boot = Enabled**.
   Upload `firmware/hwtest/hwtest.ino` and read the Serial Monitor @ 115200.
2. **Flash the firmware.** Open `firmware/pendant_recorder/pendant_recorder.ino`.
3. **Bench test.** Breadboard it, power from USB, Serial Monitor @ 115200.
   Click the button → `recording -> /REC_0001.WAV` → click again → `saved: ... bytes`.
   If you get silence or pure noise, tune `SAMPLE_SHIFT` (try 8, 11, 12) — the INMP441
   packs 24-bit data into a 32-bit slot and clones differ.
3. **Listen.** Play the WAV. Speech should be clear, slightly hissy (no AGC — keep the
   mic port facing out of the case, don't bury it).
4. **Test sync.** Hold the button 3 s, join `NeoClone` from your phone, download a file.
5. **Solder + case.** TP4056 → LiPo → 5V pin. Tape or 3D-print a case with a mic hole,
   the button on top, LED visible. Add a lanyard loop.
6. **Battery sanity.** Charge, then leave it idle overnight and check it's still alive.
   Idle should be weeks, not hours. If it's dead, light sleep didn't engage — see below.

## Battery expectation

| State | Draw | On a 500 mAh cell |
|---|---|---|
| Idle (light sleep) | ~1 mA | ~2–3 weeks |
| Recording | ~50 mA | ~10 h of recorded speech |
| WiFi sync | ~120 mA | ~4 h (only on for minutes) |

Recording is the only thing that costs you, and you only record when you talk.
Tune with `USE_LIGHT_SLEEP` in the firmware.

## Tuning knobs

| Knob | File | Effect |
|---|---|---|
| `SAMPLE_RATE` | .ino | 8000 halves file size, still fine for speech |
| `SAMPLE_SHIFT` | .ino | 8 / 11 / 12 — fixes silence vs. noise |
| `SYNC_HOLD_MS` | .ino | how long to hold for sync mode |
| `AP_SSID` / `AP_PASS` | .ino | hotspot name/password |
| `USE_LIGHT_SLEEP` | .ino | 1 = weeks of standby, 0 = ~1 day but simpler |
| `--model` | transcribe.py | `tiny` fast, `small`/`medium` accurate |

## ⚠️ Legal / privacy — read this

Recording other people is **not** the same as recording yourself. In India, recording
a private conversation you're not a party to can attract IT Act / privacy liability,
and many places require **all-party consent**. Covert room recording is the fastest
route to a serious problem.

Since this build only records while you deliberately press the button and you announce
"let me note this down", it stays on the right side of the line — keep it that way.
Also treat the recordings and transcripts as sensitive: they contain everything you
ever said. Don't sync over an untrusted hotspot, and change `AP_PASS`.

## Firmware status

`pendant_recorder.ino` is a complete starting sketch: push-to-talk recording, WAV on
SD, file rotation, LED status, light sleep, and the WiFi sync server with a phone-friendly
page. It has **not** been compiled or flashed here (no hardware attached) — expect to
fiddle with the shift value and possibly pin numbers on first run.
