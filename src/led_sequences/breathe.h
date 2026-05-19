// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <Arduino.h>
#include "led_sequences.h"
#include <Adafruit_NeoPixel.h>

struct led_breatheAnim : led_animation {
  float curBright = 0.2;       // current brightness as a float 0.0 - 1.0
  float minBright = 0.0;       // minimum brightness as a float 0.0 - 1.0
  float maxBright = 0.6;       // maximum brightness as a float 0.0 - 1.0
  float stepBright = 0.01;      // how much to raise curBright per delay
  bool raising = false;          // raising (True) or falling (False)
  uint16_t defaultDelay = 50; // animation speed in ms - should match above line
  uint16_t pauseDelay = 50;   // how long to pause at bottom of sequence
};

void led_breathe(Adafruit_NeoPixel &s, led_breatheAnim &anim);
