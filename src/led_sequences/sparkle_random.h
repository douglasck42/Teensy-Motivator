// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <Arduino.h>
#include "led_sequences.h"
#include <Adafruit_NeoPixel.h>

struct led_sparkleRandomAnim : led_animation {
  uint8_t count = 0;         // how many pixels to light (1 .. 100+) - will be randomized as the routine functions
  uint8_t minCount = 0;      // minimum number of random pixels to light
  uint8_t maxCount = 0;      // maximium number of random pixels to light
};

void led_sparkleRandom(Adafruit_NeoPixel &s, led_sparkleRandomAnim &anim);
