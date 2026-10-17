# How to get the outer box

Four routes, cheapest and easiest first. You can start at the top and upgrade later —
the box is genuinely the **last** thing you need.

---

## Route 1 — Free: reuse something you already own

| What | Why it works |
|---|---|
| **Old TWS earbud charging case** | Best free option. Already has a **USB-C hole**, a hinge, a magnet lid — and a battery inside you can reuse. |
| **Old Bluetooth camera shutter remote** | Small, pendant-shaped, has a button and often a USB port. Almost designed for this. |
| **Tic-Tac box / mint box** | The classic first prototype. Works fine, looks rough. |
| **Small plastic box, pill box, spectacle case** | Any small rigid box. Too big is worse than too small — bigger looks less like a pendant. |
| **Dead power bank shell** | Often a nice rounded case with a USB-C cutout already. |

A dead TWS earbud case or an old camera shutter remote are the two that genuinely
surprise people. Look in your drawers before buying anything.

## Route 2 — ₹60–225: buy a plastic project box

Sold as "**ABS project box**", "**DIY enclosure**", "**utility enclosure**" or "**junction
box**" on Robu, Robocraze, Amazon or Flipkart.

Target external size: roughly **50 × 35 × 20 mm**. Anything in that ballpark works.

One nice find: the **DFRobot Beetle project box** on Robu is **36 × 36 × 15 mm** for
₹146 — almost exactly pendant-sized already.

Then you need three holes:

- **Microphone** — 1–2 mm, with foam behind it
- **LED window** — 3 mm, or cover it with frosted tape so it glows instead of glaring
- **USB-C cutout** — rectangular, make it slightly oversized and tidy it with a small file

## Route 3 — ₹150–400: get it 3D printed

This is how you get something that looks like the real thing.

**Step 1: you need a design file (an .stl).** Design it yourself in **TinkerCAD** —
free, runs in a browser, drag-and-drop, genuinely beginner-friendly. Or ask me and I'll
write you a parameterised model you can render and hand over.

**Step 2: get it printed.** Options:

| Where | Price | Note |
|---|---|---|
| A **local print shop** — search Google Maps for "3D printing service near me" | ₹150–300 | Also try your city's industrial area, and nearby engineering colleges, which often print for students |
| **zbotic.in**, **dwartindustries.com**, **iamrapid.com** | ₹150–300 minimum charge | Upload .stl, get an instant quote, ships anywhere in India |
| **jlc3dp.com** | from about $0.30 + shipping | Cheapest per part, ships from China, slower |

Indian 3D printing minium charges run **₹150–300 per order**, and material is about
**₹1.50–3 per gram**. Your case weighs roughly 10–15 g, so you're realistically at the
minimum charge either way. Print in **PLA** (fine) or **PETG** (better, slightly dearer).

## Route 4 — ₹0: no box at all

Wrap the whole thing in **heat-shrink tube** and tape, or set it in silicone. Ugly, no
button access, and fine for proving the electronics work. Not a pendant.

---

## The honest bit about thickness

I earlier drew the pendant at **12 mm**. With the boards stacked on their header pins
inside a box, **expect closer to 15–16 mm** — a bit chunky.

To get back down near 12 mm you have to **solder the boards directly to each other
without headers**, which removes roughly 5 mm of pin height. That's a Phase 2 job, after
you've proven it works on a breadboard.

So: first build, don't chase thin. Get it working in whatever box fits.

---

## Making the holes — the beginner-safe way

Drilling thin plastic cracks it. The reliable trick:

1. Take a **needle or a thin nail**, hold it with pliers.
2. **Heat it over a candle** for ten seconds.
3. **Push it through** the plastic — it melts a clean hole with no cracking.
4. For the USB-C slot, melt a row of holes and tidy the edges with a small file or a
   craft knife.

A soldering iron tip also works, but it ruins the tip's plating, so use an old one.

**Don't glue the box shut.** You'll want it open for charging, reprogramming, and the
first three attempts at fitting everything in.

---

## What I'd actually do, in order

1. **Tic-Tac box** for the first working prototype. Free, ugly, done in five minutes.
2. Once it works, look for a **dead TWS earbud case** or a **camera shutter remote** in
   your drawers. Free, and they already look like a product.
3. Only if neither exists, **buy a ₹60–225 project box** or **pay ₹150–300 for a 3D
   print**.

Do not spend money on the box before the electronics work. It is the easiest part to
change and the easiest part to waste money on.

---

## What I need to help further

- **Your city** — so I can name a specific local print shop instead of "search Google Maps".
- Whether you want me to **write you an .stl / OpenSCAD model** of the case.

---

# Will it fit in a Tic Tac box?

## The numbers

**Tic Tac box** (standard 18 g pack): roughly **62 × 38 × 16 mm outside**, so about
**56 × 34 × 13 mm of usable space inside**. It varies between packs — **measure yours
with a ruler**, the internal depth is the number that decides everything.

**Our parts:**

| Part | Size (mm) | Thickness with header pins |
|---|---|---|
| ESP32-C3 Super Mini | 22.5 × 18 × 3.5 | ~11 |
| INMP441 mic | 14 × 12 × 3 | ~10 |
| microSD module | 24 × 18 × 3.5 | ~11 (card adds ~2) |
| TP4056 charging board (Type-C) | 26 × 17 × 3.5 | ~11 |
| LiPo cell, 200–300 mAh | ~30 × 20 × 4 | — |
| Tactile button | 6 × 6 × 5 | — |

## Verdict

| How you build it | Fits? | Why |
|---|---|---|
| **Breadboard + jumper wires** | **No** | the breadboard alone is 47 × 35 × 9 mm, and jumpers stick up another 10 mm |
| **With header pins everywhere** | **No** | the stack becomes 20–25 mm thick, roughly double the box |
| **Soldered flat, no headers** | **Yes, barely** | needs ~14–15 mm, box gives ~13 mm — about 1–2 mm short |
| **Soldered flat + no battery** (powered by a USB power bank) | **Yes, comfortably** | only ~9–10 mm of boards, no cell and no TP4056 |

## The two-layer packing that comes closest

```
Tic Tac box, 56 × 34 mm footprint

  top view                                  side view
  ┌──────────────────────────────┐          ┌──────────┐
  │ ┌────────┐ ┌───────────┐     │   layer3 │ SD + mic │ 5.5 mm
  │ │microSD │ │  INMP441  │     │   layer2 │ C3+TP4056│ 3.5 mm
  │ └────────┘ └───────────┘     │   layer1 │  LiPo    │ 4.0 mm
  │ ┌────────┐ ┌───────────┐     │          └──────────┘
  │ │ESP32-C3│ │  TP4056   │     │           13 mm + wiring
  │ └────────┘ └───────────┘     │
  └──────────────────────────────┘
```

Layer 2 works out to 22.5 + 26 = 48.5 mm along the long axis, which fits in 56 mm.
Footprint is fine. **Thickness is the only problem.**

## How to claw back those 2 mm

- Use a **thinner cell** — a 150 mAh pouch is 2–3 mm instead of 4 mm.
- **Don't close the hinge.** Tape the lid down with a slight bulge; nobody will notice.
- File the **inside ribs** off the box lid — Tic Tac boxes have raised ridges that eat
  a millimetre or two.
- Trim the **header pins flush** on the boards after soldering, and butt the PCBs directly
  against each other.

## The simplest answer for right now

You said you want a **working prototype and don't care about the box**. So:

**Don't put the prototype in a box at all.** Wire it on a **mini breadboard**, rubber-band
the battery to the side, and use it like that for a week. Once you know the electronics
work, then solder it flat and slide it into the Tic Tac box.

Putting a breadboard prototype into a mint box just makes it harder to debug — and you
*will* be debugging it.

## If you want it in a box today

Power it from a **USB power bank** instead of a battery. Then there's no cell and no
TP4056 board, the electronics are under 10 mm thick, and it drops into a Tic Tac box
easily. You lose the wireless freedom, but you get a boxed, working device this week.

## ⚠️ Two safety notes about a mint box

- **Never charge a LiPo inside a closed plastic box unattended.** Mint box plastic is thin
  and not flame-retardant. Charge it out in the open, on something non-flammable.
- **A swollen cell will crack a Tic Tac box open silently.** Check it by hand once a month;
  if the lid is bulging, stop using that cell immediately.

## Also worth knowing

A Tic Tac box is **transparent**, which is actually a small advantage: you can see the LED
through it with no window cut at all. You still need to cut a **mic hole** and a **USB-C
opening** — the hot-needle trick in the section above works fine on this plastic.

**Chlormint** and similar flat Indian mint boxes are the same shape and are easier to find
in Nagaur than Tic Tac. Same analysis, same dimensions, worth ₹10.
