/*
 * wiretest - checks the mic wiring with the mic REMOVED.
 *
 * Use one spare jumper as a probe. Put one end in the mic's SD row.
 * Touch / plug the other end into each of the other mic rows in turn
 * (VDD, GND, SCK, WS, L/R). The Serial Monitor says what the SD line
 * is touching. Expected:
 *   VDD row -> 3.3V      GND row -> GND      L/R row -> GND
 *   SCK row -> SCK (pin 0)   WS row -> WS (pin 1)   no probe -> nothing
 *
 * Safe: pin 3 (SD) is only ever an input. Pins 0 and 1 only drive
 * their own rows.
 * Board: ESP32C3 Dev Module, USB CDC On Boot = Enabled, 115200 baud.
 */
#include <Arduino.h>

#define PIN_SCK 0
#define PIN_WS  1
#define PIN_SD  3

static const char *last = "";

static const char *probe() {
  // pins 0/1 released, check for a fixed level on SD
  pinMode(PIN_SCK, INPUT);
  pinMode(PIN_WS,  INPUT);
  pinMode(PIN_SD, INPUT_PULLUP);   delayMicroseconds(300);
  int up = digitalRead(PIN_SD);
  pinMode(PIN_SD, INPUT_PULLDOWN); delayMicroseconds(300);
  int down = digitalRead(PIN_SD);
  if (up == 0 && down == 0) return "GND";
  if (up == 1 && down == 1) return "3.3V";

  // floating: see if pin 0 or pin 1 can move it
  pinMode(PIN_SCK, OUTPUT);
  pinMode(PIN_WS,  OUTPUT);
  digitalWrite(PIN_SCK, HIGH); digitalWrite(PIN_WS, LOW);  delayMicroseconds(300);
  int a = digitalRead(PIN_SD);
  digitalWrite(PIN_SCK, LOW);  digitalWrite(PIN_WS, HIGH); delayMicroseconds(300);
  int b = digitalRead(PIN_SD);
  digitalWrite(PIN_WS, LOW);
  pinMode(PIN_SCK, INPUT);
  pinMode(PIN_WS,  INPUT);

  if (a == 1 && b == 0) return "SCK (pin 0)";
  if (a == 0 && b == 1) return "WS (pin 1)";
  return "nothing";
}

void setup() {
  Serial.begin(115200);
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 30000) delay(50);
  delay(500);
  Serial.println(F("\n=== wire test (mic removed) ==="));
  Serial.println(F("Probe jumper: one end in the SD row, other end into each mic row."));
  Serial.println(F("Expect: VDD->3.3V  GND->GND  L/R->GND  SCK->SCK (pin 0)  WS->WS (pin 1)\n"));
}

void loop() {
  const char *now = probe();
  if (strcmp(now, last) != 0) {
    Serial.printf("SD row is touching: %s\n", now);
    last = now;
  }
  delay(150);
}
