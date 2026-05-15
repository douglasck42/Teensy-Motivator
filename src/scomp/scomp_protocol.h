// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later
// Built with Anthropic Claude
//
// scomp_protocol.h — Wire protocol shared between Teensy 4.1 and ESP32 Scomp Motion.
// No Arduino or platform dependencies; copy this file as-is to both projects.
//
// Frame layout:  [START:1][TYPE:1][LEN_LO:1][LEN_HI:1][PAYLOAD:LEN][CRC8:1][END:1]
// CRC-8 covers bytes:  TYPE + LEN_LO + LEN_HI + PAYLOAD  (Dallas/Maxim, poly 0x8C)
// LEN is little-endian uint16_t, allowing payloads up to 65535 bytes.

#pragma once
#include <stdint.h>

// ---- Protocol version — bump on any breaking change ----
#define SCOMP_PROTOCOL_VERSION  2

// ---- Frame constants ----
#define SCOMP_START        0xAB
#define SCOMP_END          0xCD
#define SCOMP_MAX_PAYLOAD  4096  // uint16_t length field; sized for a full JSON config

// ---- Channel counts (must match settings.h NUM_INPUT/OUTPUT_CHANNELS) ----
#define SCOMP_IN_CH   24
#define SCOMP_OUT_CH  48

// ============================================================
// Message type IDs
// ============================================================

// Teensy → ESP32
#define SCOMP_MSG_HEARTBEAT        0x01  // periodic keep-alive + SBUS status
#define SCOMP_MSG_INPUT_CHANNELS   0x02  // all input channel SBUS values
#define SCOMP_MSG_OUTPUT_CHANNELS  0x03  // all output channel µs values
#define SCOMP_MSG_AUDIO_STATE      0x04  // volume + playing state
#define SCOMP_MSG_LED_TRIGGER      0x05  // request an LED animation on the ESP32

// ESP32 → Teensy
#define SCOMP_MSG_REQUEST_STATE    0x81  // ask Teensy to send a full state dump
#define SCOMP_MSG_SET_VOLUME       0x82  // set DFPlayer volume
#define SCOMP_MSG_TRIGGER_AUDIO    0x83  // play a specific audio file
#define SCOMP_MSG_SET_SETTING      0x84  // generic key/value setting change

// ---- ScompHeartbeat.flags bitmask ----
#define SCOMP_FLAG_SBUS_FAILSAFE    (1u << 0)
#define SCOMP_FLAG_SBUS_LOST_FRAME  (1u << 1)

// ---- ScompAudioState.state values ----
#define SCOMP_AUDIO_STOPPED  0
#define SCOMP_AUDIO_PLAYING  1

// ---- ScompLedTrigger.animation_id values ----
#define SCOMP_LED_OFF       0x00
#define SCOMP_LED_SOLID     0x01
#define SCOMP_LED_BREATHE   0x02
#define SCOMP_LED_BLINK     0x03
#define SCOMP_LED_RAINBOW   0x04
#define SCOMP_LED_FAILSAFE  0x10  // convention: red blink for SBUS failsafe
#define SCOMP_LED_AUDIO     0x11  // convention: audio-reactive animation

// ---- ScompSetSetting.key values ----
#define SCOMP_SETTING_AUDIO_VOLUME  0x01  // value.u8  = new volume (0-30)
#define SCOMP_SETTING_AUDIO_MUTE    0x02  // value.u8  = 0/1
#define SCOMP_SETTING_DEBUG         0x10  // value.u8  = 0/1

// ============================================================
// Payload structs — all packed, little-endian (ARM + Xtensa both LE)
// ============================================================

// 0x01  Teensy → ESP32  (6 bytes)
typedef struct __attribute__((packed)) {
    uint8_t  version;    // SCOMP_PROTOCOL_VERSION
    uint32_t uptime_ms;
    uint8_t  flags;      // SCOMP_FLAG_* bitmask
} ScompHeartbeat;

// 0x02  Teensy → ESP32  (51 bytes)
typedef struct __attribute__((packed)) {
    uint16_t values[SCOMP_IN_CH];  // raw SBUS values (172-1810 typical)
    uint8_t  enabled[3];             // bit N set → channel N is enabled
} ScompInputChannels;

// 0x03  Teensy → ESP32  (102 bytes)
typedef struct __attribute__((packed)) {
    uint16_t values[SCOMP_OUT_CH]; // pulse width in µs
    uint8_t  enabled[6];             // bit N set → channel N is enabled
} ScompOutputChannels;

// 0x04  Teensy → ESP32  (4 bytes)
typedef struct __attribute__((packed)) {
    uint8_t  volume;       // 0-30
    uint8_t  state;        // SCOMP_AUDIO_STOPPED / SCOMP_AUDIO_PLAYING
    uint16_t file_number;
} ScompAudioState;

// 0x05  Teensy → ESP32  (5 bytes)
typedef struct __attribute__((packed)) {
    uint8_t animation_id;  // SCOMP_LED_* constant
    uint8_t r, g, b;       // colour hint (animation may ignore)
    uint8_t brightness;    // 0-255
} ScompLedTrigger;

// 0x82  ESP32 → Teensy  (1 byte)
typedef struct __attribute__((packed)) {
    uint8_t volume;        // 0-30
} ScompSetVolume;

// 0x83  ESP32 → Teensy  (2 bytes)
typedef struct __attribute__((packed)) {
    uint16_t file_number;
} ScompTriggerAudio;

// 0x84  ESP32 → Teensy  (6 bytes)
typedef struct __attribute__((packed)) {
    uint8_t key;           // SCOMP_SETTING_* constant
    uint8_t index;         // sub-index (e.g., channel number); 0 if unused
    uint8_t value_type;    // 0=uint8, 1=uint16, 2=bool
    uint8_t _pad;
    union {
        uint8_t  u8;
        uint16_t u16;
    } value;
} ScompSetSetting;

// ============================================================
// CRC-8  (Dallas/Maxim, polynomial 0x8C reflected)
// ============================================================

static inline uint8_t scomp_crc8_feed(uint8_t crc, const uint8_t *data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        uint8_t b = data[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            crc = ((crc ^ b) & 1u) ? (uint8_t)((crc >> 1) ^ 0x8Cu) : (uint8_t)(crc >> 1);
            b >>= 1;
        }
    }
    return crc;
}

static inline uint8_t scomp_crc8(const uint8_t *data, uint8_t len) {
    return scomp_crc8_feed(0, data, len);
}
