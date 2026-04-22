#include "sbus_bolderflight.h"

SbusHandler::SbusHandler(HardwareSerial* serial)
#if defined(ARDUINO_ARCH_ESP32)
    : _sbus(serial, SBUS_RX_PIN, SBUS_TX_PIN, true) {}
#else
    : _sbus(serial) {}
#endif

void SbusHandler::begin() {
    _sbus.Begin();
}

bool SbusHandler::read() {
    if (_sbus.Read()) {
        _data = _sbus.data();
        return true;
    }
    return false;
}

int16_t SbusHandler::channel(uint8_t index) const {
    if (index < _data.NUM_CH) return _data.ch[index];
    return 0;
}

bool SbusHandler::failsafe() const { return _data.failsafe; }
bool SbusHandler::lostFrame() const { return _data.lost_frame; }