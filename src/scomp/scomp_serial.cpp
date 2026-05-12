// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later
// Built with Anthropic Claude

#include "scomp_serial.h"

// ============================================================
// Init
// ============================================================

void ScompSerial::begin(Stream &port) {
    _port     = &port;
    _rx_state = RxState::WAIT_START;
}

void ScompSerial::onMessage(ScompCallback cb, void *user_data) {
    _cb      = cb;
    _cb_data = user_data;
}

// ============================================================
// Receive — state machine, call from loop()
// ============================================================

void ScompSerial::update() {
    if (!_port) return;

    while (_port->available()) {
        uint8_t b = (uint8_t)_port->read();
        _rx_bytes++;

        switch (_rx_state) {
            case RxState::WAIT_START:
                if (b == SCOMP_START) _rx_state = RxState::READ_TYPE;
                else _rx_sync_drops++;
                break;

            case RxState::READ_TYPE:
                _rx_type  = b;
                _rx_state = RxState::READ_LEN_LO;
                break;

            case RxState::READ_LEN_LO:
                _rx_len   = b;               // low byte
                _rx_state = RxState::READ_LEN_HI;
                break;

            case RxState::READ_LEN_HI:
                _rx_len  |= ((uint16_t)b << 8); // high byte
                _rx_pos   = 0;
                _rx_state = (_rx_len == 0) ? RxState::READ_CRC : RxState::READ_PAYLOAD;
                break;

            case RxState::READ_PAYLOAD:
                if (_rx_pos < SCOMP_MAX_PAYLOAD) _rx_buf[_rx_pos] = b;
                if (++_rx_pos >= _rx_len) _rx_state = RxState::READ_CRC;
                break;

            case RxState::READ_CRC: {
                // Validate CRC over [type, len_lo, len_hi, payload...]
                uint8_t len_lo = (uint8_t)(_rx_len & 0xFF);
                uint8_t len_hi = (uint8_t)(_rx_len >> 8);
                uint8_t crc = scomp_crc8_feed(0,   &_rx_type, 1);
                crc         = scomp_crc8_feed(crc, &len_lo,   1);
                crc         = scomp_crc8_feed(crc, &len_hi,   1);
                crc         = scomp_crc8_feed(crc, _rx_buf, _rx_len);
                if (b == crc) {
                    _rx_state = RxState::READ_END;
                } else {
                    _rx_crc_errors++;
                    _rx_state = RxState::WAIT_START; // CRC mismatch — discard
                }
                break;
            }

            case RxState::READ_END:
                if (b == SCOMP_END) {
                    _rx_frames++;
                    if (_cb) _cb(_rx_type, _rx_buf, _rx_len, _cb_data);
                }
                _rx_state = RxState::WAIT_START;
                break;
        }
    }
}

// ============================================================
// Transmit
// ============================================================

bool ScompSerial::send(uint8_t msg_type, const void *payload, uint16_t len) {
    if (!_port) return false;

    uint8_t len_lo = (uint8_t)(len & 0xFF);
    uint8_t len_hi = (uint8_t)(len >> 8);
    uint8_t crc = scomp_crc8_feed(0,   &msg_type, 1);
    crc         = scomp_crc8_feed(crc, &len_lo,   1);
    crc         = scomp_crc8_feed(crc, &len_hi,   1);
    crc         = scomp_crc8_feed(crc, (const uint8_t *)payload, len);

    _port->write(SCOMP_START);
    _port->write(msg_type);
    _port->write(len_lo);
    _port->write(len_hi);
    if (len) _port->write((const uint8_t *)payload, len);
    _port->write(crc);
    _port->write(SCOMP_END);
    return true;
}

// ---- Teensy → ESP32 ----

bool ScompSerial::sendHeartbeat(const ScompHeartbeat &msg) {
    return send(SCOMP_MSG_HEARTBEAT, &msg, sizeof(msg));
}
bool ScompSerial::sendInputChannels(const ScompInputChannels &msg) {
    return send(SCOMP_MSG_INPUT_CHANNELS, &msg, sizeof(msg));
}
bool ScompSerial::sendOutputChannels(const ScompOutputChannels &msg) {
    return send(SCOMP_MSG_OUTPUT_CHANNELS, &msg, sizeof(msg));
}
bool ScompSerial::sendAudioState(const ScompAudioState &msg) {
    return send(SCOMP_MSG_AUDIO_STATE, &msg, sizeof(msg));
}
bool ScompSerial::sendLedTrigger(const ScompLedTrigger &msg) {
    return send(SCOMP_MSG_LED_TRIGGER, &msg, sizeof(msg));
}

// ---- ESP32 → Teensy ----

bool ScompSerial::sendRequestState() {
    return send(SCOMP_MSG_REQUEST_STATE, nullptr, 0);
}
bool ScompSerial::sendSetVolume(const ScompSetVolume &msg) {
    return send(SCOMP_MSG_SET_VOLUME, &msg, sizeof(msg));
}
bool ScompSerial::sendTriggerAudio(const ScompTriggerAudio &msg) {
    return send(SCOMP_MSG_TRIGGER_AUDIO, &msg, sizeof(msg));
}
bool ScompSerial::sendSetSetting(const ScompSetSetting &msg) {
    return send(SCOMP_MSG_SET_SETTING, &msg, sizeof(msg));
}

// ---- RX diagnostics ----

uint32_t ScompSerial::rxBytes()     const { return _rx_bytes; }
uint32_t ScompSerial::rxFrames()    const { return _rx_frames; }
uint32_t ScompSerial::rxCrcErrors() const { return _rx_crc_errors; }
uint32_t ScompSerial::rxSyncDrops() const { return _rx_sync_drops; }
