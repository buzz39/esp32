# Getting a battery out of a dead phone — read this fully first

Lithium batteries are the only part of this project that can hurt you. Not "might
void your warranty" hurt — **fire that water does not put out easily**. Everything
else here is harmless if wired wrong; a battery is not.

Read all of this before you open anything.

---

## What's actually inside a dead phone

A dead phone usually has two separate things:

1. A **battery** (thin silver pouch, or a rigid cylinder) — this is what we want.
2. A small **circuit board** stuck to the battery with a few wires. This is the
   battery's bodyguard: it stops overcharging and short circuits. Keep it. Never
   cut it off.

Important: a phone is "dead" for many reasons. The battery is often **perfectly fine** —
the screen broke, the software died, the charging port wore out. You cannot tell by
looking. You'll need a way to measure it later.

---

## Three routes, easiest and safest first

### Route 1 — use the old power bank as-is. No opening anything.

If you have a **power bank** (the brick you charge your phone with), you already own a
complete, safe, ready-made power system:

- it contains a battery,
- it contains a charger,
- it contains all the protection circuits,
- it outputs clean 5 volts over USB.

**You plug the pendant's USB cable straight into it.** Done. No TP4056 needed, no
battery to open, no soldering near a cell, nothing to puncture.

Trade-off: it's two objects instead of one — the power bank in your pocket, the pendant
on your lanyard, joined by a short cable. But it is **free, safe, and works today**,
and it's how I'd build Phase 1.

### Route 2 — an old phone with a removable back cover. Easy.

Old Nokia, old Samsung, old feature phones: you pop the back cover off with your
fingernail and the battery lifts out. It's a solid block with metal contacts, no glue.

These are easy and safe to reuse. You may even find a **holder** that clips onto it,
which means **no soldering near the battery at all**.

**How to tell:** if you can see a seam around the back and it looks like it comes off,
it's this type. If the phone is a glass slab with no visible seam, skip to Route 3.

### Route 3 — a modern glued-in smartphone battery. Hard and risky.

If the phone is a smartphone from the last ~8 years, the battery is **glued in behind
the screen or under the board**. Getting it out usually means heating the phone to
soften the glue and prying with a thin plastic card.

This is exactly where beginners puncture a cell. Once a phone is opened and I cannot
see it, I can't guide you safely. **I'd skip this.** The saving is ₹150, and the risk
is a fire in your home.

---

## The soldering problem nobody mentions

Even after you get a phone battery out, there's an awkward catch:

Phone battery terminals are often made of **aluminium**, and **solder does not stick to
aluminium.** The pros use a spot welder to fuse nickel strips onto the tabs — a tool
you don't have.

So a bare phone battery is genuinely annoying to connect. That is a real reason to
prefer Route 1, or an **18650 cylinder cell** (from an old power bank or an old laptop
battery pack), which has steel tabs that solder normally, and for which **cheap plastic
holders exist for about ₹30 that need zero soldering.**

---

## Safety rules — non-negotiable

- **Never puncture, bend, fold, crush or stab a lithium cell.** Not with a screwdriver,
  not with a pry tool, not "just a little".
- **Never use a metal tool to pry near a cell.** Use plastic.
- **Never short the two terminals together.** No loose wires lying across them.
- **If a cell is puffed, swollen, or bulging — stop. Do not use it.** Dispose of it
  properly (many phone shops and municipal e-waste points accept them).
- **Work on a hard, non-flammable surface** — a tiled floor or a metal tray. Away from
  curtains, bedding, paper.
- **Have a metal lid or a bucket of sand within arm's reach.** Sand smothers a lithium
  fire. Water does not reliably work.
- **If a cell gets hot, smells sweet or chemical, or hisses — walk away from it** and
  let it burn out somewhere it can't spread.
- **Never charge a cell you extracted** unless you know its condition. A deeply
  discharged old cell may be unusable; a damaged one can fail while charging.
  Never charge unattended.
- **Don't leave a charging battery on a bed, sofa, or anything flammable.**

If any of that sounds like more risk than you want: that is a completely reasonable
conclusion. Route 1 avoids all of it.

---

## The smart cheap route, summarised

| Route | Effort | Risk | Cost |
|---|---|---|---|
| 1. Old power bank as-is | none | none | ₹0 |
| 2. Removable-cover phone battery | low | low | ₹0 |
| 3. 18650 cell from old power bank/laptop + ₹30 holder | medium | medium | ₹30 |
| 4. Prying a glued smartphone battery | high | **high** | ₹0 |

Start at Route 1. Move to Route 3 only if you want a single-box wearable and you're
comfortable following a careful video.

---

## What I need to know about your dead phones

1. **Make and model** of each one, if you can read it on the phone or the box.
2. **Does the back cover pop off**, or is it a sealed glass slab?
3. Do you also have an old **power bank** or an old **laptop battery** anywhere?
4. Does anyone at home own a **multimeter** (a small gadget for measuring voltage)?
