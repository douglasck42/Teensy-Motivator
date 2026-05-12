// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later
// Built with Anthropic Claude
//
// scomp_serial.h — Framing layer over any Arduino Stream (HardwareSerial, etc.).
// Uses Stream* so it is portable between Teensy and ESP32 Arduino cores.

#pragma once
#include <Arduino.h>
#include "scomp_protocol.h"

// Callback signature: called once per complete, CRC-valid message received.
typedef void (*ScompCallback)(uint8_t msg_type, const uint8_t *payload, uint8_t len, void *user_data);

class ScompSerial {
public:
    // Pass a pre-opened Stream; caller is responsible for Serial5.begin() etc.
    void begin(Stream &port);

    // Call every loop() iteration — drives the receive state machine.
    void update();

    // Register the inbound message handler.
    void onMessage(ScompCallback cb, void *user_data = nullptr);

    // ---- Generic sender (all others delegate here) ----
    bool send(uint8_t msg_type, const void *payload, uint8_t len);

    // ---- RX diagnostics ----
    uint32_t rxBytes()      const;
    uint32_t rxFrames()     const;
    uint32_t rxCrcErrors()  const;
    uint32_t rxSyncDrops()  const;  // bytes discarded waiting for 0xAB start byte

    // ---- Teensy → ESP32 convenience senders ----
    bool sendHeartbeat(const ScompHeartbeat &msg);
    bool sendInputChannels(const ScompInputChannels &msg);
    bool sendOutputChannels(const ScompOutputChannels &msg);
    bool sendAudioState(const ScompAudioState &msg);
    bool sendLedTrigger(const ScompLedTrigger &msg);

    // ---- ESP32 → Teensy convenience senders ----
    bool sendRequestState();
    bool sendSetVolume(const ScompSetVolume &msg);
    bool sendTriggerAudio(const ScompTriggerAudio &msg);
    bool sendSetSetting(const ScompSetSetting &msg);

private:
    enum class RxState : uint8_t {
        WAIT_START,
        READ_TYPE,
        READ_LEN,
        READ_PAYLOAD,
        READ_CRC,
        READ_END,
    };

    Stream          *_port      = nullptr;
    ScompCallback  _cb        = nullptr;
    void            *_cb_data   = nullptr;

    RxState  _rx_state = RxState::WAIT_START;
    uint8_t  _rx_type  = 0;
    uint8_t  _rx_len   = 0;
    uint8_t  _rx_pos   = 0;
    uint8_t  _rx_buf[SCOMP_MAX_PAYLOAD];

    uint32_t _rx_bytes      = 0;
    uint32_t _rx_frames     = 0;
    uint32_t _rx_crc_errors = 0;
    uint32_t _rx_sync_drops = 0;
};
