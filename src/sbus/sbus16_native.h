// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later
// Heavy AI here, I've been switching between Claude, ChatGPT and various local LLMs at the time and don't recall which
// I'd presume Heavily derived from the bolderflight sbus library, MIT licensed: https://github.com/bolderflight/sbus

#pragma once
#include <Arduino.h>

// SBUS protocol constants
static constexpr uint8_t  SBUS_STARTBYTE     = 0x0F;
static constexpr uint8_t  SBUS_ENDBYTE       = 0x00;
static constexpr uint8_t  SBUS_FRAME_LEN     = 25;
static constexpr uint32_t SBUS_BAUD          = 100000;
static constexpr uint32_t SBUS_FRAME_GAP_MS  = 4;    // Gap between frames

class Sbus16Native {
public:
    Sbus16Native(HardwareSerial* serial);
    void begin();
    bool read();
    int16_t channel(uint8_t index) const;
    bool failsafe() const;
    bool lostFrame() const;

private:
    HardwareSerial* _serial;
    uint8_t  _buf[SBUS_FRAME_LEN];
    uint8_t  _bufIdx;
    int16_t  _channels[16];
    bool     _failsafe;
    bool     _lostFrame;
    uint32_t _lastByteMs;

    bool parseFrame();
};