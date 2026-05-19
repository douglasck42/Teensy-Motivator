// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Arduino.h>
#include "sparkle_random.h"

// --------------------------------------
// Function to randomly light LEDs in a range - sparkle fill
// --------------------------------------
void led_sparkleRandom(Adafruit_NeoPixel &s, led_sparkleRandomAnim &anim) {
  //if (anim.length == 0 || anim.count == 0) return;
  if (anim.serialDebug) Serial.println("sparkleColorFill");
  uint16_t np = s.numPixels();

  // --- Step 1: turn OFF all LEDs in this range only
  for (uint16_t i = 0; i < anim.length; i++) {
    uint16_t idx = (anim.startIndex + i) % np;
    s.setPixelColor(idx, 0); // off
  }

  // --- Step 2: light random ones
  for (uint8_t i = 0; i < anim.count; i++) {
    uint16_t rIndex = random(anim.length); // random offset in this range
    uint16_t idx = (anim.startIndex + rIndex) % np;
    if (anim.serialDebug) Serial.printf("sparkleColorFill: %d = %d,%d,%d,%d\n", idx, anim.r, anim.g, anim.b, anim.w);

    s.setPixelColor(idx, s.Color(anim.r, anim.g, anim.b, anim.w));
  }

  if (anim.minDelay != anim.maxDelay) {
      anim.delay = random(anim.minDelay, anim.maxDelay);
  }
  //anim.count = random(anim.minCount, anim.maxCount);
  // Show results (or let caller show after combining multiple anims)
  //s.show();
}