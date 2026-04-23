#define BUILD_VERSION "0.1.4"
#include <Arduino.h>
//#include <LittleFS.h>
#include "sbus/sbus_config.h"
#include "settings.h"
#include "dfplayer/dfp.h"
#include "kyberpad/kyberpad.h"
#include <PololuMaestro.h>

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
MiniMaestro maestro(Serial7);  // or whichever serial port

// Define all functons:
void inputOutputMapping(int channel, int value, unsigned long now);
void ioVolume(int channel, int16_t value);


// ========================= CONFIG =========================
// 0 = auto-detect, 16 = SBUS-16, 24 = SBUS-24 - this value is fungible in the hopes of adding in detection for SBUS-16 vs SBUS-24 in the future, but for now it is just used to select the driver and set the expected number of channels
uint8_t SBUS_CHANNELS = 0;
const uint32_t SBUS_INTERVAL_MS = 15; // Slightly over 14ms SBUS frame rate, you probably don't want to change this
constexpr uint32_t SBUS_LATE_WARNING_MS = 5;   // warn if we're this many ms late
constexpr uint32_t SBUS_WARN_COOLDOWN_MS   = 1000; // suppress repeated warnings for this long

// ========================= SETUP =========================
void setup() {
    Serial.begin(115200);
#if WAIT_FOR_SERIAL == 1
    if (WAIT_FOR_SERIAL) {
        while (!Serial) {
            delay(100);
        }
    }
    Serial.println("Nano Motivator v" BUILD_VERSION ": starting up... (serial required)");
#else
    delay(2000); // Wait for Serial to be ready but non-blocking
    Serial.println("Nano Motivator v" BUILD_VERSION ": starting up... (serial optional)");
#endif

    Serial.print("Maestro: Init Serial7");
    Serial7.begin(57600);

    // Enable watchdog (5 second timeout)
    #if WATCHDOG_ENABLED
    esp_task_wdt_init(WATCHDOG_TIMEOUT_SECONDS, true);
    esp_task_wdt_add(NULL); // add current thread
    #endif

    loadSettings();

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

    Serial.println("Nano Motivator now motivating! (setup complete)");

} // setup()

// Print out a given SBUS Channel (Remember that index 0 is channel 1 in SBUS)
void printChannel(uint8_t index, int16_t value) {
    if (settings.system.debug_sbus) {
        Serial.printf("SBUS: CH%d: %d\n", index + 1, value);
    }
}

// Print out ALL SBUS Channels (Remember that index 0 is channel 1 in SBUS)
void printAllChannels() {
    Serial.print("SBUS: ");
    for (uint8_t i = 0; i < SBUS_CHANNELS; i++) {
        Serial.printf("CH%d:%d ", i + 1, sbusHandler.channel(i));
    }
    Serial.println("");
}

// Read the SBUS values and save them to the settings.channel[] array.
// If a channel value has changed by more than the defined DEADBAND since the last read, print it out to the serial console and call the inputOutputMapping function to handle any necessary output changes based on the new input value.
void handleSerialOutput(unsigned long now) {
    for (uint8_t current_channel = 0; current_channel < SBUS_CHANNELS; current_channel++) {
        int16_t current_value = sbusHandler.channel(current_channel);
        
        if (settings.system.debug_sbus) {
            if (abs(current_value - settings.channel[current_channel].value) > settings.channel[current_channel].print_jitter) {
                printChannel(current_channel, current_value);
            }
        }

        if (current_value < settings.channel[current_channel].min) {
            current_value = settings.channel[current_channel].min;
        } else if (current_value > settings.channel[current_channel].max) {
            current_value = settings.channel[current_channel].max;
        }

        if (settings.channel[current_channel].value != current_value) {
            settings.channel[current_channel].value = current_value;
            inputOutputMapping(current_channel, current_value, now);
        }
    }
}

// Handle the processing of Input/Outputs
void inputOutputMapping(int channel, int value, unsigned long now) {

    // Skip processing for this channel if it's not enabled
    if (!settings.channel[channel].enabled) {
        return; 
    }

    settings.channel[channel].value = value; // Update the current value for this channel in the settings

    // Map the volume control channel to the DFPlayer volume
    if (settings.channel[channel].volume_channel) {
        ioVolume(channel, value);
    // Map KyberPad button channels to the ioKyberpadButtons function (The Software-defined Screen)
    } else if (settings.channel[channel].kyberpad_channel) {
        ioKyberpadButtons(channel, value, now);
    // Map KyberPad page selector channel to the ioKyberpadPage function (Two or Three way toggle)
    } else if (settings.channel[channel].kyberpad_page_channel) {
        ioKyberpadPage(channel, value);
    }
}

void ioVolume(int channel, int16_t value) {
    // Map the SBUS value to a volume level between 0 and 30
    int16_t volume = map(value, settings.audio.sbus_min, settings.audio.sbus_max, settings.audio.min, settings.audio.max);
    volume = constrain(volume, settings.audio.min, settings.audio.max);
    if (volume != settings.audio.volume) {
        dfpVolume(volume);
        Serial.printf("DFPlayer: Volume set to %d of %d\n", volume, settings.audio.max);
        settings.audio.volume = volume; // Update the settings with the new volume level
    }
}


// ========================= LOOP =========================
void loop() {

    // timers, these get set to current millis() at various points in the code to manage timing of different functions and features
    static uint32_t millis_lastSbusRead = 0;
    static uint32_t millis_lastPrintAll = 0;
    static uint32_t millis_lastHeartbeat = 0;
    static uint32_t millis_lastSbusWarn = 0; // tracks last time we emitted a late warning
    static uint32_t millis_maestro = 0; //
    static uint32_t maestro_position = 6000; //

    unsigned long now = millis();

    // watchdog reset to prevent system hangs, especially important if the SD card is missing or there's an issue with the DFPlayer that could cause blocking calls
    #if WATCHDOG_ENABLED
    esp_task_wdt_reset();
    #endif

    // Heartbeat
    if (now - millis_lastHeartbeat >= HEARTBEAT_INTERVAL_MS) {
        Serial.println("Heartbeat: Nano Motivator alive");
        millis_lastHeartbeat = now;
    }
    if (now - millis_lastPrintAll >= PRINT_ALL_INTERVAL_MS) {
        printAllChannels();
        millis_lastPrintAll = now;
        millis_lastHeartbeat = now; // if we're printing all channels, also reset the heartbeat timer since we know the system is alive and well enough to print out all channel values
    }

    // SBUS Input and output handling
    if (now - millis_lastSbusRead >= SBUS_INTERVAL_MS) {

        // Generate warnings if we're running late - indictive of code running to slow or too blocking somewhere else
        uint32_t actualInterval = now - millis_lastSbusRead;
        if (actualInterval > SBUS_INTERVAL_MS + SBUS_LATE_WARNING_MS) {
            if (now - millis_lastSbusWarn >= SBUS_WARN_COOLDOWN_MS) {
                millis_lastSbusWarn = now;
                Serial.printf("[WARN] SBUS poll late: %lu ms (expected %u ms)\n",
                            actualInterval, SBUS_INTERVAL_MS);
            }
        }

        // Update the timer and read the sbus
        millis_lastSbusRead = now;
        if (sbusHandler.read()) {
            handleSerialOutput(now);
            if (sbusHandler.failsafe())  Serial.println("SBUS: ** FAILSAFE **");
            if (sbusHandler.lostFrame()) Serial.println("SBUS: ** LOST FRAME **");
        }

        int32_t qus_min = settings.channel[1].min_us * 4;
        int32_t qus_max = settings.channel[1].max_us * 4;

        maestro_position = map(settings.channel[1].value, settings.channel[1].min, settings.channel[1].max, qus_min, qus_max);
        maestro_position = constrain(maestro_position, qus_min, qus_max);
        maestro.setTarget(0, maestro_position);
        if (now - millis_maestro >= 1500) {
            millis_maestro = now;
            Serial.printf("Channel 2 SBUS %d Maestro %d\n", settings.channel[1].value, maestro_position);
        }
    }

}