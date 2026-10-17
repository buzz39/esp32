# The ESP32, explained

You already know what a microprocessor is because you own one — the 8085. So the
easiest way to explain the ESP32 is to compare them.

---

## The one-line version

The ESP32 is a **complete tiny computer on a single chip** — processor, memory,
WiFi and Bluetooth all built in — made by a company called **Espressif**. It costs
about ₹210 on a small board and you program it with a plain USB cable.

---

## 8085 vs ESP32

| | Intel 8085 (yours) | ESP32-C3 |
|---|---|---|
| Made in | 1975 | 2020s |
| Bits | 8-bit | 32-bit |
| Speed | ~3 MHz | 160 MHz — roughly **50× faster** |
| Memory inside | none | 400 KB working memory, 4 MB storage |
| WiFi | no | **yes, built in** |
| Bluetooth | no | yes (low-energy type) |
| Sound input | no | yes, digital audio hardware |
| Built-in clock & timers | needs external chips | yes |
| Extra chips needed to run | clock, RAM, ROM, latch | **none** |
| How you load a program | burn an EPROM with a special programmer | **plug in a USB cable and click Upload** |
| Cost | — | ₹210 |

That last row is the real difference. With the 8085 you had to write assembly, burn it
to an EPROM chip with a separate programmer, plug the chip in, and hope. With the ESP32
you plug in a cable, click a button, and two seconds later it's running.

---

## What's actually inside the chip

The ESP32-C3 is a stack of everything you'd otherwise wire up on a big board:

- **A processor** — 32-bit, RISC-V design, running at up to 160 MHz.
- **Working memory (RAM)** — 400 KB. This is the desk where the program works. Small.
- **Storage (flash)** — 4 MB. This is the cupboard where the program and small files live.
- **WiFi radio** — 2.4 GHz. Can connect to your home WiFi, *or* act as its own
  WiFi hotspot. Our pendant does the second one.
- **Bluetooth Low Energy radio** — for talking to phones. (Note: this version has BLE
  only, not the older "Bluetooth Classic".)
- **A bunch of small support circuits** — timers, a clock, and special blocks for
  talking to other hardware.

---

## What is a "board" versus a "chip"?

The raw chip is a square millimetre of silicon with invisible legs. Useless to a human.

So it gets mounted on a small printed circuit board with the helpful extras:
- a **USB-C socket**, so a normal phone cable powers it and programs it,
- a **voltage regulator**, to turn 5 volts from USB into the 3.3 volts the chip wants,
- **pins** sticking out, so you can connect wires to it,
- a tiny **LED** so you can tell it's alive,
- two **buttons**, one of which is reset.

That whole assembly is what you buy for ₹210. The one we picked is called the
**ESP32-C3 Super Mini** — "Super Mini" because it's about the size of your thumbnail.
Different board makers use different names for the same chip.

---

## The pins — where the outside world connects

The board has about **11 usable pins**. Each one can be told to do a job by your program.

"GPIO" just means **General Purpose Input/Output** — a pin you can either switch on and
off like a light, or read like a switch. That's it. That's the whole idea.

Some pins can also play special roles. Three of them matter to us:

- **I2S** — a digital audio pipe. Instead of a wobbly electrical signal like an old
  cassette, the microphone sends actual numbers: "sample one is 412, sample two is 388…"
  That's why the sound is clean, and it's why we picked the INMP441 mic that speaks I2S.
- **SPI** — a fast pipe for talking to a memory card. Four wires, and it's how the
  ESP32 saves your recordings onto the microSD card.
- **Strapping pins** — a few pins that the chip checks *at the moment it powers on* to
  decide how to start up. We use one of them for the button, so you must not hold the
  button down while plugging the power in. Small gotcha, easy to live with.

Our project uses 9 of the 11 pins: 3 for the microphone, 4 for the memory card, 1 for
the button, 1 for the LED.

---

## What it's good at

- **WiFi projects.** Anything that needs to talk to the internet or a phone, cheaply.
  This is why the ESP32 powers huge numbers of real products: smart plugs, smart bulbs,
  cheap weather stations, WiFi sensors, 3D printer control boards.
- **Reading sensors** and switching things on and off.
- **Being cheap.** Under ₹250 for a computer with WiFi is genuinely remarkable.
- **Having a huge community.** Almost any problem you hit, someone has already posted
  the answer and a working example. This matters a lot when you're learning.

## What it's bad at

- **Heavy computing.** 400 KB of working memory is tiny. It cannot run speech-to-text,
  or any real AI. That's exactly why our design sends the audio to your phone — the
  phone has a thousand times more muscle.
- **Storing lots of data.** 4 MB holds about 90 seconds of audio. Hence the memory card.
- **Being fussy about power.** It wants a steady 3.3 volts and doesn't like being
  yanked. It also draws a fair bit when the WiFi is on.
- **Analog readings.** Its voltage-measuring input is a bit noisy. Another reason to
  use a *digital* microphone rather than a cheap analog one.

---

## The family, so shop listings make sense

"ESP32" is a family, not one thing. If a listing confuses you, here's the map:

| Chip | Cores | Wireless | Notes |
|---|---|---|---|
| **ESP32** (the original) | 2 | WiFi + Bluetooth Classic + BLE | the classic big boards |
| **ESP32-S2** | 1 | WiFi only | no Bluetooth |
| **ESP32-S3** | 2 | WiFi + BLE | extra hardware for cameras and voice — the "fancy" one |
| **ESP32-C3** | 1 | WiFi + BLE | **our pick — cheapest** |
| **ESP32-C6** | 1 | WiFi 6 + BLE + smart-home radio | newer, pricier |

The **C3** is the cheapest way to get WiFi and digital audio in one chip, which is
precisely what our pendant needs. The **S3** would be the pick if you later wanted to do
audio processing *on the device* itself.

---

## How you'd program it

A program for this chip is called **firmware**, and copying it onto the chip is called
**flashing**. You need a computer (your Windows laptop is fine) and three things:

1. A program called the **Arduino IDE** — free.
2. A small add-on inside it called the **ESP32 board support** — free, one click to install.
3. Your program, written in a language called **C++**.

Some people use **MicroPython** instead, which is easier to read. You can also use a
tool called **PlatformIO**. For our project, Arduino is the simplest path and it's what
the code I already wrote is written for.

The whole loop is: write code → plug in the USB cable → click Upload → it runs. That's it.

---

## Care and safety

- **Only 3.3 volts.** Feed 5 volts into a normal pin and you kill the chip. The board
  has exactly one pin labelled for 5V input, meant for power only.
- **Don't short two pins together** while it's powered.
- **It gets slightly warm** in normal use. That's fine. Too hot to touch is not.
- **Unplug before you rewire anything.**
- Handle by the edges; the pins on the underside are sharp.

---

## Why this chip is the right choice for our pendant

| Our requirement | ESP32-C3 |
|---|---|
| Read a digital microphone | yes, via I2S |
| Save audio files | yes, via SPI to a memory card |
| Send files to a phone without a cable | yes, it can be its own WiFi hotspot |
| Fit in a pendant | yes, thumbnail-sized |
| Cost almost nothing | yes, ₹210 |
| Run the speech-to-text itself | **no** — so the phone does it |

That last row is the whole design. The ESP32 is the hands and ears; your phone is the
brain.
