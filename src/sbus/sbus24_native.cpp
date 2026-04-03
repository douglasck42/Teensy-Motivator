#include "sbus24_native.h"

Sbus24Native::Sbus24Native(HardwareSerial* serial)
    : _serial(serial), _bufIdx(0), _parserState(0), _prevByte(SBUS_FOOTER),
      _failsafe(false), _lostFrame(false), _lastByteMs(0) {
    memset(_channels, 0, sizeof(_channels));
    memset(_payload, 0, sizeof(_payload));
    memset(_buf, 0, sizeof(_buf));
}

void Sbus24Native::begin() {
    _parserState = 0;
    _prevByte = SBUS_FOOTER;
    // SBUS: 100000 baud, 8E2, inverted signal
    _serial->begin(SBUS_BAUD, SERIAL_8E2_RXINV);
}

// Bolderflight-style state machine parser.
// Looks for 0x0F preceded by 0x00 (or SBUS2 footer),
// collects 34 payload bytes, validates footer.
bool Sbus24Native::read() {
    static elapsedMicros sbusTime = 0;

    // Reset parser if no byte received within timeout
    if (sbusTime > SBUS_TIMEOUT_US) {
        _parserState = 0;
    }

    bool gotFrame = false;

    while (_serial->available()) {
        sbusTime = 0;
        uint8_t cur = _serial->read();

        if (_parserState == 0) {
            // Looking for header preceded by a valid footer byte
            if ((cur == SBUS_HEADER) &&
                ((_prevByte == SBUS_FOOTER) || ((_prevByte & SBUS2_FOOTER_MASK) == SBUS2_FOOTER_VAL))) {
                _parserState = 1;
            }
        } else {
            // Collecting payload bytes
            if ((_parserState - 1) < SBUS_PAYLOAD_LEN) {
                _payload[_parserState - 1] = cur;
                _parserState++;
            }
            // Check footer once payload is complete
            if ((_parserState - 1) == SBUS_PAYLOAD_LEN) {
                if ((cur == SBUS_FOOTER) || ((cur & SBUS2_FOOTER_MASK) == SBUS2_FOOTER_VAL)) {
                    _parserState = 0;
                    if (parseFrame()) gotFrame = true;
                } else {
                    _parserState = 0;
                }
            }
        }
        _prevByte = cur;
    }

    return gotFrame;
}

// Unpack 24 x 11-bit channels from 34-byte payload.
// Layout mirrors bolderflight SBUS-24:
//   payload[0..10]  = CH1-CH8   (identical to SBUS-16 block 1)
//   payload[11..21] = CH9-CH16  (identical to SBUS-16 block 2)
//   payload[22..32] = CH17-CH24 (new third block)
//   payload[33]     = flags
bool Sbus24Native::parseFrame() {
    // CH1-CH8
    _channels[0]  = (int16_t)((_payload[0]  | _payload[1]  << 8)                          & 0x07FF);
    _channels[1]  = (int16_t)((_payload[1]  >> 3 | _payload[2]  << 5)                     & 0x07FF);
    _channels[2]  = (int16_t)((_payload[2]  >> 6 | _payload[3]  << 2 | _payload[4]  << 10) & 0x07FF);
    _channels[3]  = (int16_t)((_payload[4]  >> 1 | _payload[5]  << 7)                     & 0x07FF);
    _channels[4]  = (int16_t)((_payload[5]  >> 4 | _payload[6]  << 4)                     & 0x07FF);
    _channels[5]  = (int16_t)((_payload[6]  >> 7 | _payload[7]  << 1 | _payload[8]  << 9) & 0x07FF);
    _channels[6]  = (int16_t)((_payload[8]  >> 2 | _payload[9]  << 6)                     & 0x07FF);
    _channels[7]  = (int16_t)((_payload[9]  >> 5 | _payload[10] << 3)                     & 0x07FF);

    // CH9-CH16
    _channels[8]  = (int16_t)((_payload[11] | _payload[12] << 8)                           & 0x07FF);
    _channels[9]  = (int16_t)((_payload[12] >> 3 | _payload[13] << 5)                      & 0x07FF);
    _channels[10] = (int16_t)((_payload[13] >> 6 | _payload[14] << 2 | _payload[15] << 10) & 0x07FF);
    _channels[11] = (int16_t)((_payload[15] >> 1 | _payload[16] << 7)                      & 0x07FF);
    _channels[12] = (int16_t)((_payload[16] >> 4 | _payload[17] << 4)                      & 0x07FF);
    _channels[13] = (int16_t)((_payload[17] >> 7 | _payload[18] << 1 | _payload[19] << 9)  & 0x07FF);
    _channels[14] = (int16_t)((_payload[19] >> 2 | _payload[20] << 6)                      & 0x07FF);
    _channels[15] = (int16_t)((_payload[20] >> 5 | _payload[21] << 3)                      & 0x07FF);

    // CH17-CH24
    _channels[16] = (int16_t)((_payload[22] | _payload[23] << 8)                           & 0x07FF);
    _channels[17] = (int16_t)((_payload[23] >> 3 | _payload[24] << 5)                      & 0x07FF);
    _channels[18] = (int16_t)((_payload[24] >> 6 | _payload[25] << 2 | _payload[26] << 10) & 0x07FF);
    _channels[19] = (int16_t)((_payload[26] >> 1 | _payload[27] << 7)                      & 0x07FF);
    _channels[20] = (int16_t)((_payload[27] >> 4 | _payload[28] << 4)                      & 0x07FF);
    _channels[21] = (int16_t)((_payload[28] >> 7 | _payload[29] << 1 | _payload[30] << 9)  & 0x07FF);
    _channels[22] = (int16_t)((_payload[30] >> 2 | _payload[31] << 6)                      & 0x07FF);
    _channels[23] = (int16_t)((_payload[31] >> 5 | _payload[32] << 3)                      & 0x07FF);

    // Flags
    _lostFrame = (_payload[33] & SBUS_FLAG_LOST_FRAME) != 0;
    _failsafe  = (_payload[33] & SBUS_FLAG_FAILSAFE)   != 0;

    return true;
}

int16_t Sbus24Native::channel(uint8_t index) const {
    if (index < 24) return _channels[index];
    return 0;
}

bool Sbus24Native::failsafe()  const { return _failsafe;  }
bool Sbus24Native::lostFrame() const { return _lostFrame; }
