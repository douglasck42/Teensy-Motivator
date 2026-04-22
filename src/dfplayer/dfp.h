#pragma once
#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

// Physical Pinouts
#ifndef DF_SERIAL
#define DF_SERIAL Serial2
#endif

// Software Serial for DFPlayer on pins 2 (RX) and 3 (TX) on the Nano, but use Serial2 hardware serial on the ESP32 if available
#define FPSerial DF_SERIAL  // expands to Serial2

// DFRobotDFPlayerMini instance using the defined serial port
extern DFRobotDFPlayerMini myDFPlayer;

void dfpSetup();
void dfpPrintDetail(uint8_t type, int value);
void dfpStop();
void dfpPlay(int fileNumber);
void dfpVolume(int volume);
