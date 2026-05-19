#include <Arduino.h>
#include "common/settings.h"
#include "common/common.h"
#include <Adafruit_NeoPixel.h>
#include "teensy_led.h"

#ifndef NUM_LEDS_STRIP1
#define NUM_LEDS_STRIP1 2
#endif

#define FILL_TEST_STRIP 1

//static Adafruit_NeoPixel strip1(NUM_LEDS_STRIP1, LED_BANK1_PIN, NEO_GRBW + NEO_KHZ800);
static Adafruit_NeoPixel strip1(NUM_LEDS_STRIP1, LED_BANK1_PIN, NEO_GRB + NEO_KHZ800);

void teensy_led_setup() {
    strip1.begin();
    strip1.setBrightness(64);
    strip1.clear();
    strip1.show();
}

void teensy_led_loop() {
    static unsigned long last_ms = 0;
    static uint8_t phase = 0;
    unsigned long now = millis();

    #if FILL_TEST_STRIP == 1
    if (now - last_ms < 2000) return;
    last_ms = now;

    switch (phase) {
        case 0: fillStrip(strip1, 255,   0,   0,   0); 
            Serial.println("LED Test: Red");
            break;  // red
        case 1: fillStrip(strip1,   0, 255,   0,   0);
            Serial.println("LED Test: Green");
            break;  // green
        case 2: fillStrip(strip1,   0,   0, 255,   0); 
            Serial.println("LED Test: Blue");
            break;  // blue
        case 3: fillStrip(strip1,   0,   0,   0, 255); 
            Serial.println("LED Test: White");
            break;  // white (W channel)
        case 4: fillStrip(strip1, 255, 255, 255,   0); 
            Serial.println("LED Test: White (RGB mix)");
            break;  // white (RGB mix)
    }
    if (++phase > 4) phase = 0;
    #endif
}

void fillStrip(Adafruit_NeoPixel &strip, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    uint32_t color = strip.Color(r, g, b, w);
    for (int i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, color);
    }
    strip.show();
}
