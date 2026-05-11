// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later
// Heavy AI here, I've been switching between Claude, ChatGPT and various local LLMs at the time and don't recall which
// I'd presume Heavily derived from the bolderflight sbus library, MIT licensed: https://github.com/bolderflight/sbus

#include "sbus16_native.h"
#include "elapsedMillis.h"

Sbus16Native::Sbus16Native(HardwareSerial* serial)
    : _serial(serial), _bufIdx(0),
      _failsafe(false), _lostFrame(false), _lastByteMs(0) {
    memset(_channels, 0, sizeof(_channels));
    memset(_buf, 0, sizeof(_buf));
}

void Sbus16Native::begin() {
    // SBUS: 100000 baud, 8E2, inverted
#if defined(ARDUINO_ARCH_ESP32)
    _serial->begin(SBUS_BAUD, SERIAL_8E2, SBUS_RX_PIN, SBUS_TX_PIN, true);
#else
    // Teensy hardware serial supports SERIAL_8E2 with inversion constant
    _serial->begin(SBUS_BAUD, SERIAL_8E2_RXINV);
#endif
}

bool Sbus16Native::read() {
    bool gotFrame = false;

    while (_serial->available()) {
        uint8_t byte = _serial->read();
        uint32_t now = millis();

        // If gap since last byte exceeds threshold, reset frame sync
        if (_bufIdx > 0 && (now - _lastByteMs) > SBUS_FRAME_GAP_MS) {
            _bufIdx = 0;
        }
        _lastByteMs = now;

        // Wait for start byte at position 0
        if (_bufIdx == 0 && byte != SBUS_STARTBYTE) {
            continue;
        }

        _buf[_bufIdx++] = byte;

        // Once we have a full frame, parse it
        if (_bufIdx == SBUS_FRAME_LEN) {
            _bufIdx = 0;
            if (_buf[24] == SBUS_ENDBYTE) {
                if (parseFrame()) gotFrame = true;
            }
        }
    }

    return gotFrame;
}

// ...existing code...
bool Sbus16Native::parseFrame() {
    // Unpack 16 x 11-bit channels from bytes 1-22 using a generic bit-offset method
    uint8_t *p = _buf + 1; // data starts at _buf[1]
    for (uint8_t i = 0; i < 16; ++i) {
        uint32_t bitOffset = i * 11;
        uint16_t byteIndex = bitOffset / 8;
        uint16_t bitIndex = bitOffset % 8;

        // Read 3 bytes to cover the 11 bits spanning byte boundaries
        uint32_t raw = (uint32_t)p[byteIndex]
                     | ((uint32_t)p[byteIndex + 1] << 8)
                     | ((uint32_t)p[byteIndex + 2] << 16);

        _channels[i] = (int16_t)((raw >> bitIndex) & 0x07FF);
    }

    // Byte 23 contains flags (within _buf: index 23)
    _lostFrame = (_buf[23] & (1 << 5));
    _failsafe  = (_buf[23] & (1 << 4));

    return true;
}

int16_t Sbus16Native::channel(uint8_t index) const {
    if (index < 16) return _channels[index];
    return 0;
}

bool Sbus16Native::failsafe() const { return _failsafe; }
bool Sbus16Native::lostFrame() const { return _lostFrame; }