// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

struct LedStrip {
  Adafruit_NeoPixel &strip;
  bool isRGBW = false;
  bool useGammaCorrection = false;
  uint8_t brightness = 128; // 0–255
  uint16_t startIndex = 0;   // starting LED Number, 0-Indexed
  uint16_t length = 0;       // number of LEDs in range

  LedStrip(Adafruit_NeoPixel &s,
           bool rgbw,
           bool gamma,
           uint8_t bright,
           uint16_t start,
           uint16_t len)
    : strip(s),
      isRGBW(rgbw),
      useGammaCorrection(gamma),
      brightness(bright),
      startIndex(start),
      length(len) {}
};

struct led_animation {
  uint16_t startIndex = 0;   // starting LED Number, 0-Indexed
  uint16_t length = 0;       // number of LEDs in range
  uint16_t delay = 500;      // animation speed in ms - will be randomized as the routine functions
  uint16_t minDelay = 500;   // animation speed in ms - minimum for randomization - as shown, a 0ms variation
  uint16_t maxDelay = 500;   // animation speed in ms - maximum for randomization - as shown, a 0ms variation
  unsigned long lastUpdate = 0;   // when was this last updated
  bool serialDebug = false;     // enable serial debugging
  bool enabled = false;      // run this animation

  // base color
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  uint8_t w = 0;    
};

void setupLEDSequence();
void stopLEDSequence();
void solidFill(Adafruit_NeoPixel &s, uint32_t color, uint8_t start_led, uint8_t number_of_leds);
uint32_t hexColor(LedStrip &s, uint32_t hex, uint8_t brightnessOverride = 255);
uint32_t RGBToHex(uint8_t r, uint8_t g, uint8_t b);
#define INCLUDE_REFERENCE_FUNCTIONS 0
#if INCLUDE_REFERENCE_FUNCTIONS == 1
void hexToRGB(uint32_t color, uint8_t &r, uint8_t &g, uint8_t &b);
void hexToRGBW(uint32_t color, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &w);
uint32_t hexColorRGB(Adafruit_NeoPixel &s, uint32_t hex);
uint32_t hexColorRGBW(Adafruit_NeoPixel &s, uint32_t hex);
#endif

#include "sparkle_random.h"
#include "spinHeadTail.h"
#include "breathe.h"
#include "game_of_life.h"
//#include "Sprites_8x8/sprites_8x8.h"
