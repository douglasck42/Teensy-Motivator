#pragma once
#include <Arduino.h>

#ifndef LED_ENABLED
LED_ENABLED=0
#endif
#ifndef LED_BANK1_PIN
LED_BANK1_PIN=40
#endif

#include <Adafruit_NeoPixel.h>

// struct StripTypeDef { const char* name; neoPixelType type; };
// static const StripTypeDef STRIP_TYPES[] = {
//     { "GRB_800",  NEO_GRB  + NEO_KHZ800 },   // SCOMP_STRIP_GRB_800  — WS2812B
//     { "RGB_800",  NEO_RGB  + NEO_KHZ800 },   // SCOMP_STRIP_RGB_800
//     { "GRBW_800", NEO_GRBW + NEO_KHZ800 },   // SCOMP_STRIP_GRBW_800 — SK6812
//     { "RGBW_800", NEO_RGBW + NEO_KHZ800 },   // SCOMP_STRIP_RGBW_800
//     { "GRB_400",  NEO_GRB  + NEO_KHZ400 },   // SCOMP_STRIP_GRB_400
//     { "RGB_400",  NEO_RGB  + NEO_KHZ400 },   // SCOMP_STRIP_RGB_400
// };
// static const uint8_t STRIP_TYPES_COUNT = sizeof(STRIP_TYPES) / sizeof(STRIP_TYPES[0]);

void teensy_led_setup();
void teensy_led_loop();
void fillStrip(Adafruit_NeoPixel &strip, uint8_t r, uint8_t g, uint8_t b, uint8_t w);
