# NeoClone — hardware decision sheet

Goal: cheapest wearable push-to-talk voice-memo pendant. Software later.

## Verdict: it's doable under ₹500, but only with two salvaged parts

The budget only works if you **reuse a LiPo cell and a microSD card** from old junk
(dead phone, old power bank, old action cam, old dashcam). Those two are ₹150–500 new
and that's what pushes the build over budget. Everything else is ~₹400.

## Recommended approach — Option A: the pendant

| Part | Role | New price | Salvage? |
|---|---|---|---|
| ESP32-C3 Super Mini (USB-C) | brain + WiFi + I2S | ₹209–289 | no |
| INMP441 I2S mic module | digital mic, no ADC noise | ₹129–180 | no |
| TP4056 Type-C w/ protection | charges the cell | ₹29–41 | no |
| microSD SPI module | stores the WAVs | ₹41–49 | no |
| LiPo 3.7 V 500–1000 mAh | power | ₹150–250 | **yes → ₹0** |
| microSD 16 GB | storage | ₹250 | **yes → ₹0** |
| Momentary button, wire, heatshrink | case + button | ₹30 | partly |

## Cheapest sellers found (India, mid-2026, incl. GST)

| Part | Price | Store |
|---|---|---|
| ESP32-C3 Super Mini | **₹209** | probots.co.in (₹177 + 18% GST) |
| ESP32-C3 Super Mini | ₹235 | quartzcomponents.com |
| ESP32-C3 Super Mini | ₹256 | flyrobo.in |
| ESP32-C3 Super Mini | ₹289 | robu.in |
| INMP441 | **₹129** | robobazar.in |
| INMP441 | ₹135 | robocraze.com |
| TP4056 (Type-C, protected) | **₹29** | electronicspices.com |
| TP4056 | ₹41 | robocraze.com |
| microSD SPI module | **₹41** | robocraze.com |
| microSD SPI module | ₹49 | hubtronics.in |

## Three ways to actually buy it — and what each really costs

### 1. Single seller, one order (Robocraze) — safest, ~₹520
₹235 + ₹135 + ₹41 + ₹41 = ₹452, plus their shipping (free only above ₹999).
Salvaged cell + card. **Slightly over ₹500**, one parcel, one return policy.

### 2. Cheapest across sellers — deceptively worst, ~₹620
₹209 + ₹129 + ₹29 + ₹41 = ₹408 of parts, but **three or four separate shipping fees
of ₹50–80 each** = ₹408 + ~₹210 = ₹620. Chasing the lowest unit price costs more.
Only worth it if the sellers offer free shipping at your cart value.

### 3. Local electronics market — cheapest, ~₹400, no shipping
Delhi (Lajpat Rai Mkt), Bengaluru (SP Road), Mumbai (Lamington Rd). Typical walk-in:
C3 ₹200, INMP441 ₹110, TP4056 ₹25, SD module ₹35, button/wire ₹30 = **₹400**.
Bring the parts list and test the mic board on the spot if you can.

## Options if ₹500 is hard

| Option | Change | Saving | Cost |
|---|---|---|---|
| **B — tethered** | no TP4056, no cell; run off a USB power bank in your pocket | −₹30…−₹280 | ₹400 → ₹130, but you're always on a cable |
| **C — no SD** | C3's 4 MB flash instead of card+module | −₹41 | but only ~60–90 s of total audio before you must sync. Fragile. |
| **D — buy 2 mics** | spare INMP441 | +₹135 | cheap clone boards arrive dead or need a different bit-shift. Insurance. |
| **E — phone only** | nothing at all | −₹400 | ₹0, and honestly it does the job today |

## The design that wins on cost *and* usefulness

Push-to-talk (click to record, click to stop) instead of always-on:

- No display, one button, one LED → cheapest possible part count.
- Battery lasts weeks, because it only draws power while you're talking.
- WiFi hotspot sync instead of Bluetooth → the C3 has **no Bluetooth Classic**, so
  WiFi is not just cheaper but the only sane option.
- Phone does all transcription/summarising → free, no cloud, no subscription.

## Decisions I need from you

1. **Battery + card:** can you salvage both? (If yes → under ₹500. If no → ~₹700.)
2. **Sourcing:** single online seller (easy, ~₹520) or local market (~₹400)?
3. **Tethering acceptable?** Option B drops the price hard but needs a power bank.
4. **Buy a spare mic?** Recommended given clone-board quality.
5. **How small does it need to be?** A bigger salvaged 18650 cell = longer life, chunkier pendant.

Once you pick, I'll finalise the single exact shopping list (one cart, exact SKUs,
total including shipping) and *then* we do the software.

---

# Your use case: 45-minute meetings + thoughts while walking

## Capacity check at 16 kHz mono 16-bit

| Resource | 45 min needs | Available | Headroom |
|---|---|---|---|
| Storage | ~86 MB | a 4 GB card holds ~30 h | **~40 meetings** |
| Battery charge | ~37 mAh | a 500 mAh cell | **~7% of one charge** |
| Single file limit | 86 MB | FAT32 allows 4 GB | none needed |

**Verdict: comfortably supported. Capacity is not the problem — the following are.**

## 1. Wind noise — the big one for walking

The INMP441 has a bare port with **no windscreen**. Outdoors, walking, a breeze will
produce loud rumble that drowns your voice and wrecks the transcription.

Fixes, in order of effort:
- Buy a **foam mic windscreen** (₹20 for a pack) — cut a small piece and glue it over
  the mic port in the case.
- Keep the mic port facing **down and away** from the direction you walk.
- Mount the mic behind a small hole with acoustic mesh, not open to the air.

This is the single most important mechanical detail for your use case. Budget ₹20 and
a hole in the case for it.

## 2. Muffling — don't put it in your pocket

A mic inside a pocket or under clothing records cloth rustle and dull, muffled speech.
Whisper will mis-transcribe it.

- For **walking**: hang it outside on a lanyard, at chest height.
- For **meetings**: take it off and **lay it on the table**, mic facing up and clear.

A 45-minute meeting recorded inside a trouser pocket is close to useless.

## 3. Accidental button presses

With click-to-start and click-to-stop, a button in a pocket or on a lanyard against your
body **will get pressed by accident** — stopping your recording 3 minutes in, or starting
one you didn't want.

Options to fix this later (software/hardware):
- Require a **double-click** to stop, or a long press to stop.
- Use a **recessed** button that needs a deliberate push.
- Add a **"session lock"**: first press starts and locks, only a long hold ends it.

For a 45-minute meeting, a lock matters a lot. Note it for the software phase.

## 4. Crash safety on long recordings

If the battery dies during a 45-minute meeting, the file's header is still empty and the
recording is unplayable even though the audio is on the card.

Fix: rotate to a new file every **5 minutes**, and/or re-write the WAV header every ~30
seconds while recording. Then a power failure costs you at most 5 minutes. Do not skip
this — it is how you lose a meeting.

## 5. Transfer and transcription time (45 min clip)

| Step | Roughly |
|---|---|
| Copy 86 MB over the pendant's WiFi hotspot to a phone | 2–4 minutes |
| Transcribe 45 min with Whisper `small` on a laptop | 10–20 minutes |
| Transcribe with Whisper `tiny` | much faster, less accurate |

So a 45-minute meeting is not instant on the phone. If that bothers you, the PC batch
path (card in a reader, run the script overnight) is the better workflow.

## 6. Far-away speakers in a meeting room

The INMP441 is a single omnidirectional mic. It hears the person near it best. People at
the far end of a long table will be quiet, and Whisper may drop or garble them.

- Best fix: put the device **in the middle of the table**, not at your seat.
- If meetings are the main use, a future upgrade is a lapel mic on your collar, or two
  mics. Not needed for v1 — just know the limit.

## Recommended settings for your use case

| Setting | Value | Why |
|---|---|---|
| `SAMPLE_RATE` | 16000 | good speech quality, 86 MB per 45 min |
| File rotation | 300 s (5 min) | crash safety, at most 5 min lost |
| Header re-patch | every ~30 s | file always playable |
| Case | foam over the mic port | wind protection |
| Button | recessed, or long-press to stop | no accidental stops |
| Battery | 500–1000 mAh salvaged | 45 min uses ~7% of it |

## Privacy reminder for meetings

Recording a meeting you're part of is normally fine, but **tell the room**. A meeting
recorded without the other participants knowing is a genuine problem — legally in some
places, and socially everywhere. A visible LED while recording is the honest default.
