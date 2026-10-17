# START HERE — NeoClone in plain English

No hardware experience assumed. Read this top to bottom.

---

## What we're actually building

A small device you wear on a lanyard or clip.

- Press the button → it starts listening.
- Say your thought out loud.
- Press again → it stops and saves.
- Later, you open your phone, connect to it, and the recording comes across.
- Your phone turns the recording into written text and a short summary.

That's the whole product. It is a **wearable voice-memo button that writes things down
for you later**. The expensive Neo Sapien (₹11,699) does the same job with fancier
electronics.

---

## The parts, and what each one actually is

Think of it as building a tiny computer, one piece at a time.

### 1. ESP32-C3 Super Mini — about ₹210

A tiny computer, roughly the size of your thumbnail. Two important things about it:

- It has **WiFi built in**, so your phone can talk to it.
- It can read sound from a digital microphone.

In the team of parts, this is **the brain**.

### 2. INMP441 microphone module — about ₹130

A small green board, about 1.5 cm across, with a microscopic microphone chip on it.

Why this one and not a cheaper mic? Because it is **digital**. Cheap microphones send
a wobbly electrical signal that picks up hiss and buzzing. This one hands over clean
numbers instead. Your voice comes out much clearer.

In the team, this is **the ear**.

### 3. microSD card module, plus a memory card — about ₹45

A small adapter board with a slot in it. You push a memory card into it (the same kind
of card that goes inside a phone or a camera). It lets the tiny computer save files —
your voice memos live here.

In the team, this is **the notebook**.

### 4. TP4056 charging board — about ₹30

A very small board that safely recharges a battery from a normal USB cable. Without it
you could never recharge the device. Only needed once you add a battery.

In the team, this is **the charger**.

### 5. A small rechargeable battery — ₹150–250 new, or free

A flat, thin battery the size of a small chocolate square. Same technology as the
battery inside a phone or a power bank.

In the team, this is **the fuel**.

### Also needed

- **A push button** — ₹5. The thing you press.
- **Wires** — a few rupees. Connect the parts together.
- **A case** — a small plastic pill box, or a piece of heat-shrink tube. Keeps it tidy.

---

## The one thing nobody warns beginners about: soldering

To connect these parts, metal has to be joined. The normal way is **soldering** —
melting a little bit of metal wire with a hot tool called a **soldering iron**.

- A basic soldering iron costs **₹300–500** and you buy it once.
- It is a real skill, but it is not hard, and there are hundreds of short videos.
- If you don't own one, that is an **extra cost on top of the parts**.

**Good news: for your very first build you can avoid soldering completely.**
There is a reusable board called a **breadboard** where you just push wires into holes.
No heat, no skill, nothing permanent. It looks messy, but it works, and it proves the
whole idea before you spend money on an iron.

---

## What "salvage" means

Salvage = **take a part out of a broken device instead of buying it new.**

- An old dead phone, or an old power bank → contains a rechargeable battery you can reuse.
- An old phone, dashcam, action camera, or a spare from a phone box → contains a memory card.

**Never reuse a battery that looks puffed, swollen, or dented.** That is a fire risk.

---

## The honest recommendation for someone starting from zero

Build it in **two phases**, cheapest and easiest first.

### Phase 1 — a working prototype, no soldering, no battery: about ₹490

| Part | Price |
|---|---|
| ESP32-C3 Super Mini | ₹210 |
| INMP441 microphone | ₹130 |
| microSD card module | ₹45 |
| Breadboard (reusable plug board) | ₹60 |
| Jumper wires (pack) | ₹50 |
| Memory card | free from old junk |
| Power: a USB power bank or your laptop | free |

You plug everything into the breadboard, plug the USB cable in, and it works. Nothing
is permanent, nothing can be ruined, and you learn how it all fits together.

**Note this lands at about ₹490 without needing to salvage a battery at all.**

### Phase 2 — make it wearable: add ₹300–450

Only after Phase 1 works:

- Add the TP4056 charging board (₹30) and a battery (free salvaged, or ₹150–250).
- Buy a soldering iron (₹300–500) and solder the parts onto a small board.
- Put it in a case, add a lanyard.

Now it is a real pendant with no wires.

---

## The truth about the ₹500 target

- **Phase 1 fits under ₹500** and gives you something that genuinely works.
- **Phase 2 (fully wireless, wearable) does not fit under ₹500** unless you already own
  a soldering iron and can salvage a battery and a memory card. Realistically it is
  ₹500–900 depending on what you already have.

Anyone who tells you a wireless AI pendant costs under ₹500 is leaving out the
soldering iron and the battery.

---

## What I need to know from you

Answer these three and I'll write your exact shopping list:

1. **Which city are you in?** So I can name the actual nearby market, or a specific
   online store that ships to you cheaply.
2. **Do you own a soldering iron, or are you willing to buy one?** This decides whether
   we go breadboard-only (easy, ₹490) or fully wearable (more money, more skill).
3. **What old electronics do you have lying around?** Ideally look for:
   - a dead phone, tablet, or power bank (→ battery)
   - a spare microSD card, from a phone box or an old camera (→ storage)

If the answer to all three is "nothing and no idea" — that's fine too. The honest
cheapest option is to skip hardware entirely and just use a voice memo app on your
phone, which costs ₹0. Then build this later as a learning project.
