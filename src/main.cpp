// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later

#define BUILD_VERSION "0.2.0"
#include <Arduino.h>
//#include <LittleFS.h>
#include "sbus/sbus_config.h"
#include "settings.h"
#include "dfplayer/dfp.h"
#include "kyberpad/kyberpad.h"
#include "maestro/maestro.h"
#include "json/JsonStorage.h"

// Watching flags
#ifndef WATCHDOG_ENABLED
#define WATCHDOG_ENABLED 1
#endif
#ifndef WATCHDOG_TIMEOUT_SECONDS
#define WATCHDOG_TIMEOUT_SECONDS 10
#endif
#if WATCHDOG_ENABLED
#include <esp_task_wdt.h>
#endif

// Other timer values
#ifndef HEARTBEAT_INTERVAL_MS
#define HEARTBEAT_INTERVAL_MS 60000
#endif
#ifndef PRINT_ALL_INTERVAL_MS
#define PRINT_ALL_INTERVAL_MS 30000
#endif

#ifndef WAIT_FOR_SERIAL
#define WAIT_FOR_SERIAL 0
#endif

// Hand off the serial port here - Serial1 = pins 0/1 on Teensy 4.1
SbusDriverType sbusHandler(&SBUS_SERIAL);
Maestro maestro;

// Define all functons:
void inputOutputMapping(uint8_t channel, unsigned long now);
void ioVolume(uint8_t channel);


// ========================= CONFIG =========================
// 0 = auto-detect, 16 = SBUS-16, 24 = SBUS-24 - this value is fungible in the hopes of adding in detection for SBUS-16 vs SBUS-24 in the future, but for now it is just used to select the driver and set the expected number of channels
uint8_t SBUS_CHANNELS = 0;
const uint8_t SBUS_INTERVAL_MS = 15; // Slightly over 14ms SBUS frame rate, you probably don't want to change this
constexpr uint8_t SBUS_LATE_WARNING_MS = 5;   // warn if we're this many ms late
constexpr uint16_t SBUS_WARN_COOLDOWN_MS   = 1000; // suppress repeated warnings for this long

void loadSettingsWrapper();

// ========================= SETUP =========================
void setup() {
    Serial.begin(115200);
#if WAIT_FOR_SERIAL == 1
    if (WAIT_FOR_SERIAL) {
        while (!Serial) {
            delay(100);
        }
    }

    Serial.println("Teensy Motivator v" BUILD_VERSION ": starting up... (serial required)");
#else
    delay(2000); // Wait for Serial to be ready but non-blocking
    Serial.println("Teensy Motivator v" BUILD_VERSION ": starting up... (serial optional)");
#endif


    Serial.print("Initializing Serial Ports");
    // 57600, 115200 - it's all static right now so
    // Serial2 - Handled by dfp.h and dfp.cpp
    Serial3.begin(115200);
    Serial4.begin(115200);
    Serial5.begin(115200);
    Serial6.begin(115200);
    Serial7.begin(115200);  // Maestro static for now
    // Serial8 - Handled by SBUS

    // Enable watchdog (5 second timeout)
    #if WATCHDOG_ENABLED
    esp_task_wdt_init(WATCHDOG_TIMEOUT_SECONDS, true);
    esp_task_wdt_add(NULL); // add current thread
    #endif

    loadSettingsDefaults();
    loadSettingsWrapper();

    //if (!LittleFS.begin(true)) {
    //    Serial.println("LittleFS: mount failed");
    //}
    //if (LittleFS.exists("/config.json")) {
    //    Serial.println("LittleFS: config.json EXISTS");
    //} else {
    //    Serial.println("LittleFS: config.json NOT FOUND");
    //}

#if SBUS_DRIVER == SBUS_DRIVER_BOLDERFLIGHT
    Serial.println("Driver: Bolderflight SBUS-16");
    SBUS_CHANNELS = 16;
#elif SBUS_DRIVER == SBUS_DRIVER_NATIVE16
    Serial.println("Driver: Native SBUS-16");
    SBUS_CHANNELS = 16;
#elif SBUS_DRIVER == SBUS_DRIVER_NATIVE24
    Serial.println("Driver: Native SBUS-24");
    SBUS_CHANNELS = 24;
#endif

    sbusHandler.begin();
    Serial.println("SBUS handler ready!");

    dfpSetup(); // Configure DFPlayer settings based on the detected state of the player and the SD card. This is called from setup() after initializing the player to ensure that the player is configured correctly before use.

    // force the volume to the mid level
    dfpVolume(settings.audio.initial);
    Serial.printf("DFPlayer: Volume set to %d of %d\n", settings.audio.initial, settings.audio.max);
    settings.audio.volume = settings.audio.initial; // Update the settings with the new volume level

    Serial.println("Teensy Motivator now motivating! (setup complete)");

} // setup()

void loadSettingsWrapper() {
    // Initialize SD card
    if (!JsonStorage::begin(BUILTIN_SDCARD)) {
        Serial.println("setup_load_wrapper(): Teensy 4.1 SD init failed");
        // cfg remains at compiled defaults — safe to continue
    }

    // Load settings — scoped block so JsonDocument is freed immediately after
    {
        if (settingsLoad("/config.json", settings)) {
            Serial.println("Settings loaded");
        } else {
            Serial.println("No config found, using defaults");
            // Optionally write defaults out on first boot:
            settingsSave("/config.json", settings);
        }
    }

    settingsSave("/config_backup.json", settings); // Backup the loaded (or default) settings for debugging purposes
    
}
Stream& getSerialPort(uint8_t port) {
    switch (port) {
        case 1: return Serial1;
        case 2: return Serial2;
        case 3: return Serial3;
        case 4: return Serial4;
        case 5: return Serial5;
        case 6: return Serial6;
        case 7: return Serial7;
        case 8: return Serial8;
        default: return Serial;  // fallback to USB serial
    }
}

// Print out ALL SBUS Channels (Remember that index 0 is channel 1 in SBUS)
void printAllChannels() {
    Serial.print("SBUS: ");
    for (uint8_t i = 0; i < SBUS_CHANNELS; i++) {
        Serial.printf("Ch%d:%d ", i + 1, sbusHandler.channel(i));
    }
    Serial.println("");
}

uint16_t sbusToMicroseconds(uint8_t channel) {
    return map(settings.ichannel[channel].sbus_value, sbusGetMin(channel), sbusGetMax(channel), usGetMin(CH_IN, channel), usGetMax(CH_IN, channel));
}

// Read the SBUS values and save them to the settings.ichannel[] array
// We don't actually DO anything here except save them (.sbus_value AND .us_value)
void readSbusInputs(unsigned long now) {
    static unsigned long millis_lastSerialOuptut = 0;
    for (uint8_t current_channel = 0; current_channel < SBUS_CHANNELS; current_channel++) {
        int16_t current_value = sbusHandler.channel(current_channel);
        
        current_value = constrain(current_value, sbusGetMin(current_channel), sbusGetMax(current_channel));
        auto& ch = settings.ichannel[current_channel];

        if (ch.sbus_value != current_value) {
            ch.updated = true;      // Downstream functions should ACT on this value
            ch.sbus_value = current_value;
            ch.us_value = sbusToMicroseconds(current_channel);
        }
        if (ch.serial_debug_output && (now - millis_lastSerialOuptut >= settings.system.per_channel_serial_output_throttle)) {
            printChannel(CH_IN, current_channel);
        }
    }
    if (now - millis_lastSerialOuptut >= settings.system.per_channel_serial_output_throttle) {
        millis_lastSerialOuptut = now;
    }
}

//
void inputFunctions(unsigned long now) {
    for (uint8_t current_channel = 0; current_channel < SBUS_CHANNELS; current_channel++) {
        auto& ch = settings.ichannel[current_channel];
        if (ch.updated) {
                 
            switch (ch.channelFunction) {
                case iChannelFunction::KYBERPAD:
                    ch.updated = false;     // We're calling the function, turn this off now
                    ioKyberpadButtons(current_channel, now);
                    break;
                case iChannelFunction::KYBERPAD_PAGE:
                    ch.updated = false;     // We're calling the function, turn this off now
                    ioKyberpadPage(current_channel);
                    break;
                case iChannelFunction::VOLUME:
                    ch.updated = false;     // We're calling the function, turn this off now
                    ioVolume(current_channel);
                    break;
                default: break;
            }
        }
    }
}

// consider the output channels and make things happen
void sendOuputs(unsigned long now) {
    static unsigned long millis_output = now;
    bool do_debug = (now - millis_output >= settings.system.per_channel_serial_output_throttle);
    bool did_debug = false;
    for (uint8_t current_channel = 0; current_channel < NUM_OUTPUT_CHANNELS; current_channel++) {
        auto& och = settings.ochannel[current_channel];
        if (!och.enabled || !och.updated) continue;
        och.updated = false;        // we're processing the channel now, clear this

        // splash of debug output
        if (och.serial_debug_output && do_debug) {
            ioPrintChannel(current_channel, och.ichannel);
            did_debug = true;
        }

        if (och.serial_port_out > 2 && och.serial_port_out < 9 && och.maestro_id > 0) {
            maestro.setTarget(getSerialPort(och.serial_port_out), och.maestro_id, och.maestro_ch, och.us_value);
        }
    }
    if (do_debug && did_debug) millis_output = now;
}

// Handle the processing of Input/Outputs
void outputMapping(unsigned long now) {
    #define outputMappingDebug 0    // Set to 1 to enable the outputMapping debug serial output; developer special

    #if outputMappingDebug == 1
    static unsigned long millis_output = now;
    bool do_debug = (now - millis_output >= settings.system.per_channel_serial_output_throttle);
    bool did_debug = false;
    #endif

    for (uint8_t current_channel = 0; current_channel < NUM_OUTPUT_CHANNELS; current_channel++) {
        auto& och = settings.ochannel[current_channel];

        // Skip processing for this channel if it's not enabled
        if (!och.enabled) continue; 

        if (och.ichannel_enabled) {
            auto& ich = settings.ichannel[och.ichannel];
            if (ich.updated) {
                och.updated = true;
                // adjust the relative µs values to match input channel 100% to output channel 100%
                och.us_value = map(ich.us_value, usGetMin(CH_IN, och.ichannel), usGetMax(CH_IN, och.ichannel), usGetMin(CH_OUT, current_channel), usGetMax(CH_OUT, current_channel));

                ich.updated = false;        // we've processed it, clear it
                #if outputMappingDebug == 1
                if (och.serial_debug_output && do_debug) {
                    ioPrintChannel(current_channel, och.ichannel);
                    did_debug = true;
                }
                #endif
            }
        }
    }

    #if outputMappingDebug == 1
    if (do_debug && did_debug) millis_output = now;
    #endif

}

void ioVolume(uint8_t channel) {
    // Map the SBUS value to a volume level between 0 and 30
    int8_t volume = map(settings.ichannel[channel].us_value, usGetMin(CH_IN, channel), usGetMax(CH_IN, channel), settings.audio.min, settings.audio.max);
    volume = constrain(volume, settings.audio.min, settings.audio.max);
    if (volume != settings.audio.volume) {
        dfpVolume(volume);
        Serial.printf("DFPlayer: Volume set to %d of %d\n", volume, settings.audio.max);
        settings.audio.volume = volume; // Update the settings with the new volume level
    }
}


// ========================= LOOP =========================
void loop() {
    unsigned long now = millis();

    // timers, these get set to current millis() at various points in the code to manage timing of different functions and features
    static unsigned long millis_lastSbusRead = now;
    static unsigned long millis_lastPrintAll = now;
    static unsigned long millis_lastHeartbeat = now;
    static unsigned long millis_lastSbusWarn = now; // tracks last time we emitted a late warning
    //static unsigned long millis_maestro = 0; //
    //static uint16_t maestro_position = 6000; //


    // watchdog reset to prevent system hangs, especially important if the SD card is missing or there's an issue with the DFPlayer that could cause blocking calls
    #if WATCHDOG_ENABLED
    esp_task_wdt_reset();
    #endif

    // Heartbeat
    if (now - millis_lastHeartbeat >= HEARTBEAT_INTERVAL_MS) {
        Serial.println("Heartbeat: Teensy Motivator alive");
        millis_lastHeartbeat = now;
    }
    if (now - millis_lastPrintAll >= PRINT_ALL_INTERVAL_MS) {
        printAllChannels();
        millis_lastPrintAll = now;
        millis_lastHeartbeat = now; // if we're printing all channels, also reset the heartbeat timer since we know the system is alive and well enough to print out all channel values
    }

    // SBUS Input and output handling
    if ((now - millis_lastSbusRead) >= SBUS_INTERVAL_MS) {

        // Generate warnings if we're running late - indictive of code running to slow or too blocking somewhere else
        unsigned long actualInterval = now - millis_lastSbusRead;
        if (actualInterval > (SBUS_INTERVAL_MS + SBUS_LATE_WARNING_MS)) {
            if ((now - millis_lastSbusWarn) >= SBUS_WARN_COOLDOWN_MS) {
                millis_lastSbusWarn = now;
                Serial.printf("[WARN] SBUS poll late: %lu ms (expected %u ms)\n", actualInterval, SBUS_INTERVAL_MS);
            }
        }

        // Update the timer and read the sbus
        millis_lastSbusRead = now;
        if (sbusHandler.read()) {
            readSbusInputs(now);
            if (sbusHandler.failsafe())  Serial.println("SBUS: ** FAILSAFE **");
            if (sbusHandler.lostFrame()) Serial.println("SBUS: ** LOST FRAME **");
        }

        inputFunctions(now);        // Handling of predefined input functions (like kyberpad)
        outputMapping(now);         // Create maps of user outputs
        // run programmatic automation/scriptings tools
        sendOuputs(now);            // write the serial output queues

        //uint8_t maestro_id = 1;
        //uint8_t channel = 0;
        //maestro.setTarget(Serial7, maestro_id, channel, settings.ichannel[channel].us_value);
        //if (now - millis_maestro >= 1500) {
        //    millis_maestro = now;
        //    Serial.printf("Channel 2 SBUS %d Maestro %dµs\n", settings.ichannel[1].sbus_value, settings.ichannel[channel].us_value);
        //}
    }

}