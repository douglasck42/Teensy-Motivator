// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <Arduino.h>
#include "sbus.h"

class SbusHandler {
public:
    SbusHandler(HardwareSerial* serial);
    void begin();
    bool read();
    int16_t channel(uint8_t index) const;
    bool failsafe() const;
    bool lostFrame() const;

private:
    bfs::SbusRx _sbus;
    bfs::SbusData _data;
};