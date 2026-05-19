// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Arduino.h>
#include "spinHeadTail.h"

//----------------------------------------------------
// Draw only this animation’s pixels, leave others alone
//----------------------------------------------------
void led_spinHeadTail(Adafruit_NeoPixel &s, led_SpinAnim &anim) {
    if (anim.serialDebug) Serial.print("Wipe: ");
    // 1. Wipe (to off) only the pixels we used last time
    for (uint16_t i = 0; i < anim.trailLen; i++) {
        uint16_t idx = (anim.startIndex + (anim.pos + i) % anim.length) % s.numPixels();
        s.setPixelColor(idx, 0); // zero = off
        if (anim.serialDebug) {
            Serial.print(" ");
            Serial.print(idx);
        }
    }
    if (anim.serialDebug) Serial.println("");

    // 2. Advance position around the ring
    anim.pos = (anim.pos + 1) % anim.length;
    if (anim.serialDebug) {
        Serial.print("Position: ");
        Serial.println(anim.pos);
    }

    if (anim.serialDebug) Serial.print("Draw: ");
    // 3. Draw the new head and trailing pixels
    for (uint16_t i = 0; i < anim.trailLen; i++) {
        // "0" means head, "1" .. "trailLen-1" are tail steps behind
        uint16_t offset = (anim.pos + anim.length - i) % anim.length;
        uint16_t idx = (anim.startIndex + offset) % s.numPixels();

        // compute fade: head (i=0) = 255, tail = fades down
        float u = (float)i / (anim.trailLen - 1);  // 0.0 → 1.0 along tail
        float t = 1.0f - u;                        // keep 1.0 at head
        uint8_t fade = 255 * t * t;                // quadratic drop

        uint32_t color = s.Color(
            (anim.r * fade) / 255,
            (anim.g * fade) / 255,
            (anim.b * fade) / 255,
            (anim.w * fade) / 255
        );

        s.setPixelColor(idx, color);
        if (anim.serialDebug) {
            Serial.print(" ");
            Serial.print(idx);
        }

    }
    if (anim.serialDebug) Serial.println("");

    if (anim.minDelay != anim.maxDelay) {
        anim.delay = random(anim.minDelay, anim.maxDelay);
    }

  // caller decides when to strip.show()
}
