# Scomp Serial Protocol

Serial communication layer between the Teensy 4.1 (Motivator) and the ESP32 (Sparkle Motion).
The three files in this folder are the complete implementation — copy them as-is to both projects.
Only the serial port passed to `scomp.begin()` differs between sides.

**These files should be kept in sync between Teensy Motivator and Scomp Repositories**

## Hardware

| Signal | Teensy 4.1     | ESP32 |
|--------|----------------|-------|
| TX     | Serial5 pin 20 | RXD (your UART) |
| RX     | Serial5 pin 21 | TXD (your UART) |
| Baud   | 115200 (`SCOMP_BAUD_RATE`) | 115200 |
