// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <Arduino.h>
#include "json/JsonStorage.h"

#define NUM_INPUT_CHANNELS 24     // SBUS may have 16 or 24, we don't really care here which
#define NUM_OUTPUT_CHANNELS 48    // Maximum number of Output channels, currently a uint8_t so 255 is THE maximum

// Kyberpad Screen Fixes Values
#define PAGES 3           // 3 Pages
#define ROWS 3            // 3 rows
#define COLUMNS 5         // 5 columns
#define BUTTON_COUNT 16   // 15 buttons in total on a screen plus a null value for no buttons pressed

enum class iChannelFunction : uint8_t {
    NONE = 0,
    KYBERPAD,
    KYBERPAD_PAGE,
    VOLUME,
    SERVO_DIRECT,
};
struct iFunctionMap {
    const char* name;
    iChannelFunction function;
};
iChannelFunction iLookupFunction(const String& name);

enum class oChannelFunction : uint8_t {
    NONE = 0,
    SERVO_DIRECT,
};
struct oFunctionMap {
    const char* name;
    oChannelFunction function;
};
oChannelFunction oLookupFunction(const String& name);

enum class ChannelType : uint8_t {
    iCHANNEL,
    oCHANNEL
};
#define CH_IN  ChannelType::iCHANNEL
#define CH_OUT ChannelType::oCHANNEL

#define JITTER 20

struct Settings {

    // ---------- AUDIO ----------
    struct Audio {
        uint8_t volume = 15;
        uint8_t min = 0;
        uint8_t initial = 15;
        uint8_t max = 30;
        bool mute = false;
        uint16_t sbus_min = 172;    // The minimum SBUS value that corresponds to volume 0. This is used for mapping the SBUS input range to the volume range.
        uint16_t sbus_max = 1810;   // The maximum SBUS value that corresponds to the maximum volume. This is used for mapping the SBUS input range to the volume range.
        bool dfp_debug = false; // Flag to indicate whether to print DFPlayer-related debug information to the serial console. This can be useful for verifying that audio controls are working as expected, but can be very verbose, so it is disabled by default.

        void reset() { *this = Audio{}; }

        void to_json(JsonObject obj) const {
            obj["volume"]    = volume;
            obj["min"]       = min;
            obj["initial"]   = initial;
            obj["max"]       = max;
            obj["mute"]      = mute;
            obj["sbus_min"]  = sbus_min;
            obj["sbus_max"]  = sbus_max;
            obj["dfp_debug"] = dfp_debug;
        }

        void from_json(JsonObjectConst obj) {
            volume    = obj["volume"]    | volume;
            min       = obj["min"]       | min;
            initial   = obj["initial"]   | initial;
            max       = obj["max"]       | max;
            mute      = obj["mute"]      | mute;
            sbus_min  = obj["sbus_min"]  | sbus_min;
            sbus_max  = obj["sbus_max"]  | sbus_max;
            dfp_debug = obj["dfp_debug"] | dfp_debug;
        }

    } audio;

    // ---------- SYSTEM ----------
    struct System {
        bool debug = false;                 // generic debug flag for miscellaneous serial output
        bool debug_sbus = false;            // debug flag for SBUS input values. If true, will print out all SBUS channel values on each read. Can be very verbose, so use with caution.        
        bool debug_io_mapping = false;      // debug flag for input/output mapping. If true, will print out the channel and value of each SBUS input that triggers a mapping function. Useful for verifying that inputs are being read and mapped correctly.
        bool debug_audio = false;           // debug flag for audio-related output. If true, will print out information related to audio control, such as volume changes. Useful for verifying that audio controls are working as expected.
        uint8_t num_ichannels = NUM_INPUT_CHANNELS;
        uint8_t num_ochannels = NUM_OUTPUT_CHANNELS;

        uint16_t sbus_standard_min = 172;    // Defaults for 100 Percent on the SBUS with 100% Min from the FrSky
        uint16_t sbus_standard_max = 1810;   // Defaults for 100 Percent on the SBUS with 100% Max from the FrSky
        uint16_t sbus_abs_min = 8;           // Defaults for 125 Percent on the SBUS with 125% Min from the FrSky - In theory this is really 0-2047 but I've never observed less than 8 or more than 1976
        uint16_t sbus_abs_max = 1976;        // Defaults for 125 Percent on the SBUS with 125% Max from the FrSky - In theory this is really 0-2047 but I've never observed less than 8 or more than 1976
        uint16_t sbus_jitter = JITTER;       // SBUS jitter value thresholds (kyberpad and switches)

        uint16_t us_standard_min = 992;  // Defaults for the min µs - USed by the FrSky Transmitter screens and Pololu Maestro configuration tool
        uint16_t us_standard_max = 2000; // Defaults for the max µs - USed by the FrSky Transmitter screens and Pololu Maestro configuration tool
        uint16_t us_abs_min = 496;       // Defaults for the absolute min µs - Used by the FrSky Transmitter screens and Pololu Maestro configuration tool
        uint16_t us_abs_max = 2496;      // Defaults for the absolute max µs - Used by the FrSky Transmitter screens and Pololu Maestro configuration tool
        uint16_t us_jitter = JITTER;     // µs jitter value thresholds (kyberpad and switches)

        unsigned long per_channel_serial_output_throttle = 1500; // Per-Channel serial output throttling

        // Two-position switch values; only a global make your switches match (+/- JITTER)
        uint16_t toggle_2_sbus_min = 172;
        uint16_t toggle_2_sbus_max = 1811;
        uint16_t toggle_2_us_min = 988;
        uint16_t toggle_2_us_max = 2012;

        // Three-position switch values; only a global make your switches match (+/- JITTER)
        uint16_t toggle_3_sbus_min = 172;
        uint16_t toggle_3_sbus_mid = 991;
        uint16_t toggle_3_sbus_max = 1811;
        uint16_t toggle_3_us_min = 988;
        uint16_t toggle_3_us_mid = 1500;
        uint16_t toggle_3_us_max = 2012;

        void reset() { *this = System{}; }

        void to_json(JsonObject obj) const {
            // num_ichannels / num_ochannels intentionally omitted (compile-time constants)
            obj["debug"]            = debug;
            obj["debug_sbus"]       = debug_sbus;
            obj["debug_io_mapping"] = debug_io_mapping;
            obj["debug_audio"]      = debug_audio;

            obj["sbus_standard_min"] = sbus_standard_min;
            obj["sbus_standard_max"] = sbus_standard_max;
            obj["sbus_abs_min"]      = sbus_abs_min;
            obj["sbus_abs_max"]      = sbus_abs_max;
            obj["sbus_jitter"]       = sbus_jitter;

            obj["us_standard_min"]   = us_standard_min;
            obj["us_standard_max"]   = us_standard_max;
            obj["us_abs_min"]        = us_abs_min;
            obj["us_abs_max"]        = us_abs_max;
            obj["us_jitter"]         = us_jitter;

            obj["per_channel_serial_output_throttle"] = per_channel_serial_output_throttle;

            obj["toggle_2_sbus_min"] = toggle_2_sbus_min;
            obj["toggle_2_sbus_max"] = toggle_2_sbus_max;
            obj["toggle_2_us_min"]   = toggle_2_us_min;
            obj["toggle_2_us_max"]   = toggle_2_us_max;

            obj["toggle_3_sbus_min"] = toggle_3_sbus_min;
            obj["toggle_3_sbus_mid"] = toggle_3_sbus_mid;
            obj["toggle_3_sbus_max"] = toggle_3_sbus_max;
            obj["toggle_3_us_min"]   = toggle_3_us_min;
            obj["toggle_3_us_mid"]   = toggle_3_us_mid;
            obj["toggle_3_us_max"]   = toggle_3_us_max;
        }

        void from_json(JsonObjectConst obj) {
            debug            = obj["debug"]            | debug;
            debug_sbus       = obj["debug_sbus"]       | debug_sbus;
            debug_io_mapping = obj["debug_io_mapping"] | debug_io_mapping;
            debug_audio      = obj["debug_audio"]      | debug_audio;

            sbus_standard_min = obj["sbus_standard_min"] | sbus_standard_min;
            sbus_standard_max = obj["sbus_standard_max"] | sbus_standard_max;
            sbus_abs_min      = obj["sbus_abs_min"]      | sbus_abs_min;
            sbus_abs_max      = obj["sbus_abs_max"]      | sbus_abs_max;
            sbus_jitter       = obj["sbus_jitter"]       | sbus_jitter;

            us_standard_min   = obj["us_standard_min"]   | us_standard_min;
            us_standard_max   = obj["us_standard_max"]   | us_standard_max;
            us_abs_min        = obj["us_abs_min"]         | us_abs_min;
            us_abs_max        = obj["us_abs_max"]         | us_abs_max;
            us_jitter         = obj["us_jitter"]          | us_jitter;

            per_channel_serial_output_throttle =
                obj["per_channel_serial_output_throttle"] | per_channel_serial_output_throttle;

            toggle_2_sbus_min = obj["toggle_2_sbus_min"] | toggle_2_sbus_min;
            toggle_2_sbus_max = obj["toggle_2_sbus_max"] | toggle_2_sbus_max;
            toggle_2_us_min   = obj["toggle_2_us_min"]   | toggle_2_us_min;
            toggle_2_us_max   = obj["toggle_2_us_max"]   | toggle_2_us_max;

            toggle_3_sbus_min = obj["toggle_3_sbus_min"] | toggle_3_sbus_min;
            toggle_3_sbus_mid = obj["toggle_3_sbus_mid"] | toggle_3_sbus_mid;
            toggle_3_sbus_max = obj["toggle_3_sbus_max"] | toggle_3_sbus_max;
            toggle_3_us_min   = obj["toggle_3_us_min"]   | toggle_3_us_min;
            toggle_3_us_mid   = obj["toggle_3_us_mid"]   | toggle_3_us_mid;
            toggle_3_us_max   = obj["toggle_3_us_max"]   | toggle_3_us_max;
        }

    } system;

    struct iChannel {         // Input Channels, from the user (programmatic scripts write directly to the output channels)
        bool enabled = false; // Flag to indicate if this channel is enabled for processing. This can be used to ignore certain channels without having to change the code logic for reading SBUS input.
        bool updated = false; // Run-Time only, has this input changed from the last iteration
        bool serial_debug_output = false;     // per-channel messaging of values on the serial port

        uint16_t sbus_value = 0; // Run-Time only, The raw SBUS value read from the channel. This is used for debugging and mapping purposes.
        uint16_t sbus_min = 0; // Only used if > 0 - The minimum SBUS value for this channel, values below this will be treated as min
        uint16_t sbus_max = 0; // Only used if > 0 - The maximum SBUS value for this channel, values above this will be treated as max
        bool sbus_abs_minmax = false; // Default is to use the standard values, this triggers the abs (absolute min/max) values; manual min/max below overrides this entirely

        uint16_t us_value = 0; // Run-Time only, The converted µs value - Used by the FrSky Transmitter screens and Pololu Maestro configuration tool
        uint16_t us_min = 0;  // The minimum µs value for this channel - Used by the FrSky Transmitter screens and Pololu Maestro configuration tool
        uint16_t us_max = 0;  // The maximum µs value for this channel - Used by the FrSky Transmitter screens and Pololu Maestro configuration tool
        bool us_abs_minmax = false; // Default is to use the standard values, this triggers the abs (absolute min/max) values; manual min/max below overrides this entirely

        uint8_t print_jitter = 10; // Minimum change in SBUS value to report when debugging. This is used to prevent spamming the serial output with minor fluctuations in SBUS input.
        String description;
        iChannelFunction channelFunction = iChannelFunction::SERVO_DIRECT; // The name of the function that should be called to handle changes in this channel's value. This is used to map SBUS input to specific output functions.

        void reset() { *this = iChannel{}; }

        // Runtime-only fields omitted: updated, sbus_value, us_value
        void to_json(JsonObject obj) const {
            obj["enabled"]         = enabled;
            obj["serial_debug"]    = serial_debug_output;
            obj["sbus_min"]        = sbus_min;
            obj["sbus_max"]        = sbus_max;
            obj["sbus_abs_minmax"] = sbus_abs_minmax;
            obj["us_min"]          = us_min;
            obj["us_max"]          = us_max;
            obj["us_abs_minmax"]   = us_abs_minmax;
            obj["print_jitter"]    = print_jitter;
            obj["description"]     = description.c_str();
            obj["function"]        = static_cast<int>(channelFunction);
        }

        void from_json(JsonObjectConst obj) {
            enabled              = obj["enabled"]         | enabled;
            serial_debug_output  = obj["serial_debug"]    | serial_debug_output;
            sbus_min             = obj["sbus_min"]        | sbus_min;
            sbus_max             = obj["sbus_max"]        | sbus_max;
            sbus_abs_minmax      = obj["sbus_abs_minmax"] | sbus_abs_minmax;
            us_min               = obj["us_min"]          | us_min;
            us_max               = obj["us_max"]          | us_max;
            us_abs_minmax        = obj["us_abs_minmax"]   | us_abs_minmax;
            print_jitter         = obj["print_jitter"]    | print_jitter;
            channelFunction      = static_cast<iChannelFunction>(
                                       obj["function"]    | static_cast<int>(channelFunction));
            if (obj["description"].is<const char*>())
                description      = obj["description"].as<const char*>();
        }

    };

    iChannel ichannel[NUM_INPUT_CHANNELS];

    struct oChannel {         // Input Channels, from the user (programmatic scripts write directly to the output channels)
        bool enabled = false; // Flag to indicate if this channel is enabled for processing. This can be used to ignore certain channels without having to change the code logic for reading SBUS input.
        bool updated = false; // Run-Time only, has this input changed from the last iteration
        bool serial_debug_output = false;     // per-channel messaging of values on the serial port

        bool ichannel_enabled = false;  // Use an input channel to define the output channel value
        uint8_t ichannel = 0;           // the ichannel number (0-indexed)

        uint16_t us_value = 0; // Run-Time only, The converted µs value - Used by the FrSky Transmitter screens and Pololu Maestro configuration tool
        uint16_t us_min = 0;  // The minimum µs value for this channel - Used by the FrSky Transmitter screens and Pololu Maestro configuration tool
        uint16_t us_max = 0;  // The maximum µs value for this channel - Used by the FrSky Transmitter screens and Pololu Maestro configuration tool
        bool us_abs_minmax = false; // Default is to use the standard values, this triggers the abs (absolute min/max) values; manual min/max below overrides this entirely

        uint8_t serial_port_out = 0;
        uint8_t maestro_id = 0;
        uint8_t maestro_ch = 0;

        uint8_t print_jitter = 10; // Minimum change in value to report when debugging
        String description;
        oChannelFunction channelFunction = oChannelFunction::SERVO_DIRECT;; // The name of the function that should be called to handle changes in this channel's value. This is used to map SBUS input to specific output functions.

        void reset() { *this = oChannel{}; }

        // Runtime-only fields omitted: updated, us_value
        void to_json(JsonObject obj) const {
            obj["enabled"]          = enabled;
            obj["serial_debug"]     = serial_debug_output;
            obj["ichannel_enabled"] = ichannel_enabled;
            obj["ichannel"]         = ichannel;
            obj["us_min"]           = us_min;
            obj["us_max"]           = us_max;
            obj["us_abs_minmax"]    = us_abs_minmax;
            obj["serial_port_out"]  = serial_port_out;
            obj["maestro_id"]       = maestro_id;
            obj["maestro_ch"]       = maestro_ch;
            obj["print_jitter"]     = print_jitter;
            obj["description"]      = description.c_str();
            obj["function"]         = static_cast<int>(channelFunction);
        }

        void from_json(JsonObjectConst obj) {
            enabled              = obj["enabled"]          | enabled;
            serial_debug_output  = obj["serial_debug"]     | serial_debug_output;
            ichannel_enabled     = obj["ichannel_enabled"] | ichannel_enabled;
            ichannel             = obj["ichannel"]         | ichannel;
            us_min               = obj["us_min"]           | us_min;
            us_max               = obj["us_max"]           | us_max;
            us_abs_minmax        = obj["us_abs_minmax"]    | us_abs_minmax;
            serial_port_out      = obj["serial_port_out"]  | serial_port_out;
            maestro_id           = obj["maestro_id"]       | maestro_id;
            maestro_ch           = obj["maestro_ch"]       | maestro_ch;
            print_jitter         = obj["print_jitter"]     | print_jitter;
            channelFunction      = static_cast<oChannelFunction>(
                                       obj["function"]     | static_cast<int>(channelFunction));
            if (obj["description"].is<const char*>())
                description      = obj["description"].as<const char*>();
        }

    };

    oChannel ochannel[NUM_OUTPUT_CHANNELS];

    struct Kyperpad {
        uint8_t page = 0;  // Run-Time only, 

        uint8_t pages = PAGES;
        uint8_t rows = ROWS;
        uint8_t columns = COLUMNS;
        uint8_t button_count = BUTTON_COUNT;

        uint16_t sbus_value_null = 172; // The SBUS value that corresponds to no buttons being pressed on the KyberPad. This is used to determine when no buttons are active based on the SBUS input.
        //uint16_t sbus_value_null_min = sbus_value_null - JITTER;
        //uint16_t sbus_value_null_max = sbus_value_null + JITTER;
        uint16_t kyberpad_debounce = 300; // Kyberpad sends the channel selection for approx 250ms, so ignore it for that long after a change is detected to prevent spamming the command actions

        boolean serial_print_button_mapping = false; // Flag to indicate whether to print the mapping of SBUS values to KyberPad buttons to the serial console. This can be useful for debugging and verifying that the button mappings are correct, but can be very verbose, so it is disabled by default.

        void reset() { *this = Kyperpad{}; }

        // Runtime-only omitted: page
        // Compile-time constants omitted: pages, rows, columns, button_count
        void to_json(JsonObject obj) const {
            obj["sbus_value_null"]             = sbus_value_null;
            obj["kyberpad_debounce"]           = kyberpad_debounce;
            obj["serial_print_button_mapping"] = serial_print_button_mapping;
        }

        void from_json(JsonObjectConst obj) {
            sbus_value_null             = obj["sbus_value_null"]             | sbus_value_null;
            kyberpad_debounce           = obj["kyberpad_debounce"]           | kyberpad_debounce;
            serial_print_button_mapping = obj["serial_print_button_mapping"] | serial_print_button_mapping;
        }
    } kyberpad;

    // We record the NULL/No button pressed value at 0, then buttons 1-15
    struct Kyperpadbuttonvalues {
        uint16_t sbus_value = 0; // The SBUS value that corresponds to this button being pressed. This is used to determine which button is currently active based on the SBUS input.
    } kyperpadbuttonvalues[BUTTON_COUNT];

    struct KyberpadButton {
        String description;              // A human-readable description of the button's function. This is used for debugging and can be used in the future for dynamic mapping of button functions.
        uint16_t audio_file_start = 0;   // The starting audio file number associated with this button. This is used to determine which audio file to play when the button is pressed, allowing for multiple buttons to be mapped to different audio files or sets of audio files.
        uint16_t audio_file_end = 0;     // The ending audio file number associated with this button. This is used in conjunction with audio_file_start to determine the range of audio files to play when the button is pressed, allowing for multiple buttons to be mapped to different audio files or sets of audio files.
        uint16_t audio_file_current = 0; // Runtime-only, The current audio file number to play for this button. This is used to keep track of which audio file in the specified range should be played next when the button is pressed, allowing for cycling through a set of audio files for each button.
        bool audio_randomize = false;    // Flag to indicate whether to randomize audio file selection for this button. If true, a random audio file from the specified range will be played each time the button is pressed. If false, audio files will be played in sequential order based on audio_file_current.
        bool kyberpad_stop = false;      // Flag to indicate if this channel should stop further processing of KyberPad button inputs when a change is detected. This can be used to prevent certain channels from triggering button actions, such as the page selector channel which can cause unintended button presses when changing pages due to overlapping SBUS value ranges.
        bool led_scomp_link = true;      // Is this running on the Scomp-Link serial port? Or does it pass it to the relays via WiFi?
        uint8_t led_bank = 0;            // Scomp-Link or Scomp-Relay LED Bank Number, 0-indexed
        uint16_t led_sequence = 0;       // Scomp-Link LED Sequence Number. This is used to trigger specific LED patterns on the Scomp-Link when the button is pressed, allowing for visual feedback and indication of button presses and actions.

        void reset() { *this = KyberpadButton{}; }

        // Runtime-only omitted: audio_file_current
        void to_json(JsonObject obj) const {
            obj["description"]      = description.c_str();
            obj["audio_file_start"] = audio_file_start;
            obj["audio_file_end"]   = audio_file_end;
            obj["audio_randomize"]  = audio_randomize;
            obj["kyberpad_stop"]    = kyberpad_stop;
            obj["led scomp link"]   = led_scomp_link;
            obj["led bank"]         = led_bank;
            obj["led sequence"]     = led_sequence;
        }

        void from_json(JsonObjectConst obj) {
            audio_file_start = obj["audio_file_start"] | audio_file_start;
            audio_file_end   = obj["audio_file_end"]   | audio_file_end;
            audio_randomize  = obj["audio_randomize"]  | audio_randomize;
            kyberpad_stop    = obj["kyberpad_stop"]    | kyberpad_stop;
            if (obj["description"].is<const char*>())
                description  = obj["description"].as<const char*>();
            led_scomp_link   = obj["led scomp link"]   | led_scomp_link;
            led_bank         = obj["led bank"]         | led_bank;
            led_sequence     = obj["led sequence"]     | led_sequence;
        }
    } kyberpadbuttons[PAGES][ROWS][COLUMNS];

};

// Global instance (like a Python singleton dict)
extern Settings settings;

void loadSettingsDefaults();
void resetSettingsDefaults();
bool settingsLoad(const char *path, Settings &cfg);
bool settingsSave(const char *path, const Settings &cfg, bool pretty = true);
void settingsBackup(const char *basePath, const Settings &cfg, uint8_t maxVersions = 3);

uint16_t sbusGetMin(uint8_t channel);
uint16_t sbusGetMax(uint8_t channel);
uint16_t usGetMin(ChannelType type, uint8_t channel);
uint16_t usGetMax(ChannelType type, uint8_t channel);
void printChannel(ChannelType type, uint8_t channel);
void ioPrintChannel(uint8_t ochannel, uint8_t ichannel);
void printChannelUs(ChannelType type);
