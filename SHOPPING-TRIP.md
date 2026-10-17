# Sitabuldi shopping trip — take this with you

Open this on your phone. Ask for things by name; show the shopkeeper this screen if needed.

---

## A. Buy these — the core

| # | Ask for exactly this | Qty | Don't pay more than |
|---|---|---|---|
| 1 | **"ESP32 C3 Super Mini board, USB-C, chhota board"** | 1–2 | ₹250 each |
| 2 | **"INMP441 I2S microphone module"** | 1 | ₹180 |
| 3 | **"micro SD card module, SPI / TF card module"** | 1 | ₹80 |
| 4 | **"Breadboard, small — 170 point"** | 1 | ₹80 |
| 5 | **"Jumper wires, male-to-male, 40 piece set"** | 1 set | ₹100 |
| 6 | **"Tactile push button 6 mm"** (pack) | 1 pack | ₹30 |

**Target: ₹400–500 total.**

## B. If something isn't available — fallbacks

**ESP32 board:** if no Super Mini, **any ESP32 board is fine** — "ESP32 DevKit V1, 30 pin" or
"ESP32 WROOM board". It's bigger and the pin numbers change, but I'll fix the code. Just
**do not** accept an **ESP8266** (NodeMCU) — it can't do this properly.

**INMP441:** if absent, ask for **"SPH0645 I2S microphone"** or **"ICS-43434 I2S mic"**.
Either is a digital I2S mic and workable. **Refuse analog mics** — MAX9814, MAX4466,
"condenser mic module". They need a different circuit and sound worse.

**micro SD module:** if absent, ask for "SD card breakout board". Must have **6 pins**.

## C. Optional — only if cheap, only if you're comfortable

| Item | Don't pay more than | Note |
|---|---|---|
| Soldering iron, 25 W or 60 W | ₹350 | **Not needed for the prototype.** Buy later, once it works |
| Solder wire (thin, 0.8 mm) + flux | ₹80 | only with the iron |
| Multimeter, cheap digital | ₹350 | useful, not essential |
| Single-strand hookup wire, small roll | ₹40 | neater than jumper wires later |
| **TP4056 Type-C with protection** | ₹60 | Phase 2 only |
| 3.7 V Li-ion cell | ₹250 | Phase 2 only — **do not carry a loose cell around unprotected** |

## D. At the counter — check before you pay

1. **The board is USB-C** (not micro-USB). If micro-USB, you'll need a matching data cable.
2. **Count the pins.** Super Mini has pins along two edges, about 11 usable. If it's a
   big 30-pin board, that's fine — just tell me.
3. **The mic module says INMP441 / SPH0645** on the board silk-screen, and has 6 pins:
   VDD, GND, L/R (or WS), SCK, SD (or DATA), WS.
4. **The SD module has 6 pins** and a card slot.
5. **Ask: "headers soldered hai?"** — if yes, great, it's ready to plug into the breadboard.
   If no, you'll need the soldering iron or ask them to solder it (₹50).
6. **Ask them to test the board**: "Ekdum chal raha hai na? USB pe power do." Watch for a
   power LED.
7. **Send me a photo of the board before you pay.** I'll confirm it's the right one.

## E. While you're there, also ask

- "**ESP32 board aur INMP441 solder kar denge?**" — most hobby shops will solder headers
  and the mic for ₹50–100. **That removes the hardest part of the build.** Strongly worth it.
- "**USB-C data cable hai?**" — a spare is handy. Must be a **data** cable, not charge-only.
- "**Micro SD card 8GB ya 16GB?**" — only if you don't have one at home. Must be FAT32
  (cards over 32 GB arrive as exFAT and need reformatting on a PC).

## F. What NOT to buy today

- A case, box, lanyard, foam — you said the box can wait. You can't size it until the
  electronics work.
- A battery, charging board — Phase 2, and you should salvage a cell first.
- A soldering iron, unless it's cheap and you're keen. The prototype needs none.
- Any "Arduino kit" or "IoT kit" — you'll pay for 40 parts you don't need.

## G. Budget

| | |
|---|---|
| Core, if you buy the board at Robo Bazar instead | ₹199 + ₹129 = ₹328 |
| Core, all from Sitabuldi | ₹400–500 |
| With multimeter | +₹350 |
| With soldering iron + solder | +₹430 |

Bring **₹600–700 cash and UPI** to be comfortable. Ask about a combined-purchase discount
when you're buying five or six things at one shop — Sitabuldi shops usually round down.

---

## When you get home — the first hour

1. **Plug the board into your laptop with the USB cable.**
   Windows should show a new COM port. If nothing appears, you need a **CP2102 or CH340
   driver** — free download, and I'll tell you which one once I know your board.
2. **Install the Arduino IDE**, then in Boards Manager install **"esp32 by Espressif"** —
   any recent version; 2.0.x and 3.x both build the firmware.
3. **Load the test sketch first** — `neosapien-clone\firmware\hwtest\hwtest.ino` — then the
   real firmware at `neosapien-clone\firmware\pendant_recorder\pendant_recorder.ino`.
4. **Test in this order, one piece at a time.** Don't wire everything at once:
   - Board alone → does the built-in LED blink?
   - Wire **only the mic** → do you get sound?
   - Wire **only the SD card** → does it list the card?
   - Then both together.
5. **When it works, tell me.** Then we move to Phase 2: soldering it small and adding a
   battery.

## The one thing most likely to go wrong

The INMP441 gives **silence or pure noise** on the first try. That's a well-known issue —
the 24-bit data sits in a 32-bit slot and the correct bit-shift varies between board
clones. The fix is one line in the code (`SAMPLE_SHIFT`, try 8, 11, 12). Don't panic and
don't assume the mic is dead.
