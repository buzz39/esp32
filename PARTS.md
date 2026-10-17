# Master parts list

Everything needed, with quantities, prices and — for each item — whether you buy it or
pull it out of a drawer. Prices are Nagpur, checked recently.

---

## A. The absolute minimum — a working prototype, today

This is the whole shopping list for something that records and hands files to your phone.

| # | Part | Qty | Spec | Buy or home | Price |
|---|---|---|---|---|---|
| 1 | **ESP32-C3 Super Mini** | 1 | USB-C, ~22.5 × 18 mm | **BUY** — Robo Bazar | ₹199 |
| 2 | **INMP441 mic module** | 1 | I2S, 6 pins | **BUY** — Robo Bazar | ₹129 |
| 3 | **microSD card module** | 1 | SPI, 6 pins, 3.3 V | BUY — Sitabuldi | ₹45 |
| 4 | **microSD card** | 1 | any size, **FAT32** | HOME / mobile shop | ₹0–250 |
| 5 | **Mini breadboard** | 1 | 170-point (or full size) | BUY — Sitabuldi | ₹60 |
| 6 | **Jumper wires** | ~20 | male-to-male, 10 cm | BUY — Sitabuldi | ₹50 |
| 7 | **Push button** | 1 | 6 mm tactile, 4-pin | BUY or old mouse/remote | ₹5 |
| 8 | **USB-C cable** | 1 | **must carry data** | HOME | ₹0 |
| 9 | **USB power source** | 1 | laptop or power bank | HOME | ₹0 |

**Total: ₹330–490.** Two things come from a shop that stocks them in Nagpur (₹328),
the rest is local and cheap.

This runs off your laptop's USB port. It records when you press the button, saves WAV
files to the card, and can serve them over WiFi. No battery, no case, no soldering.

### Nice to add at this stage

| Part | Qty | Why | Price |
|---|---|---|---|
| LED + 330 Ω resistor | 1 each | a status light you can see. Skip if using the board's built-in LED | ₹5 |
| Extra ESP32-C3 board | 1 | a spare. A dead board on day one is the classic way a first project dies | ₹199 |

## B. To make it a wireless wearable — add later

| # | Part | Qty | Spec | Price |
|---|---|---|---|---|
| 10 | **TP4056 charging board** | 1 | **Type-C**, with protection | ₹30–45 |
| 11 | **LiPo cell** | 1 | 3.7 V, 200–1000 mAh, pouch | ₹0 salvaged / ₹150–250 new |
| 12 | **JST connector or thin wire** | 1 set | to join cell to TP4056 | ₹20 |
| 13 | Case | 1 | Tic Tac / Chlormint box or 3D print | ₹0–350 |
| 14 | Lanyard or clip | 1 | ID lanyard, keyring | ₹0 |
| 15 | Foam windscreen | 1 | for wind noise when outdoors | ₹20 |

**Add ~₹70 + whatever you spend on the cell and case.**

Case: a Tic Tac box fits the **soldered** version only, and even then it's 1–2 mm too
thick — see `BOX.md`.

## C. Tools

| Tool | Needed for | Buy? | Price |
|---|---|---|---|
| **Soldering iron** (+ solder, flux) | Phase 2: making it small. Not needed for the breadboard prototype | only for Phase 2 | ₹300–500 |
| **Multimeter** | checking voltages, finding which wire is which, testing a salvaged cell | strongly recommended | ₹250–400 |
| Wire strippers | stripping salvaged cable | optional — scissors work | ₹100 |
| Side cutters | trimming header pins | optional | ₹120 |
| Helping-hands / tweezers | holding tiny wires | optional | ₹100 |

You can build and fully test Section A **without a soldering iron**. Don't buy one until
the prototype works.

## D. Consumables

| Item | Use | Price |
|---|---|---|
| Heat-shrink tube (assorted) | insulating joints | ₹40 |
| Electrical tape | sealing the case | ₹20 |
| Double-sided foam tape | sticking the battery down | ₹30 |
| Single-strand wire (or old Ethernet cable) | permanent connections later | ₹0–30 |

## E. Salvage list — check your house before buying

| Look for | Where | Used for | Worth |
|---|---|---|---|
| **microSD card** | old phone, camera, dashcam, phone box, MP3 player | the recordings | ₹250 |
| **USB-C / micro-USB cable** | any drawer — must carry **data**, not just power | powering + programming | ₹50 |
| **Old power bank** | drawer | replaces the battery, TP4056 and charger entirely | ₹200+ |
| **Old Ethernet / LAN cable** | near the router | 8 thin wires, ideal for permanent connections | ₹30 |
| **Old mouse or TV remote** | junk box | the tactile push button inside | ₹5 |
| **Old earphone / headset** | drawer | thin flexible wire | ₹20 |
| **Battery cell** | dead phone, earbuds case, fitness band | power (see `BATTERY.md` — read the safety part) | ₹150–250 |
| **Small plastic box** | kitchen / tools | case | ₹50 |
| **ID lanyard / keyring** | drawer | wearing it | ₹20 |

## F. Optional upgrades, later

| Upgrade | Why | Price |
|---|---|---|
| ESP32-S3 board instead of C3 | can do more audio work on-device | ₹450+ |
| Second mic | better pickup of far-away speakers in a meeting room | ₹129 |
| Proper 3D-printed case | looks like a real product | ₹150–350 |
| Bigger cell (1000 mAh) | ~20 h recording instead of ~4 h | ₹250 |
| Lavalier mic on a cable | better meeting audio | ₹300 |

---

## The wiring these parts imply

```
INMP441  ->  ESP32-C3          microSD module -> ESP32-C3
  VDD   ->  3V3                  VCC   ->  3V3
  GND   ->  GND                  GND   ->  GND
  L/R   ->  GND                  CS    ->  GPIO7
  WS    ->  GPIO1                SCK   ->  GPIO6
  SCK   ->  GPIO0                MOSI  ->  GPIO5
  SD    ->  GPIO3                MISO  ->  GPIO4

Button  ->  GPIO2  and  GND
LED     ->  GPIO8  (or the board's built-in LED)
```

9 of the board's ~11 usable pins are used. That's why the pin choice is fixed, and why
swapping to a different board model means changing the code.

---

## Totals at a glance

| Scenario | Cost |
|---|---|
| **Bare prototype**, card + cable from home, no soldering | **₹330–490** |
| Prototype + multimeter + spare board | ₹780–1,090 |
| Prototype + soldering iron | ₹630–990 |
| **Finished wireless pendant** (salvaged cell + Tic Tac box) | ₹400–600 |
| **Finished wireless pendant** (new cell + 3D-printed case + iron) | ₹1,000–1,500 |

Compare with the real Neo 1 at ₹11,699.

## Buy in this order

1. **ESP32-C3 Super Mini + INMP441** — Robo Bazar, ₹328. Everything else is worthless
   without these.
2. **Breadboard + jumper wires + SD module** — Sitabuldi, ~₹155.
3. **Card and cable** — from your drawers. Buy only if genuinely absent.
4. Get it recording on a breadboard. **Do not buy anything from Section B or C until this works.**
