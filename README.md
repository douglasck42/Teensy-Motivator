# Teensy-Motivator
Teensy 4.1 Based Droid Control. This code coveres the primary Teensy 4.1 system which it's primary job is converting inputs to outputs, trigging programmatic automation of events, triggering MP3s and LED sequences, and even override 
normal input/output maps for the purposes of automation.

## System Architecture
* Core System - This `Teensy Motivator` Repository
    * Teensy 4.1
    * DFPlayer Mini
    * Pololu D36V50F5 5v Voltage Regulator (accepting 5.5-50v DC input)
    * An available PCB Board simplifying the mounting of all of this (Hardware v1.0)
* Control System - Optional - `Teensy Scomp Link` Repository
    * Adafruit Sparkle Motion ESP32 Arduino
    * Attached via Serial Port to the Teensy 4.1
    * Provides 2 banks of WS2812b or 5050 LEDs
    * Provides Wireless Control - both a Web Interface and remote triggers via ESP-NOW
* Remote System - Optional - `Teensy Scomp Relay` Repository
    * Adafruit Sparkle Motion ESP32 Arduino
    * Provides 2 banks of WS2812b or 5050 LEDs
    * Listens for ESP-NOW Triggers to run LED animations
* KyberPad
    * From the Kyber team - Provides LUA scripts for up to 3 screens of 15 buttons each
    * Enables the triggering of MP3s, Automation, Etc
    * Seperate Purchase required

## Inputs:
    - Single Input: SBUS-16 or SBUS-24
    - Web Interface `Teensy Scomp Link`
    - KyberPad R/C Triggers
    - Serial Console for debugging and test bench work

## Outputs:
    - LEDs: Triggered via Serial communications Adafruit Sparkle
    - Serial2: DFPlayer music trigger
    - Serial3: Pololu Maestro or Similar (MARCduino, etc)
    - Serial4: Pololu Maestro or Similar (MARCduino, etc)
    - Serial5: Teensy Scomp Link (Adafruit Sparkle Motion)
    - Serial6: Pololu Maestro or Similar (MARCduino, etc)
    - Serial7: Pololu Maestro or Similar (MARCduino, etc)
    - Serial8: SBUS (FrSky Receiver)
    - Can Bus 1: Robotic Actuators, etc
    - Can Bus 3: Robotic Actuators, etc
    - Web Interface via Wi-Fi (status messages, etc)
    - Two GPIO Pins (11,12 and 40, 41) - Future Possibilities
    - I2C - Future Possibilities

## LED Animations
    - (TBD...)

## Maestro Serial Animations
    # 1-Axis Random Idle - Randomly Move a servo
    # 2-Axis Random Idle - Randomly Move a servo
    # 2-Axis Random Spinner - Randomly trigger DC Motor spins (B2EMO Tool Spinners)
    # 2-Axis Fixed Spinner - trigger DC Motor spins on a set sequence (B2EMO Tool Spinners)
    # 3-Axis Random Head Idle - Given 3 axis, randomly move a head unit around (BB-8 or B2EMO Simple Head)
    # 4-Axis Random Head Idle - Given 4 axis, randomly move a head unit around (B2EMO DF-Pro Head)
    