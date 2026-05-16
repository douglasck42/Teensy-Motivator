// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scomp_link.h"

void ScompLink::begin(Stream &port, uint8_t my_node_flag) {
    _xfer.begin(port, true);  // false = disable SerialTransfer's own debug printing
    _my_flag = my_node_flag;
}


// ============================================================
// update() — call every loop() tick
// ============================================================
void ScompLink::update() {
    while (true) {
        uint8_t bytesRead = _xfer.available();
        if (bytesRead > 0) {
            _rx_frames++;
            uint8_t msg_type = 0;
            _xfer.rxObj(msg_type, 0);
            _dispatch(msg_type);
        } else if (_xfer.status < -1) {
            // actual framing/CRC error — reset() already flushed the buffer inside available()
            _rx_errors++;
#if defined(DEBUG_SCOMP_RX) && DEBUG_SCOMP_RX
            Serial.printf("[SCOMP RX] error status=%d\n", _xfer.status);
#endif
            break;
        } else {
            break;  // NO_DATA (0) or CONTINUE (-1, mid-packet): check again next tick
        }
    }
}


// ============================================================
// Formatting
// ============================================================
char* formatUptime(unsigned long ms) {
    static char buf[32];
    unsigned long seconds = ms / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours   = minutes / 60;
    unsigned long days    = hours   / 24;

    seconds %= 60;
    minutes %= 60;
    hours   %= 24;

    if (days > 0)
        sprintf(buf, "%lud %02luh %02lum %02lus", days, hours, minutes, seconds);
    else if (hours > 0)
        sprintf(buf, "%02luh %02lum %02lus", hours, minutes, seconds);
    else if (minutes > 0)
        sprintf(buf, "%02lum %02lus", minutes, seconds);
    else
        sprintf(buf, "%02lus", seconds);

    return buf;
}

void ScompLink::printHeartbeat() {
    // Print out heartbeat status in a human-readable format. This is useful for debugging and monitoring the connection status between the local and remote nodes. It shows the uptime of both nodes and how long ago we last heard from the peer.
    // ESP32 doesn't like formatUptime with printf, so it's been relgated to it's own line
    uint32_t now = millis();
    static uint32_t last_print_ms = 0;
    if (now - last_print_ms < (HEARTBEAT_INTERVAL_MS - 100)) {
        return; // rate limit to avoid spamming Serial
    }
    last_print_ms = now;
    Serial.printf("[SCOMP] Local Node 0x%02X UP ", SCOMP_FLAG_NODE_LOCAL);
    Serial.print(formatUptime(now));
    if (peerAlive()) {
        Serial.printf(" | Remote Node 0x%02X UP ", _peer_node_flag);
        Serial.print(formatUptime(_peer_uptime_ms));
    } else {
        Serial.printf(" | Remote Node 0x%02X DOWN", _peer_node_flag);
    }
    Serial.print(" | gap ");
    Serial.print(formatUptime(now - _peer_last_ms)); 
    #if DEBUG_SCOMP_RX
    Serial.printf(" | frames=%lu errors=%lu", rxFrames(), rxErrors());
    #endif
    Serial.print("\n");
}
// ============================================================
// _dispatch() — route a received frame to the right handler
// ============================================================
void ScompLink::_dispatch(uint8_t msg_type) {
    const uint16_t off = sizeof(uint8_t);   // payload starts after the type byte

    switch (msg_type) {

        case SCOMP_MSG_HEARTBEAT: {
            ScompHeartbeat hb;
            _xfer.rxObj(hb, off);
            if (hb.node_flag == _my_flag) return;   // loopback guard — discard own echo
            _peer_last_ms   = millis();
            _peer_node_flag = hb.node_flag;
            _peer_uptime_ms = hb.uptime_ms;
            printHeartbeat();
            if (_cb_hb) _cb_hb(hb);
            break;
        }

        case SCOMP_MSG_LED_TRIGGER: {
            ScompLedTrigger msg;
            _xfer.rxObj(msg, off);
            Serial.printf("[SCOMP LED] board=%u strip=%u group=%u anim=%u\n",
                          msg.board_id, msg.strip_id, msg.group_id, msg.animation_id);
            if (_cb_led) _cb_led(msg);
            break;
        }

        case SCOMP_MSG_IN_CHANNELS: {
            ScompInputChannels msg;
            _xfer.rxObj(msg, off);
            if (_cb_in) _cb_in(msg);
            break;
        }

        case SCOMP_MSG_OUT_CHANNELS: {
            ScompOutputChannels msg;
            _xfer.rxObj(msg, off);
            if (_cb_out) _cb_out(msg);
            break;
        }

        case SCOMP_MSG_STRIP_CONFIG: {
            ScompStripConfig msg;
            _xfer.rxObj(msg, off);
            if (_cb_cfg) _cb_cfg(msg);
            break;
        }

        default:
            Serial.printf("[SCOMP RX] unknown msg type 0x%02X\n", msg_type);
            break;
    }
}

// ============================================================
// peerAlive() — true if we heard from the peer recently
// ============================================================
bool ScompLink::peerAlive() const {
    if (_peer_last_ms == 0) return false;
    return (millis() - _peer_last_ms) < SCOMP_DEADZONE_MS;
}

// ============================================================
// Send helpers
// ============================================================
void ScompLink::sendHeartbeat(uint32_t uptime_ms) {
    uint8_t msg_type = SCOMP_MSG_HEARTBEAT;
    ScompHeartbeat hb = { _my_flag, uptime_ms };
    uint16_t size = _xfer.txObj(msg_type, 0);
    size = _xfer.txObj(hb, size);
    _xfer.sendData(size);
}

void ScompLink::sendLedTrigger(const ScompLedTrigger &msg) {
    uint8_t msg_type = SCOMP_MSG_LED_TRIGGER;
    uint16_t size = _xfer.txObj(msg_type, 0);
    size = _xfer.txObj(msg, size);
    _xfer.sendData(size);
}

void ScompLink::sendInputChannels(const ScompInputChannels &msg) {
    uint8_t msg_type = SCOMP_MSG_IN_CHANNELS;
    uint16_t size = _xfer.txObj(msg_type, 0);
    size = _xfer.txObj(msg, size);
    _xfer.sendData(size);
}

void ScompLink::sendOutputChannels(const ScompOutputChannels &msg) {
    uint8_t msg_type = SCOMP_MSG_OUT_CHANNELS;
    uint16_t size = _xfer.txObj(msg_type, 0);
    size = _xfer.txObj(msg, size);
    _xfer.sendData(size);
}

void ScompLink::sendStripConfig(const ScompStripConfig &msg) {
    uint8_t msg_type = SCOMP_MSG_STRIP_CONFIG;
    uint16_t size = _xfer.txObj(msg_type, 0);
    size = _xfer.txObj(msg, size);
    _xfer.sendData(size);
}

