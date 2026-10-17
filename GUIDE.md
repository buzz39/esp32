# Build guide — from parts on the table to your first recording

Follow this in order. Each step says what to do, what you should see, and what to do if
it fails. Don't skip ahead — the whole point is to have one thing working before adding
the next, so when something breaks you know exactly what caused it.

---

## Where you are

**Done:**
- ESP32-C3 Super Mini bought, headers soldered, USB-C confirmed
- Board plugs into the laptop and shows a COM port
- LED lights up
- INMP441 mic, microSD module, breadboard, jumper wires, buttons — all bought

**Not yet known:**
- Is the board sitting in the breadboard correctly (see Step 3)
- Are the mic's pins soldered
- Do you have a microSD card
- Do you have a soldering iron

---

## STEP 1 — Install the Arduino IDE

1. Go to **arduino.cc/en/software**
2. Download the **Windows** version (the "Win 10 and newer, 64 bits" one)
3. Install it — accept the default options, including any driver prompts
4. Open it. A window opens with a blank sketch

**If Windows asks about installing drivers for a "USB serial device", say yes.**
That's the ESP32 talking.

---

## STEP 2 — Add ESP32 board support

The IDE doesn't know what an ESP32 is until you add it.

1. **File → Preferences**
2. Find the box labelled **"Additional boards manager URLs"**
3. Paste this in it:
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
4. Click **OK**
5. Now **Tools → Board → Boards Manager**
6. In the search box type `esp32`
7. Find **"esp32 by Espressif Systems"** and click **INSTALL**

This downloads about 400 MB and takes several minutes. Let it finish.
Any recent version is fine — 2.0.x and 3.x both build our code.

**Set the board:**
- **Tools → Board → esp32 → ESP32C3 Dev Module**
- **Tools → USB CDC On Boot → Enabled** ← do not skip this
- **Tools → Port →** your COM port (the one that appeared when you plugged the board in)

Everything else in the Tools menu can stay default.

---

## STEP 3 — Check the board is sitting in the breadboard correctly

This is the step beginners get wrong, and it causes baffling failures.

A breadboard's holes are connected in **groups of five**. Each group of five runs from the
outer edge of the board to the long groove down the middle. The two halves are separate.

**The rule:** the ESP32's two rows of pins must end up in **different halves** — one row
above the groove, one row below. Then every pin gets its own group of five and nothing is
shorted.

**Check it:** look at the printed `a b c d e` and `f g h i j` on the breadboard.
- If one row of ESP32 pins is in the a–e half and the other row is in the f–j half → **good**
- If both rows are in a–e → **bad**, they're shorted. Slide the board so it straddles the groove

Also confirm: **no two of the 16 pins land in the same group of five.**

If unsure, take a top-down photo and send it to me.

---

## STEP 4 — Upload the test sketch, with nothing wired yet

Testing with nothing connected sounds pointless. It isn't — it proves the whole toolchain
works, so any later failure is definitely wiring.

1. In the IDE: **File → Open**
2. Go to `neosapien-clone\firmware\hwtest\` and open **hwtest.ino**
3. Click **Upload** (the → arrow button, top-left)
4. Wait for "Done uploading"

**Then, in order:**
5. Click the **Serial Monitor** icon (magnifier, top-right)
6. Set the speed dropdown (bottom-right of the monitor) to **115200**
7. Press the **RESET** button on the ESP32 board

Pressing RESET matters: opening the Serial Monitor restarts the board, and if you open the
monitor late you'll only see the quiet heartbeat and miss the whole report.

**What you should see:**
```
--- 1. chip ---
arduino core : 3.x.x
model        : ESP32-C3
...
--- 2. LED on GPIO8 ---
blinking 6 times - watch the board
--- 3. button on GPIO2 ---
press and hold the button for the next 6 seconds...
--- 4. INMP441 microphone ---
...
--- 5. microSD card ---
...
```

Expect the **button, mic and SD tests to fail** at this point. That's correct — nothing is
wired. What matters is that the board reports itself and the LED blinks.

**If the upload fails** with "Failed to connect": hold the **BOOT** button, tap **RESET**,
release **BOOT**, then click Upload again.

**If the Serial Monitor is empty:** check `Tools → USB CDC On Boot` is **Enabled**, then
press RESET again.

**Copy the whole output and send it to me.**

---

## STEP 5 — Wire the button

Start with the easy one.

1. Push the tactile button into the breadboard, straddling the centre groove. Its four
   legs now occupy four separate groups of five.
2. Jumper wire from one leg → the pin labelled **2** on the ESP32
3. Jumper wire from a leg on the **other side** of the button → any **GND** pin on the ESP32

The button has two pairs of internally-connected legs. If one pair doesn't work, use a leg
from the other pair.

**Re-run the test** (press RESET, watch the monitor). Step 3 should now say
`PASS - button detected`, and it should ask you to press it — do that.

---

## STEP 6 — Wire the microphone

First answer: **are the mic's pins soldered down?** If they're loose, stop here — the mic
needs an iron or a shop before it can go in.

If they're soldered, the mic has six pins:

```
INMP441 pin   →   ESP32-C3 pin
  VDD           →   3V3
  GND           →   GND
  L/R           →   GND          <- must be grounded, this picks the left channel
  WS            →   1
  SCK           →   0
  SD            →   3
```

**Note the trap:** the mic has a pin called `SCK` and the SD card module also has a pin
called `SCK`. They are **not the same signal** and go to **different ESP32 pins**. The
mic's SCK goes to GPIO0. The card's SCK goes to GPIO6.

If the round mic board's pins don't line up with the breadboard holes, **don't force them** —
tell me and send a photo. Bending them can crack the pad.

**Re-run the test.** You should now get real numbers, something like:

```
samples      : 64000
raw min/max  : -12345 / 9876
RMS  shift 8 : 812.4
RMS  shift 11: 12.7
RMS  shift 12: 1.6
```

**Send me those three RMS lines.** One of them is the correct one and I'll tell you which;
it becomes a one-line change in the real firmware. Room noise should read roughly 50–2000,
and humming into the mic should push it past 10000.

If it says *all zeros*, the mic isn't getting data — check the six wires, especially `L/R`
to GND.

---

## STEP 7 — Wire the microSD card

```
card pin   →   ESP32-C3 pin
  VCC       →   3V3
  GND       →   GND
  CS        →   7
  SCK       →   6
  MOSI      →   5
  MISO      →   4
```

Insert the card **before** running the test. The card must be **FAT32**. Cards 32 GB and
under are FAT32 out of the box; 64 GB and up arrive as exFAT and must be reformatted on a
PC as FAT32.

**Re-run the test.** You want to see the card size, the folder listing, and:

```
PASS - card is readable and writable
```

At this point **all five tests pass.** That's the hardware done. Send me the full output.

---

## STEP 8 — Upload the real firmware

1. **File → Open** → `neosapien-clone\firmware\pendant_recorder\pendant_recorder.ino`
2. Set `SAMPLE_SHIFT` to whichever value I told you from Step 6
3. Set a new `AP_PASS` while you're there — pick your own WiFi password
4. **Upload**

Serial Monitor should print something like:
```
SD ok, 8192 MB
ready. click = record, hold 3s = sync.
```

---

## STEP 9 — Record your first memo

- **Click** the button → LED comes on, `recording -> /REC_0001.WAV`
- Speak
- **Click** again → LED goes off, `saved: 320000 bytes (~10 s)`

That file is now on the SD card as a normal WAV. That's the milestone — you have a
working voice recorder.

**Watch out:** don't hold the button down while powering the board on. GPIO2 is a
"strapping" pin, and holding it low at boot makes the board start in the wrong mode.

---

## STEP 10 — Get the file to your phone

1. **Hold the button for 3 seconds** → LED starts blinking fast
2. On your phone: **Settings → WiFi** → join the network **`NeoClone`**, password whatever
   you set in Step 8
3. Your phone will warn "no internet" — that's expected, allow it / choose "keep
   connecting"
4. Open a browser → go to **`http://192.168.4.1`**
5. You'll see a list of recordings with **Get** and **Del** buttons
6. Tap **Get** on a recording
7. Tap the button once to leave sync mode

---

## STEP 11 — Transcribe

The recording is a plain WAV at 16 kHz mono — every transcription tool accepts it. Either:

- **On the phone:** share the WAV to any transcription app
- **On the PC:** there's a script at `neosapien-clone\tools\transcribe.py` that uses
  `faster-whisper` to produce a text transcript plus a summary. It needs Python; ask me
  and I'll walk you through it

---

## Then, and only then, Phase 2

Once you've recorded and transferred a real memo, the remaining work is mechanical:

1. Add the **TP4056 + LiPo cell** so it runs without the laptop
2. Solder it compact instead of breadboard
3. Put it in a case (Tic Tac box or 3D print)

Don't start any of that until Step 10 works. A wearable that doesn't record is just a
smaller broken recorder.

---

## Honest expectations

- The mic will probably **not** work on the first try. Almost nobody's does. It's a
  one-line fix, not a dead part.
- The SD card module is the **most common** source of trouble — usually the card format,
  not the wiring.
- If a wire is in the wrong hole, the symptom is usually "nothing happens at all", not
  something dramatic. No damage, just re-seat it.
