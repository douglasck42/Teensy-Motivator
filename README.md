# Teensy-Motivator
Teensy 4.1 Based Droid Control. This code coveres the primary Teensy 4.1 system which it's primary job is converting inputs to outputs, trigging programmatic automation of events, triggering MP3s and LED sequences, and even override 
normal input/output maps for the purposes of automation.

[Teensy 4.1 Overview](Documentation/teensy_overview.png)]

## System Architecture
* Core System - This `Teensy Motivator` Repository
    * Teensy 4.1
    * DFPlayer Mini
    * Pololu D36V50F5 5v Voltage Regulator (accepting 5.5-50v DC input)
    * An available PCB Board simplifying the mounting of all of this (Hardware v1.0)
* Control System - Optional - `Teensy Scomp Link` Repository
    * Adafruit Sparkle Mini ESP32 Arduino
    * Attached via Serial Port to the Teensy 4.1
    * Provides 2 banks of WS2812b or 5050 LEDs
    * Provides Wireless Control - both a Web Interface and remote triggers via ESP-NOW
* Remote System - Optional - `Teensy Scomp Relay` Repository
    * Adafruit Sparkle Mini ESP32 Arduino
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
    
# FrSky SBUS Tuning
The FrSky SBUS does not have great documentation.

## SBUS-16 or SBUS-24
First things first, right now it's a compiled setting. Pick on and that's that.

## SBUS Min/Max
If all of you outputs DO NOT exceed 100% then you can ignore this section. If you want more granular control below that, you may need to tune the min/max

If you outputs are **RESTRICTED below 100%**, you probably STILL want to **IGNORE THIS section**

If this isn't mapped properly, you won't get to min/max ratios proper. Either coming up short or blowing past a software limiter. If you're using Pololu Maestros those values, if changed, must match as well!

For SBUS, at **100%** from the FrSky transmitter, Minimum SBUS is `172` and maximum is `1810` - You change the `sbus_standard_min` and `sbus_standard_max` values if desired for default all-channel behavior.

For SBUS, at **125%** from the FrSky transmitter, Minimum SBUS is `0` and maximum is `2047` but in practice, I've only seen and configured the system for `8` and `1976`. You change the `sbus_abs_min` and `sbus_abs_max` values if desired for default all-channel behavior. 

Going above **125%** isn't supported, is discouraged by the FrSky transmitter GUI on the X20-RS, and I don't know what will happen.

Per-channel default is to use the system-wide standard values if the **per-channel default setting sbus_abs_minmax = false**. Setting **sbus_abs_minmax = true** uses the absolute full range values. Directly configuring channel[x].min and channel[x].max values uses THOSE no matter the boolean value.

## Reciever Output Warning
**On my lab bench, my TD SR6 cannot and will not transmit the absolute values (expanded range). The TD R6 and TD R10 CAN**. Unfortunatly you will have to expirement yourself to know for sure. 

*Both receivers on the v3.0.3 firmware which isn't even available for download for the TD R6*

## Pololu Maestros
As with the SBUS min/max values, if this isn't mapped properly, you won't get to min/max ratios proper. With the Maestro min/max you can't exceed that so you're safe, but if you want the physical min/max to match, these values MUST match.

The default maestro values are `992µs` min and `2000µs` max as defined by settings.maestro_standard_min and settings.maestro_standard_max

The absolute min/max maestro values are `496µs` and `2496µs` as defined by settings.maestro_abs_min and settings.maestro_abs_min.

As with SBUS, the per-channel setting of maestro_100_pct 

