// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later
//
// scomp_link.h — Serial communication layer between Teensy (Motivator) and ESP32 (Sparkle Motion).
// Built on top of SerialTransfer (PowerBroker2).
//
// Copy scomp_link.h + scomp_link.cpp to both repos.
// Only the port and node flag passed to begin() differ between sides:
//   Teensy:  scomp.begin(Serial5, SCOMP_FLAG_NODE_LOCAL);
//   ESP32:   scomp.begin(Serial2, SCOMP_FLAG_NODE_REMOTE);

#pragma once
#include <Arduino.h>
#include <SerialTransfer.h>

// not sure these are used anymore:
//#ifndef DEBUG_SCOMP_RX
//#define DEBUG_SCOMP_RX 0
//#endif
//#ifndef SCOMP_BAUD_RATE
//#define SCOMP_BAUD_RATE 115200
//#endif

// ---- Timing defaults — override via build flags before this include ----
#ifndef HEARTBEAT_INTERVAL_MS
#define HEARTBEAT_INTERVAL_MS 5000
#endif
#ifndef SCOMP_DEADZONE_MS
#define SCOMP_DEADZONE_MS (HEARTBEAT_INTERVAL_MS * 3)
#endif
#ifndef SCOMP_SEND_INTERVAL_MS
#define SCOMP_SEND_INTERVAL_MS 50      // 20 Hz channel updates
#endif

// ---- Node identity — each side stamps its own flag in outgoing heartbeats ----
#ifndef SCOMP_FLAG_NODE_LOCAL
#define SCOMP_FLAG_NODE_LOCAL  1       // Teensy (physical/SBUS side)
#endif
#ifndef SCOMP_FLAG_NODE_REMOTE
#define SCOMP_FLAG_NODE_REMOTE 2       // ESP32 (LED/wireless side)
#endif

// ---- Channel counts — match NUM_INPUT/OUTPUT_CHANNELS in settings.h ----
#ifndef SCOMP_IN_CH
#define SCOMP_IN_CH  24
#endif
#ifndef SCOMP_OUT_CH
#define SCOMP_OUT_CH 48
#endif

// ---- Message IDs (first byte of every SerialTransfer payload) ----
#define SCOMP_MSG_HEARTBEAT    0x01
#define SCOMP_MSG_LED_TRIGGER  0x02
#define SCOMP_MSG_IN_CHANNELS  0x03
#define SCOMP_MSG_OUT_CHANNELS 0x04
#define SCOMP_MSG_STRIP_CONFIG 0x05

// ============================================================
// Payload structs — packed, little-endian (ARM + Xtensa both LE)
// ============================================================

struct __attribute__((packed)) ScompHeartbeat {
    uint8_t  node_flag;    // SCOMP_FLAG_NODE_LOCAL or SCOMP_FLAG_NODE_REMOTE
    uint32_t uptime_ms;
};

struct __attribute__((packed)) ScompLedTrigger {
    uint8_t board_id;
    uint8_t strip_id;
    uint8_t group_id;
    uint8_t animation_id;
};

struct __attribute__((packed)) ScompInputChannels {
    uint16_t us[SCOMP_IN_CH];     // µs values for all input channels
};

struct __attribute__((packed)) ScompOutputChannels {
    uint16_t us[SCOMP_OUT_CH];    // µs values for all output channels
};

// ---- Strip type indices — must match STRIP_TYPES[] table on the ESP32 side ----
#define SCOMP_STRIP_GRB_800  0   // WS2812B (most common RGB)
#define SCOMP_STRIP_RGB_800  1
#define SCOMP_STRIP_GRBW_800 2   // SK6812 RGBW
#define SCOMP_STRIP_RGBW_800 3
#define SCOMP_STRIP_GRB_400  4
#define SCOMP_STRIP_RGB_400  5

struct __attribute__((packed)) ScompStripConfig {
    uint16_t strip1_num_leds;
    uint8_t  strip1_pin;
    uint8_t  strip1_type_index;  // one of SCOMP_STRIP_* above
    uint8_t  strip1_brightness;  // 0–255
    uint16_t strip2_num_leds;
    uint8_t  strip2_pin;
    uint8_t  strip2_type_index;
    uint8_t  strip2_brightness;
};

// ---- Callback types ----
typedef void (*ScompHeartbeatCb)  (const ScompHeartbeat &);
typedef void (*ScompLedTriggerCb) (const ScompLedTrigger &);
typedef void (*ScompInChannelsCb) (const ScompInputChannels &);
typedef void (*ScompOutChannelsCb)(const ScompOutputChannels &);
typedef void (*ScompStripConfigCb)(const ScompStripConfig &);

// ============================================================
// ScompLink — thin wrapper around SerialTransfer
// ============================================================

// formatting helps
char* formatUptime(unsigned long ms);

class ScompLink {
public:
    // Call once in setup(). Port and node flag are the only things that
    // differ between the Teensy and ESP32 builds.
    void begin(Stream &port, uint8_t my_node_flag);

    // Drive the receive state machine — call every loop() tick.
    void update();

    // ---- Transmit ----
    void sendHeartbeat(uint32_t uptime_ms);
    void sendLedTrigger(const ScompLedTrigger &msg);
    void sendInputChannels(const ScompInputChannels &msg);
    void sendOutputChannels(const ScompOutputChannels &msg);
    void sendStripConfig(const ScompStripConfig &msg);

    // ---- Receive callbacks ----
    void onHeartbeat    (ScompHeartbeatCb   cb) { _cb_hb  = cb; }
    void onLedTrigger   (ScompLedTriggerCb  cb) { _cb_led = cb; }
    void onInputChannels (ScompInChannelsCb  cb) { _cb_in  = cb; }
    void onOutputChannels(ScompOutChannelsCb cb) { _cb_out = cb; }
    void onStripConfig  (ScompStripConfigCb  cb) { _cb_cfg = cb; }

    // Returns true if a peer heartbeat arrived within SCOMP_DEADZONE_MS.
    bool peerAlive() const;

    // Print local/remote UP|DOWN status to Serial. Safe to call any time.
    void printHeartbeat();


    // RX diagnostics
    uint32_t rxFrames() const { return _rx_frames; }
    uint32_t rxErrors() const { return _rx_errors; }

private:
    SerialTransfer     _xfer;
    uint8_t            _my_flag        = 0;
    uint32_t           _peer_last_ms   = 0;
    uint8_t            _peer_node_flag = 0;
    uint32_t           _peer_uptime_ms = 0;
    uint32_t           _rx_frames      = 0;
    uint32_t           _rx_errors      = 0;

    ScompHeartbeatCb   _cb_hb  = nullptr;
    ScompLedTriggerCb  _cb_led = nullptr;
    ScompInChannelsCb  _cb_in  = nullptr;
    ScompOutChannelsCb _cb_out = nullptr;
    ScompStripConfigCb _cb_cfg = nullptr;

    void _dispatch(uint8_t msg_type);
};
