// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <Arduino.h>
#include "led_sequences.h"
#include <Adafruit_NeoPixel.h>

struct led_SpinAnim : led_animation {
  uint16_t trailLen = 0;    // how many pixels make the tail (1 = just head)
  uint16_t pos = 0;         // persistent step counter
};

void led_spinHeadTail(Adafruit_NeoPixel &s, led_SpinAnim &anim);
