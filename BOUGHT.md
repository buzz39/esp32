# What we have — inventory after the Sitabuldi trip

Bought (Oct 16). All six items verified correct.

| Item | Verdict | Notes |
|---|---|---|
| ESP32-C3 Super Mini (marked V1601) | ✅ correct board | headers **soldered** — ready to use; USB-C confirmed |
| INMP441 I2S mic (round breakout) | ✅ correct | 6 pins: VDD, GND, L/R, WS, SCK, SD |
| microSD card module, 6 pins | ✅ correct | push-in card slot |
| Solderless breadboard, ~400 point | ✅ correct | bigger than needed, good |
| Jumper wires, M-M, ~40 | ✅ correct | |
| Tactile push buttons, 6 mm, qty 2 | ✅ correct | one is enough |

## Still missing

| Item | Status |
|---|---|
| Soldering iron + solder | needed eventually (Phase 2 soldering), not for the prototype |
| microSD card | check drawers (phone / camera) |
| USB-C **data** cable | check drawers |
| TP4056 Type-C + protection | Phase 2 |
| LiPo cell | Phase 2 — salvage from dead phone |
| Case, lanyard, windscreen | Phase 2, after it records |

## Verified wiring

```
INMP441 (round breakout)      ESP32-C3 Super Mini
  VDD  ------------------------>  3V3
  GND  ------------------------>  GND
  L/R  ------------------------>  GND        <- left channel
  WS   ------------------------>  GPIO1      <- word select
  SCK  ------------------------>  GPIO0      <- bit clock
  SD   ------------------------>  GPIO3      <- data out

microSD module                ESP32-C3 Super Mini
  VCC  ------------------------>  3V3
  GND  ------------------------>  GND
  CS   ------------------------>  GPIO7
  SCK  ------------------------>  GPIO6
  MOSI ------------------------>  GPIO5
  MISO ------------------------>  GPIO4

Button: GPIO2 <-> GND
Onboard LED: GPIO8
```

Do not confuse the mic's **SCK** (bit clock) with the SD module's **SCK** (SPI clock).
Different buses, different pins.

## Order of work

1. Plug the board into the laptop → confirm a COM port appears.
2. Arduino IDE + **esp32 by Espressif** — any recent version, 2.0.x or 3.x both work.
   The sketch detects which one you have and picks the matching I2S driver.
3. Flash `firmware/pendant_recorder/pendant_recorder.ino`.
4. Test: board alone → mic alone → SD alone → everything.
