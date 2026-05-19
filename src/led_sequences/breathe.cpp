// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Arduino.h>
#include "led_sequences.h"

void led_breathe(Adafruit_NeoPixel &s, led_breatheAnim &anim) {

  anim.delay = anim.defaultDelay;

  // Change direction when appropriate
  if (anim.curBright >= anim.maxBright) {
    anim.raising = false;
    anim.delay = anim.defaultDelay; // don't pause at the top
  }
  if (anim.curBright <= anim.minBright) {
    anim.raising = true;
    anim.delay = anim.pauseDelay;
  }

  // Increment or Decrement accordingly
  if (anim.raising) {
      anim.curBright += anim.stepBright;
  } else {
      anim.curBright -= anim.stepBright;
  }

  // Set the pixels
  for (uint16_t i = 0; i < anim.length; i++) {
    uint16_t idx = (anim.startIndex + i) % s.numPixels();
    if (anim.curBright > 0) {
      s.setPixelColor(idx,
        (uint8_t)(anim.r * anim.curBright),
        (uint8_t)(anim.g * anim.curBright),
        (uint8_t)(anim.b * anim.curBright),
        (uint8_t)(anim.w * anim.curBright)
      );
    } else {
      s.setPixelColor(idx, 0, 0, 0, 0);
    }
  }

}
