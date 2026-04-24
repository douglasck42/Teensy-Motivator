#pragma once

#include <Arduino.h>

class Maestro {
public:
    Maestro() = default;

    // Core commands
    void setTarget(Stream& serial, uint8_t device, uint8_t channel, uint16_t target);
    void setSpeed(Stream& serial, uint8_t device, uint8_t channel, uint16_t speed);
    void setAcceleration(Stream& serial, uint8_t device, uint8_t channel, uint16_t accel);

    uint16_t getPosition(Stream& serial, uint8_t device, uint8_t channel, uint32_t timeout = 20);
    bool getMovingState(Stream& serial, uint8_t device, uint32_t timeout = 20);

private:
    void writeCommand(Stream& serial, uint8_t device, uint8_t command, const uint8_t* data, uint8_t length);
    bool waitForBytes(Stream& serial, size_t count, uint32_t timeout);
};