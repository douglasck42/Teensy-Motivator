// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later
// Heavy AI here, I've been switching between Claude, ChatGPT and various local LLMs at the time and don't recall which
// I'd presume Heavily derived from the bolderflight sbus library, MIT licensed: https://github.com/bolderflight/sbus
// I'd also credit Tim Hebel's sbus-24 contributions found here: https://github.com/Eebel/MyActuatorRMDX68V3_SBUS24
//      I can't say for sure that credit IS his, but that's the SBUS-24 packet implementation that WAS based on the Bolderflight SBUS-16 code

#pragma once
#include <Arduino.h>

// SBUS-24 protocol constants
// Frame: 1 header + 34 payload + 1 footer = 36 bytes
// 24 channels x 11 bits = 264 bits = 33 bytes, in three 8-channel blocks
// payload[0..10]  = CH1-CH8
// payload[11..21] = CH9-CH16
// payload[22..32] = CH17-CH24
// payload[33]     = flags
static constexpr uint8_t  SBUS_HEADER          = 0x0F;
static constexpr uint8_t  SBUS_FOOTER          = 0x00;
static constexpr uint8_t  SBUS2_FOOTER_MASK    = 0x0F;
static constexpr uint8_t  SBUS2_FOOTER_VAL     = 0x04;
static constexpr uint8_t  SBUS_FRAME_LEN       = 36;
static constexpr uint8_t  SBUS_PAYLOAD_LEN     = 34;
static constexpr uint32_t SBUS_BAUD            = 100000;
static constexpr uint32_t SBUS_TIMEOUT_US      = 7000;
static constexpr uint32_t SBUS_FRAME_GAP_MS    = 4;

// Flags byte bit masks (payload[33])
static constexpr uint8_t  SBUS_FLAG_LOST_FRAME = 0x04;
static constexpr uint8_t  SBUS_FLAG_FAILSAFE   = 0x08;

class Sbus24Native {
public:
    Sbus24Native(HardwareSerial* serial);
    void begin();
    bool read();
    void analyze();
    void analyze_timegap();
    void analyze_startstop();
    int16_t channel(uint8_t index) const;
    bool failsafe() const;
    bool lostFrame() const;

private:
    HardwareSerial* _serial;
    uint8_t  _payload[SBUS_PAYLOAD_LEN]; // parsed payload (excludes header/footer)
    uint8_t  _buf[128];                  // raw buffer for analyze methods
    uint8_t  _bufIdx;
    uint8_t  _parserState;
    uint8_t  _prevByte;
    int16_t  _channels[24];
    bool     _failsafe;
    bool     _lostFrame;
    uint32_t _lastByteMs;

    bool parseFrame();
};