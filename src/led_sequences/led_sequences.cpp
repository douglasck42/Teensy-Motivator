// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Arduino.h>
#include "led_sequences.h"

void setupLEDSequence() {
    // Function implementation
}

void stopLEDSequence() {
    // Function implementation
}

void solidFill(LedStrip &s, uint32_t hex) {
  for(int i=s.startIndex; i<= s.startIndex + s.length; i++) { // For each pixel in strip...
    s.strip.setPixelColor(i, hexColor(s, hex));         //  Set pixel's color (in RAM)
  }
}

// LedStrip body   = { strip_body, false, 128 }; // dim
// LedStrip accent = { strip_accent, true, 255 }; // bright RGBW
// 
// body.strip.setPixelColor(0, hexColor(body, 0xFF3366));
// accent.strip.setPixelColor(0, hexColor(accent, 0xFFEEAA));
// 
// // fade example
// uint8_t fade = 64;
// body.strip.setPixelColor(1, hexColor(body, 0x00AACC, fade));
uint32_t hexColor(LedStrip &s, uint32_t hex, uint8_t brightnessOverride) {

  uint16_t scale = ((uint16_t)s.brightness * brightnessOverride + 127) / 255;

  uint8_t r = (((hex >> 16) & 0xFF) * scale + 127) / 255;
  uint8_t g = (((hex >> 8) & 0xFF) * scale + 127) / 255;
  uint8_t b = ((hex & 0xFF) * scale + 127) / 255;

  if (s.useGammaCorrection) {
    r = Adafruit_NeoPixel::gamma8(r);
    g = Adafruit_NeoPixel::gamma8(g);
    b = Adafruit_NeoPixel::gamma8(b);
  }

  // ---------- RGBW PATH ----------
  if (s.isRGBW) {

    uint8_t minRGB = min(r, min(g, b));

    // only extract white for near-white values
    if (minRGB > 225) {

      uint8_t w = minRGB;   // no need for ternary

      r -= w;
      g -= w;
      b -= w;

      return s.strip.Color(r, g, b, w);
    }
  }

  // ---------- RGB PATH ----------
  return s.strip.Color(r, g, b);
}

uint32_t RGBToHex(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) |
         ((uint32_t)g << 8)  |
         b;
}

// ----------------- REFERENCE FUNCTIONS HOPEFULLY NOT NEEDED ---------------------------
#if INCLUDE_REFERENCE_FUNCTIONS == 1
// uint32_t hexColor = 0xFF3366; // R=255, G=51, B=102
// uint8_t r, g, b;
// hexToRGB(hex, r, g, b);
void hexToRGB(uint32_t color, uint8_t &r, uint8_t &g, uint8_t &b) {
  r = (color >> 16) & 0xFF;
  g = (color >> 8) & 0xFF;
  b = color & 0xFF;
}

//uint32_t hexColor = 0xFF3366; // R=255, G=51, B=102
//uint8_t r, g, b, w;
//hexToRGB(hex, r, g, b, w);
void hexToRGBW(uint32_t color, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &w) {
  r = (color >> 16) & 0xFF;
  g = (color >> 8) & 0xFF;
  b = color & 0xFF;

  // Pull out shared brightness into white channel
  w = min(r, min(g, b));

  r -= w;
  g -= w;
  b -= w;
}

// strip.setPixelColor(0, hexColor(strip, 0xFFEEAA));
uint32_t hexColorRGB(Adafruit_NeoPixel &s, uint32_t hex) {
  uint8_t r = (hex >> 16) & 0xFF;
  uint8_t g = (hex >> 8) & 0xFF;
  uint8_t b = hex & 0xFF;

  return s.Color(r, g, b);
}

// strip.setPixelColor(0, hexColorW(strip, 0xFFEEAA));
uint32_t hexColorRGBW(Adafruit_NeoPixel &s, uint32_t hex) {
  uint8_t r = (hex >> 16) & 0xFF;
  uint8_t g = (hex >> 8) & 0xFF;
  uint8_t b = hex & 0xFF;

  uint8_t w = min(r, min(g, b));
  r -= w; g -= w; b -= w;

  return s.Color(r, g, b, w);
}
#endif
