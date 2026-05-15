# Scomp Serial Protocol

Serial communication layer between the Teensy 4.1 (Motivator) and the ESP32 (Sparkle Motion).
The three files in this folder are the complete implementation — copy them as-is to both projects.
Only the serial port passed to `scomp.begin()` differs between sides.

**These files should be kept in sync between Teensy Motivator and Scomp Repositories**

## Hardware

| Signal | Teensy 4.1 | ESP32 |
|--------|-----------|-------|
| TX | Serial5 pin 20 | RXD (your UART) |
| RX | Serial5 pin 21 | TXD (your UART) |
| Baud | 115200 (`SCOMP_BAUD_RATE`) | 115200 |

## Frame Format (protocol v2)

```
[0xAB][TYPE:1][LEN_LO:1][LEN_HI:1][PAYLOAD:LEN][CRC8:1][0xCD]
```

- Length is little-endian `uint16_t` — payloads up to 4096 bytes supported
- CRC-8 (Dallas/Maxim, poly 0x8C) covers `TYPE + LEN_LO + LEN_HI + PAYLOAD`
- Corrupt or partial frames are silently discarded and the parser resets to `WAIT_START`

## Messages

| Direction | ID | Struct | Payload |
|---|---|---|---|
| T→E | `0x01` | `ScompHeartbeat` | version, uptime_ms, SBUS flags (6 B) |
| T→E | `0x02` | `ScompInputChannels` | 24 SBUS values + enabled mask (51 B) |
| T→E | `0x03` | `ScompOutputChannels` | 48 µs values + enabled mask (102 B) |
| T→E | `0x04` | `ScompAudioState` | volume, playing state, file number (4 B) |
| T→E | `0x05` | `ScompLedTrigger` | animation ID + RGB + brightness (5 B) |
| E→T | `0x81` | *(no payload)* | request full state dump |
| E→T | `0x82` | `ScompSetVolume` | volume 0–30 (1 B) |
| E→T | `0x83` | `ScompTriggerAudio` | file number (2 B) |
| E→T | `0x84` | `ScompSetSetting` | key / index / value (6 B) |

## Teensy Setup

```cpp
// In setup(), after Serial5.begin(SCOMP_BAUD_RATE):
static uint8_t scomp_tx_buf[512];
static uint8_t scomp_rx_buf[512];
Serial5.addMemoryForWrite(scomp_tx_buf, sizeof(scomp_tx_buf));
Serial5.addMemoryForRead (scomp_rx_buf, sizeof(scomp_rx_buf));
scomp.begin(Serial5);
scomp.onMessage(onScompMessage);

// In loop() — every iteration, not on a timer:
scomp.update();
```

The TX/RX buffer extensions are Teensy-specific (`addMemoryForWrite` / `addMemoryForRead`).
They make writes non-blocking for bursts up to 512 bytes and prevent RX overflow between
`update()` calls. The equivalent on ESP32 is the buffer size argument to `Serial.begin()`.

Channel state is pushed at 20 Hz (`SCOMP_SEND_INTERVAL_MS=50`, overridable via build flag).
Heartbeat is sent on the `HEARTBEAT_INTERVAL_MS` timer (default 5 s, set in `platformio.ini`).

## ESP32 Setup

```cpp
// In setup():
Serial2.begin(115200);
scomp.begin(Serial2);   // or whichever UART is wired to Teensy pin 20/21
scomp.onMessage(onScompMessage);

// In loop():
scomp.update();
```

Callback signature on both sides:
```cpp
void onScompMessage(uint8_t msg_type, const uint8_t *payload, uint16_t len, void *) { ... }
```

Do not share the scomp UART with `Serial.print` debug output — text bytes in the stream
will keep the parser stuck in `WAIT_START` waiting for the `0xAB` start byte.

## RX Diagnostics

`ScompSerial` exposes four counters useful during bring-up:

| Method | Meaning |
|--------|---------|
| `rxBytes()` | Total bytes received |
| `rxSyncDrops()` | Bytes discarded in `WAIT_START` (not `0xAB`) |
| `rxFrames()` | Complete, CRC-valid frames dispatched |
| `rxCrcErrors()` | Frames that parsed but failed CRC |

If `rxSyncDrops ≈ rxBytes` the start byte is almost never appearing — the sender is not
transmitting scomp frames or is on the wrong UART. If `rxCrcErrors` is climbing the frame
structure is being found but the CRC is wrong — likely a protocol version mismatch.

## Future: Config Upload

`SCOMP_MSG_CONFIG_UPLOAD` is not yet defined. When added, use a single message for configs
up to 4096 bytes, or chunk across multiple 50 ms ticks to avoid stalling the TX buffer.
At 115200 baud a 4 KB payload takes ~350 ms; bumping to 460800 reduces that to ~87 ms.
