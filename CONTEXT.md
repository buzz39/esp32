# CONTEXT — read this first

Handover note. **Last updated: 17 Oct 2026.**

If you are an AI assistant picking this up cold: this file is the state of play. Read it
before touching anything. The other `*.md` files are reference material; this one is the
"here's exactly where we stopped" file.

If you are the human: start at [§9 How to resume](#9-how-to-resume-on-a-fresh-machine).

---

## 1. The project in one paragraph

A cheap DIY replica of the **Neo Sapien "Neo 1"** AI pendant (real one: ₹11,699). A pendant
with **no display and one button**: click to record, click again to stop. It records 16 kHz
mono WAV to a microSD card and hands the files to a phone over a WiFi hotspot. **All AI —
transcription, summary, search — runs on the phone or PC, never on the device.** Target
budget ₹500 (₹700 acceptable). **Software is explicitly deferred** — the current goal is
nothing more than a working breadboard prototype. No enclosure work yet.

## 2. Where we are right now

| | |
|---|---|
| Phase | Breadboard prototype, **hardware bring-up**, Step 4 of 11 |
| Blocking on | **No ESP32 USB device enumerating** on the original machine |
| Also blocking | **Unconfirmed breadboard seating** (pin direction) |
| Next action | Get a COM port, flash `firmware/hwtest/hwtest.ino`, read Serial @ 115200 |

Done: all parts bought and verified, both sketches written, both **compiled successfully**
against a real toolchain (see §5). Nothing has run on hardware yet.

Not done: no upload, no serial output, no wiring.

## 3. Hardware in hand

Bought at **Sitabuldi, Nagpur** (Oct 16). All six verified correct by photograph.

| Item | State |
|---|---|
| ESP32-C3 Super Mini (marked `V1601`) | headers **already soldered**, USB-C confirmed |
| INMP441 I2S mic (round breakout, 6 pins) | **digital I2S confirmed** — the 6 pins (VDD/GND/L/R/WS/SCK/SD) settle it |
| microSD module (6 pins, SPI) | correct |
| Solderless breadboard, ~400 point, in **two halves** | correct |
| Jumper wires, M-M, ~40 | correct |
| Tactile push buttons, 6 mm, qty 2 | correct, one is enough |

**Not yet owned:** microSD card, USB-C **data** cable, soldering iron, TP4056, LiPo cell,
case/lanyard/windscreen.

> Superseded: an earlier note worried the INMP441 might be an *analog* mic needing an
> amplifier. Wrong — the 6-pin breakout proves it's I2S digital. No MAX9814 needed.

## 4. Verified wiring

This is the map both sketches use. It is not yet physically wired.

```
INMP441 (round breakout)      ESP32-C3 Super Mini
  VDD  ---------------------->  3V3
  GND  ---------------------->  GND
  L/R  ---------------------->  GND      <- grounds it to LEFT channel
  WS   ---------------------->  GPIO1    <- word select
  SCK  ---------------------->  GPIO0    <- bit clock
  SD   ---------------------->  GPIO3    <- data out

microSD module (SPI)          ESP32-C3 Super Mini
  VCC  ---------------------->  3V3
  GND  ---------------------->  GND
  CS   ---------------------->  GPIO7
  SCK  ---------------------->  GPIO6
  MOSI ---------------------->  GPIO5
  MISO ---------------------->  GPIO4

Button                      GPIO2 ----[ button ]---- GND   (internal pull-up, no resistor)
Status LED                  GPIO8   (onboard, ACTIVE LOW — LOW = lit)
```

**Trap:** the mic and the SD module *both* have a pin called `SCK`. Different buses,
different ESP32 pins. Mic SCK → GPIO0. Card SCK → GPIO6.

ESP32-C3 Super Mini exposes: 3V3, 5V, GND, GPIO 0–10, 20, 21.

## 5. Firmware

```
firmware/hwtest/hwtest.ino                  <- upload THIS first
firmware/pendant_recorder/pendant_recorder.ino
```

`hwtest` is a 5-step diagnostic: ① chip ② LED on GPIO8 ③ button on GPIO2 ④ INMP441 mic
⑤ microSD. It prints **RMS at three shift values (8, 11, 12)** — one of them is the correct
`SAMPLE_SHIFT`, which is a one-line change in the real firmware.

Both sketches support **esp32 core 2.0.x AND 3.x** via a compile-time guard:

```c
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  #include <driver/i2s_std.h>     // 3.x
#else
  #include <driver/i2s.h>         // 2.x legacy
#endif
```

A `micInit/micStart/micStop/micFree/micRead` wrapper hides the API difference.
Do not "simplify" this away — it's what lets any recent core build the code.

### Verified build results (real toolchain, not a guess)

arduino-cli 1.5.1 + esp32 core **3.3.12**, FQBN `esp32:esp32:esp32c3`:

| Sketch | Flash | RAM |
|---|---|---|
| `hwtest` | 388,646 B (29%) | 15,616 B (4%) |
| `pendant_recorder` | 1,097,939 B (83%) | 39,768 B (12%) |

Both exit 0, no errors. The recorder is at **83% flash** — it fits, but there isn't much
room left, so be careful adding libraries to it.

Compile command that works:

```powershell
arduino-cli compile --fqbn esp32:esp32:esp32c3 <sketchdir>
```

## 6. The two blockers

### 6a. No ESP32 USB device enumerates  ⚠️ most important

On the original machine, with the board plugged in and the red LED lit, **no ESP32 device
appeared at all**. The only COM ports present were COM3–COM6, all *Microsoft Bluetooth*
ports. Nothing at `VID_303A` (Espressif), no CP210x/CH34x/FTDI, no "USB JTAG/serial debug
unit".

**The red LED proves power only. It does not prove the data lines work.** A charge-only
USB-C cable powers the board and lights the LED while carrying zero data — which looks
exactly like a working board.

Check, in this order:

1. Plug in and watch for a **NEW** COM port within ~10 s. Ignore anything saying Bluetooth.
2. **Swap the USB-C cable** — a known-good *data* cable. This is the #1 cause by a wide margin.
3. Use a different USB port, and plug **straight into the laptop**, not through a dock/hub.
4. Still nothing? Hold **BOOT** while plugging in to force the ROM bootloader.

Live monitor (PowerShell):

```powershell
while ($true) {
  Get-PnpDevice -Class Ports -Status OK | Select-Object -ExpandProperty FriendlyName
  "-----"
  Start-Sleep 2
}
```

In the Arduino IDE, the reliable trick is: open **Tools → Port** with the board unplugged,
then plug it in — *the number that appears is the board.*

### 6b. Breadboard seating / pin direction — unconfirmed

A photo appeared to show the header **pin tips pointing up**, on the *same* face as the
ESP32 chip and buttons. If so, the board is **resting on** the breadboard, not plugged in.

**The rule: components up, pin tips pointing DOWN into the holes.** When correctly seated:

- you see the chip, buttons and USB-C, plus two black plastic strips lying flat at the edges
- you do **not** see any gold pin tips from above
- the board stands ~8–10 mm proud of the breadboard

**One sentence: the long pins must come out of the opposite face from the ESP32 chip.**

Not urgent — seating affects nothing until Step 5, and the board runs off USB either way.
But before changing anything, check whether the **pin labels are printed on both faces**.
If labels are only on the component side, flipping the board hides them and you'd be wiring
blind. In that case the headers should be moved properly, not flipped.

**Settled by Step 5:** if the button test reports `PASS - button detected`, the board is
definitely connected.

### 6c. Breadboard half-orientation trap

The user's breadboard is **two separate halves**. The 5-hole connected groups run
**horizontally** (same row number, letters `a`–`e` on the left half, `f`–`j` on the right).
Because the ESP32's two pin rows run vertically, each pin lands in a different row number
→ its own group. That is correct.

**Requirement: the two pin rows must be in different halves** (one in `a`–`e`, one in
`f`–`j`), or two pins can share a group and short.

## 7. Machine gotchas

- **`USB CDC On Boot` MUST be `Enabled`** in Tools, or Serial output is completely
  invisible on this native-USB board. Not optional.
- **GPIO2, GPIO8, GPIO9 are strapping pins.** Don't hold the button during power-on.
- **Application Control policy blocks `esptool.exe`** on the original machine. Symptom:
  `Failed to load Python DLL ...\_MEI...\python313.dll — An Application Control policy has
  blocked this file.` The policy applies **only to `%LOCALAPPDATA%\Temp`**. Fix — run the
  IDE with the temp dir redirected anywhere else:

  ```powershell
  $env:TEMP = "C:\Users\<you>\arduino-tmp"; $env:TMP = $env:TEMP
  & "C:\path\to\Arduino IDE.exe"
  ```

  Verified working with esptool 5.3.1. Only relevant if the machine is managed/corporate.
- **Upload fails with "Failed to connect"?** Hold **BOOT**, tap **RESET**, release **BOOT**,
  upload again.
- **Port number changes** between resets — the C3 re-enumerates. Re-select it if an upload
  fails mid-way.
- **GitHub repo:** <https://github.com/buzz39/esp32>. `real/` (manufacturer product photos,
  copyright) and `market/` (personal photos) are gitignored and deliberately **not** pushed.

## 8. Open questions — nobody has answered these yet

1. **Are the INMP441's pins soldered down?** If loose, the mic can't go in the breadboard
   and needs an iron or a shop. *(Asked, never answered. Step 6 is blocked on it.)*
2. **Do you own a soldering iron?** *(Asked, never answered.)*
3. **Do you have a microSD card?** *(Asked, never answered. Step 7 is blocked on it.)*
4. **Which way do the header pins point?** See §6b.
5. **Is the USB-C cable a data cable?** See §6a.
6. **Battery cell dimensions/mAh** — needed before any enclosure work.

## 9. How to resume on a fresh machine

```powershell
git clone https://github.com/buzz39/esp32.git
cd esp32
```

Then:

1. **Install the Arduino IDE** (arduino.cc/en/software). Accept driver prompts.
2. **Add ESP32 support:** File → Preferences → *Additional boards manager URLs* →
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`, then
   Tools → Board → Boards Manager → search `esp32` → install **esp32 by Espressif Systems**
   (~400 MB, several minutes). Any recent version; 2.0.x and 3.x both build this code.
3. **Settings:** Board → **ESP32C3 Dev Module**; **USB CDC On Boot → Enabled**;
   Port → the ESP32's port (§6a).
4. **Open** `firmware\hwtest\hwtest.ino` → **Upload**.
5. **Serial Monitor @ 115200**, then press **RESET** on the board.
   Pressing RESET matters — open the monitor late and you only see the quiet heartbeat.
6. **Send the whole Serial output back.** Expect something like:

```
--- 1. chip ---
arduino core : 3.x.x
model        : ESP32-C3
--- 2. LED on GPIO8 ---
blinking 6 times - watch the board
--- 3. button on GPIO2 ---
--- 4. INMP441 microphone ---
--- 5. microSD card ---
```

**At this stage button, mic and SD should all FAIL** — nothing is wired. That's correct.
Only the chip section and the LED blink need to pass.

Then work through `GUIDE.md` Steps 5 → 11. The milestone is **Step 9**: a real WAV on the
card. Step 10 (WiFi sync) and Step 11 (transcribe) are the payoff.

## 10. Known firmware gaps (do not forget)

- **WAV header is only written on stop.** A power failure mid-recording yields an
  unplayable file. Needs 5-minute rotation with periodic header re-patching. **Not yet built.**
- `SAMPLE_SHIFT` is still a placeholder until Step 6 produces the RMS numbers.
- `AP_PASS` should be changed from the default before real use.
- `USE_LIGHT_SLEEP` — 1 = weeks of standby, 0 = ~1 day but simpler to debug.

## 11. Reference

**Real Neo 1:** ~45 × 32 mm, ~8–10 mm thick, black rounded rectangle on a ball chain, worn
outside clothing, one button at top, phone app does all the AI, ₹11,699.

**Decisions already made — don't relitigate without reason:**

| Decision | Why |
|---|---|
| ESP32-C3 Super Mini | cheapest WiFi + I2S, ₹199 at Robo Bazar Nagpur |
| INMP441 digital mic | digital avoids ADC noise entirely |
| microSD module required | C3's 4 MB flash holds only ~90 s of audio |
| WiFi SoftAP for sync | C3 has **BLE only, no Bluetooth Classic** |
| 16 kHz mono 16-bit WAV | 32 KB/s; 45 min ≈ 86 MB |
| Push-to-talk + light sleep | weeks of standby instead of ~1 day |
| No enclosure yet | "we need a working prototype, don't worry about the box" |

**Recording-format arithmetic:** 16 kHz × 2 bytes = 32 KB/s → 45 min ≈ 86 MB. Any 8 GB+
card is plenty.

**Nagpur sourcing:** Robo Bazar (robobazar.in, 1-hour delivery in Nagpur urban) was the
primary; Makestore (107, Baji Prabhu Nagar, near Ram Nagar, 440033) is the backup. Sitabuldi
walk-in dealers: Sharma Robot Kits, Dangi E Shop, Patel Sales Corporation, Planmystudy.
See `SOURCING-NAGPUR.md`.

## 12. ⚠️ Legal / privacy

Recording other people is not the same as recording yourself. In India, recording a private
conversation you are not a party to can attract **IT Act / privacy liability**, and many
places require **all-party consent**. Push-to-talk plus saying "let me note this down" keeps
this on the right side of the line — **keep it that way.** Also: treat recordings and
transcripts as sensitive; they contain everything ever said. Change `AP_PASS`, and never
brand or sell this as Neo Sapien.
