#include "Maestro.h"

// Honestly all written by AI here

void Maestro::writeCommand(Stream& serial, uint8_t device, uint8_t command, const uint8_t* data, uint8_t length)
{
    serial.write(0xAA);     // Start byte
    serial.write(device);   // Device number
    serial.write(command);  // Command

    for (uint8_t i = 0; i < length; i++) {
        serial.write(data[i]);
    }
}

//void Maestro::setTarget(Stream& serial, uint8_t device, uint8_t channel, uint16_t target)
//{
//    uint8_t data[3];
//    data[0] = channel;
//    data[1] = target & 0x7F;
//    data[2] = (target >> 7) & 0x7F;
//
//    writeCommand(serial, device, 0x04, data, 3);
//}
void Maestro::setTarget(Stream& serial, uint8_t device, uint8_t channel, uint16_t target) {
    serial.write(0xAA);               // Pololu protocol start byte
    serial.write(device);             // Device number (e.g. 12)
    serial.write(0x04);               // Set Target command
    serial.write(channel);            // Servo channel
    serial.write(target & 0x7F);      // Low 7 bits of target
    serial.write((target >> 7) & 0x7F); // High 7 bits of target
}

void Maestro::setSpeed(Stream& serial, uint8_t device, uint8_t channel, uint16_t speed)
{
    uint8_t data[3];
    data[0] = channel;
    data[1] = speed & 0x7F;
    data[2] = (speed >> 7) & 0x7F;

    writeCommand(serial, device, 0x07, data, 3);
}

void Maestro::setAcceleration(Stream& serial, uint8_t device, uint8_t channel, uint16_t accel)
{
    uint8_t data[3];
    data[0] = channel;
    data[1] = accel & 0x7F;
    data[2] = (accel >> 7) & 0x7F;

    writeCommand(serial, device, 0x09, data, 3);
}

bool Maestro::waitForBytes(Stream& serial, size_t count, uint32_t timeout)
{
    uint32_t start = millis();
    while (serial.available() < count) {
        if (millis() - start > timeout) {
            return false;
        }
    }
    return true;
}

uint16_t Maestro::getPosition(Stream& serial, uint8_t device, uint8_t channel, uint32_t timeout)
{
    uint8_t data[1];
    data[0] = channel;

    writeCommand(serial, device, 0x10, data, 1);

    if (!waitForBytes(serial, 2, timeout)) {
        return 0xFFFF; // error / timeout
    }

    uint8_t lsb = serial.read();
    uint8_t msb = serial.read();

    return (msb << 8) | lsb;
}

bool Maestro::getMovingState(Stream& serial, uint8_t device, uint32_t timeout)
{
    writeCommand(serial, device, 0x13, nullptr, 0);

    if (!waitForBytes(serial, 1, timeout)) {
        return false; // assume not moving on timeout
    }

    return serial.read() != 0;
}