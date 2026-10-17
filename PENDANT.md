# Pendant design

Now the shape matters more than the electronics. So does one thing you might not have
thought about: **a pendant sits on your chest, in the wind, under or over your shirt.**

---

## Target size and weight

Putting the four modules plus a small cell together:

| Item | Size | Weight |
|---|---|---|
| ESP32-C3 Super Mini | 22 × 18 × 4 mm | 4 g |
| microSD module | 24 × 18 × 3 mm | 3 g |
| TP4056 charging board | 26 × 17 × 3 mm | 3 g |
| LiPo cell, 200–300 mAh | 30 × 20 × 4 mm | 5–7 g |
| Button + wires | — | 2 g |
| Case | — | 8–12 g |

**Result: roughly 45 × 35 × 12 mm and about 30 grams.**

For scale: a small matchbox cut in half. Real Neo 1 is 20 g, so we're about 10 g heavier.
Perfectly wearable.

**A 3.7 V cell of only 200 mAh gives about 4 hours of recording** — that's five
45-minute meetings per charge. You do not need a big battery for a pendant. Keeping the
cell thin is worth more than making it big.

---

## Layout

```
        FRONT  (this face points AWAY from your chest)

        ┌───────────────────────────────┐
        │  ● LED                        │
        │                               │
        │            ◉  MIC             │   ◉ = hole in the case,
        │                               │       foam behind it
        │          NeoClone             │
        └───────────────────────────────┘

        TOP EDGE  (findable by feel, press with a thumb)

        ┌───────────────────────────────┐
        │           [ BUTTON ]          │
        └───────────────────────────────┘

        BOTTOM EDGE

        ┌───────────────────────────────┐
        │  [USB-C]                      │
        └───────────────────────────────┘
```

Inside, stacked like a sandwich:

```
   front shell  ── mic hole ── LED window
   ESP32-C3 board
   microSD module
   TP4056 board
   LiPo cell
   back shell
```

Absolute rule: **the mic hole must not be where the chain or cord attaches.** A chain
rubbing against the case near the mic produces constant scraping noise in every
recording. Put the cord loop on the top edge, opposite side from the mic.

---

## Battery options, and what each does to the shape

| Source | Size | Capacity | Pendant-shaped? |
|---|---|---|---|
| Old **fitness band / earbud case** | tiny pouch | 100–200 mAh | yes, ideal |
| Old **phone battery** | thin pouch, ~60 × 45 × 5 mm | 2000–4000 mAh | great shape, but **hard to extract and to solder** |
| Old **power bank** | 18650 cylinder, 18 × 65 mm | 2000–3000 mAh | **no** — like a marker pen, far too chunky |
| Old **laptop battery** | several 18650 cylinders | large | no — same problem |

For a pendant you want a **thin pouch cell**. A cylinder is out.

The awkward truth: the pouch cell with the best shape and capacity is inside a glued
phone, which is the risky extraction. A small pouch from a dead fitness band or earbud
case is far easier and plenty for meetings.

If you'd rather not open anything at all: **wear the pendant on a lanyard and keep a
small power bank in your pocket**, joined by a thin cable. Ugly, but zero risk, and you
can upgrade later.

---

## Case options

| Option | Cost | Looks | Note |
|---|---|---|---|
| **3D printed** | ₹0 if you have access; ₹100–300 from a shop or online | best | ask a local print shop, a college lab, or an online print service |
| Small **project box** from an electronics shop | ₹30–60 | okay | comes in 50 × 35 mm sizes, easy to drill |
| **Tic-Tac box / mint tin / pill box** | ₹0 | rough | classic first prototype, works fine |
| **Heat-shrink tube** wrapped tight | ₹20 | ugly | no button access, so only for proof-of-concept |

Whatever you use, three holes: one for the mic (with foam behind it), one you can see
the LED through, and access to the USB-C socket.

---

## Wear position, and why it decides everything

| Worn how | Mic quality | Button | Verdict |
|---|---|---|---|
| **Outside on the lanyard**, chest height | good | easy | best for walking |
| **Under your shirt** | muffled, cloth rustle | can't reach it | don't |
| **Laid on the table** for meetings | best | easy | do this for meetings |
| **Clipped to a collar or shoulder** | good, closest to your mouth | tricky | good for quiet thoughts |

For meetings, take it off and **lay it flat on the table**, mic facing up. That single
habit will improve your transcripts more than any electronic upgrade.

---

## Wind is the pendant's weak point

A pendant hangs in open air on your chest. Outdoors, every gust hits it head on.

- Glue a piece of **foam windscreen** (₹20 a pack) over the mic hole, inside the case.
- Keep the mic opening facing **down and sideways**, not straight forward into the wind.
- Don't leave the hole fully open with bare electronics behind it.

If you skip this, every outdoor recording will be dominated by rumble.

---

## Button ergonomics

Two opposing problems, and the pendant has to satisfy both:

- **Too easy to press** → your lanyard or your shirt presses it, recording stops
  mid-meeting, or it starts recording in your bag. Very likely. Will happen.
- **Too hard to press** → you can't start it when you want to.

Practical answer for a pendant:
- Put the button on the **top edge**, so a thumb finds it by feel and clothing doesn't
  rest on it.
- Use a **recessed** button, not a raised one.
- For meetings, fix it in software: **long press to stop**, or a double click, so an
  accidental single touch can't end a 45-minute recording.

---

## What this means for the plan

The pendant shape pushes you towards:

1. A **thin pouch cell** (small, salvaged from a cheap device) over a cylinder.
2. A **3D printed or small project box** case, with a mic hole, LED window and USB-C cutout.
3. **Foam** over the mic, and the cord loop kept away from the mic.
4. A **recessed top-edge button**, with a software guard against accidental stops.

None of that changes the shopping list of two must-buy parts. It changes how you
finish it.

## To decide

1. Do you have access to a **3D printer** (a friend, a college lab, a local shop)?
2. Wear it **visibly on the lanyard**, or clipped under a collar?
3. Can you get a **small pouch battery** from a dead fitness band / earbuds case, or
   would you rather take the battery out of one of your dead phones?
4. **Rounded rectangle, or round** like a coin? (Round looks better, rectangular is much
   easier to fit the parts into.)
