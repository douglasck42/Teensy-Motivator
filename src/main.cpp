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
#include "scomp/scomp_serial.h"

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
#define HEARTBEAT_INTERVAL_MS 5000
#endif
#ifndef PRINT_ALL_INTERVAL_MS
#define PRINT_ALL_INTERVAL_MS 30000
#endif

#ifndef WAIT_FOR_SERIAL
#define WAIT_FOR_SERIAL 0
#endif

#ifndef DEBUG_SCOMP_RX
#define DEBUG_SCOMP_RX 0
#endif
#ifndef SCOMP_BAUD_RATE
#define SCOMP_BAUD_RATE 115200
#endif
static unsigned long millis_lastHeartbeat = 0;


// Hand off the serial port here - Serial1 = pins 0/1 on Teensy 4.1
SbusDriverType sbusHandler(&SBUS_SERIAL);
Maestro maestro;
ScompSerial scomp;

// Define all functons:
void inputOutputMapping(uint8_t channel, unsigned long now);
void ioVolume(uint8_t channel);

char* formatUptime(unsigned long ms) {
    static char buf[32];
    unsigned long seconds = ms / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours   = minutes / 60;
    unsigned long days    = hours   / 24;

    seconds %= 60;
    minutes %= 60;
    hours   %= 24;

    if (days > 0)
        sprintf(buf, "%lud %02luh %02lum %02lus", days, hours, minutes, seconds);
    else if (hours > 0)
        sprintf(buf, "%02luh %02lum %02lus", hours, minutes, seconds);
    else if (minutes > 0)
        sprintf(buf, "%02lum %02lus", minutes, seconds);
    else
        sprintf(buf, "%02lus", seconds);

    return buf;
}

// ========================= SCOMP =========================
void onScompMessage(uint8_t msg_type, const uint8_t *payload, uint8_t len, void *) {
    switch (msg_type) {
        case SCOMP_MSG_HEARTBEAT: {
            static unsigned long millis_lastEspHeartbeat = 0;
            unsigned long now_hb = millis();
            unsigned long gap = millis_lastEspHeartbeat ? (now_hb - millis_lastEspHeartbeat) : 0;
            millis_lastEspHeartbeat = now_hb;
            millis_lastHeartbeat = now_hb; // reset our heartbeat timeout timer message, this one is plenty
            if (len >= sizeof(ScompHeartbeat)) {
                const auto *hb = reinterpret_cast<const ScompHeartbeat *>(payload);
                Serial.printf("Heartbeat: Teensy UP %s", formatUptime(now_hb));
                Serial.printf(" | Scomp UP %s", formatUptime(hb->uptime_ms));
                Serial.printf(" | v%u flags=0x%02X gap=%lums\n", hb->version, hb->flags, gap);
            } else {
                Serial.printf("Heartbeat: Teensy UP %s | Scomp FAULT | short payload %u bytes\n", formatUptime(now_hb), len);
            }
            break;
        }
        case SCOMP_MSG_REQUEST_STATE: {
            if (settings.system.debug) Serial.println("SCOMP: ESP32 requested state dump");
            // full state will be sent on the next scompSendState() tick
            break;
        }
        case SCOMP_MSG_SET_VOLUME: {
            if (len < sizeof(ScompSetVolume)) break;
            const auto *msg = reinterpret_cast<const ScompSetVolume *>(payload);
            uint8_t vol = constrain(msg->volume, settings.audio.min, settings.audio.max);
            dfpVolume(vol);
            settings.audio.volume = vol;
            if (settings.system.debug) Serial.printf("SCOMP: Set volume → %u\n", vol);
            break;
        }
        case SCOMP_MSG_TRIGGER_AUDIO: {
            if (len < sizeof(ScompTriggerAudio)) break;
            const auto *msg = reinterpret_cast<const ScompTriggerAudio *>(payload);
            dfpPlay(msg->file_number);
            if (settings.system.debug) Serial.printf("SCOMP: Trigger audio → file %u\n", msg->file_number);
            break;
        }
        case SCOMP_MSG_SET_SETTING: {
            if (len < sizeof(ScompSetSetting)) break;
            const auto *msg = reinterpret_cast<const ScompSetSetting *>(payload);
            if (settings.system.debug) Serial.printf("SCOMP: Set setting key=0x%02X idx=%u val=%u\n",
                                                      msg->key, msg->index, msg->value.u8);
            switch (msg->key) {
                case SCOMP_SETTING_AUDIO_MUTE:
                    settings.audio.mute = (msg->value.u8 != 0);
                    if (settings.audio.mute) dfpVolume(0);
                    else dfpVolume(settings.audio.volume);
                    break;
                default: break;
            }
            break;
        }
        default:
            Serial.printf("SCOMP: Unknown message type 0x%02X len=%u\n", msg_type, len);
            break;
    }
}

void scompSendState(unsigned long now) {
    // Input channels
    ScompInputChannels in_msg = {};
    for (uint8_t i = 0; i < SCOMP_IN_CH; i++) {
        in_msg.values[i] = settings.ichannel[i].sbus_value;
        if (settings.ichannel[i].enabled) in_msg.enabled[i / 8] |= (1u << (i % 8));
    }
    scomp.sendInputChannels(in_msg);

    // Output channels
    ScompOutputChannels out_msg = {};
    for (uint8_t i = 0; i < SCOMP_OUT_CH; i++) {
        out_msg.values[i] = settings.ochannel[i].us_value;
        if (settings.ochannel[i].enabled) out_msg.enabled[i / 8] |= (1u << (i % 8));
    }
    scomp.sendOutputChannels(out_msg);

    // Audio state
    ScompAudioState audio_msg = {};
    audio_msg.volume = settings.audio.volume;
    audio_msg.state  = dfpIsPlaying() ? SCOMP_AUDIO_PLAYING : SCOMP_AUDIO_STOPPED;
    scomp.sendAudioState(audio_msg);
}

#ifndef SCOMP_SEND_INTERVAL_MS
#define SCOMP_SEND_INTERVAL_MS 50   // 20 Hz channel updates to ESP32
#endif


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
    static uint8_t Serial4_tx_buf[256];
    static uint8_t Serial5_tx_buf[256];
    static uint8_t Serial6_tx_buf[256];
    // 57600, 115200 - it's all static right now so
    // Serial2 - Handled by dfp.h and dfp.cpp
    Serial3.begin(115200);
    Serial4.begin(115200);
    Serial5.begin(SCOMP_BAUD_RATE);  // Scomp Motion (ESP32)
    Serial6.begin(115200);
    Serial7.begin(115200);  // Maestro static for now
    // Serial8 - Handled by SBUS

    Serial4.addMemoryForWrite(Serial4_tx_buf, sizeof(Serial4_tx_buf));
    Serial5.addMemoryForWrite(Serial5_tx_buf, sizeof(Serial5_tx_buf));
    Serial6.addMemoryForWrite(Serial6_tx_buf, sizeof(Serial6_tx_buf));

    scomp.begin(Serial5);
    scomp.onMessage(onScompMessage);

    // Enable watchdog (5 second timeout)
    #if WATCHDOG_ENABLED
    esp_task_wdt_init(WATCHDOG_TIMEOUT_SECONDS, true);
    esp_task_wdt_add(NULL); // add current thread
    #endif

    loadSettingsDefaults();
    loadSettingsWrapper();

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
    static unsigned long millis_lastSbusRead    = now;
    static unsigned long millis_lastPrintAll    = now;
    static unsigned long millis_lastSbusWarn    = now; // tracks last time we emitted a late warning
    static unsigned long millis_lastScompSend = now;
    static unsigned long millis_lastScompHeartbeat = now;

    //static unsigned long millis_maestro = 0; //
    //static uint16_t maestro_position = 6000; //

    // watchdog reset to prevent system hangs, especially important if the SD card is missing or there's an issue with the DFPlayer that could cause blocking calls
    #if WATCHDOG_ENABLED
    esp_task_wdt_reset();
    #endif

    // Scomp update to read incoming messages and trigger callbacks - this should be called every loop tick to ensure timely processing of incoming Scomp messages from the ESP32
    scomp.update();

    // Heartbeat (USB serial)
    if (now - millis_lastHeartbeat >= (HEARTBEAT_INTERVAL_MS + HEARTBEAT_INTERVAL_MS)) { // add a little extra time to ensure this doesn't get too close to the Scomp heartbeat, which is on the same timer
        #if DEBUG_SCOMP_RX
        Serial.printf("Heartbeat: Teensy UP %s | Scomp DOWN | rx bytes=%lu frames=%lu crc_err=%lu sync_drops=%lu\n",
                      formatUptime(now), scomp.rxBytes(), scomp.rxFrames(), scomp.rxCrcErrors(), scomp.rxSyncDrops());
        #else
        Serial.printf("Heartbeat: Teensy UP %s | Scomp DOWN\n", formatUptime(now));
        #endif
        millis_lastHeartbeat = now;
    }

    // Heartbeat (Scomp)
    if (now - millis_lastScompHeartbeat >= HEARTBEAT_INTERVAL_MS) {
        ScompHeartbeat hb = {};
        hb.version   = SCOMP_PROTOCOL_VERSION;
        hb.uptime_ms = now;
        hb.flags     = (sbusHandler.failsafe()  ? SCOMP_FLAG_SBUS_FAILSAFE   : 0)
                     | (sbusHandler.lostFrame() ? SCOMP_FLAG_SBUS_LOST_FRAME : 0);
        scomp.sendHeartbeat(hb);
        millis_lastScompHeartbeat = now;
    }

    // Periodic state push to ESP32
    if (now - millis_lastScompSend >= SCOMP_SEND_INTERVAL_MS) {
        millis_lastScompSend = now;
        scompSendState(now);
    }
    if (now - millis_lastPrintAll >= PRINT_ALL_INTERVAL_MS) {
        printAllChannels();
        millis_lastPrintAll = now;
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

    }

}