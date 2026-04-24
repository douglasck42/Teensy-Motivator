#pragma once
#include <Arduino.h>

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

    } system;

    struct iChannel {         // Input Channels, from the user (programmatic scripts write directly to the output channels)
        bool enabled = false; // Flag to indicate if this channel is enabled for processing. This can be used to ignore certain channels without having to change the code logic for reading SBUS input.
        bool updated = false; // has this input changed from the last iteration
        bool serial_debug_output = false;     // per-channel messaging of values on the serial port

        uint16_t sbus_value = 0; // The raw SBUS value read from the channel. This is used for debugging and mapping purposes.
        uint16_t sbus_min = 0; // Only used if > 0 - The minimum SBUS value for this channel, values below this will be treated as min
        uint16_t sbus_max = 0; // Only used if > 0 - The maximum SBUS value for this channel, values above this will be treated as max
        bool sbus_abs_minmax = false; // Default is to use the standard values, this triggers the abs (absolute min/max) values; manual min/max below overrides this entirely

        uint16_t us_value = 0; // The converted µs value - Used by the FrSky Transmitter screens and Pololu Maestro configuration tool
        uint16_t us_min = 0;  // The minimum µs value for this channel - Used by the FrSky Transmitter screens and Pololu Maestro configuration tool
        uint16_t us_max = 0;  // The maximum µs value for this channel - Used by the FrSky Transmitter screens and Pololu Maestro configuration tool
        bool us_abs_minmax = false; // Default is to use the standard values, this triggers the abs (absolute min/max) values; manual min/max below overrides this entirely

        uint8_t print_jitter = 10; // Minimum change in SBUS value to report when debugging. This is used to prevent spamming the serial output with minor fluctuations in SBUS input.
        String description;
        iChannelFunction channelFunction = iChannelFunction::SERVO_DIRECT; // The name of the function that should be called to handle changes in this channel's value. This is used to map SBUS input to specific output functions.
    };

    iChannel ichannel[NUM_INPUT_CHANNELS];

    struct oChannel {         // Input Channels, from the user (programmatic scripts write directly to the output channels)
        bool enabled = false; // Flag to indicate if this channel is enabled for processing. This can be used to ignore certain channels without having to change the code logic for reading SBUS input.
        bool updated = false; // has this input changed from the last iteration
        bool serial_debug_output = false;     // per-channel messaging of values on the serial port

        bool ichannel_enabled = false;  // Use an input channel to define the output channel value
        uint8_t ichannel = 0;           // the ichannel number (0-indexed)

        uint16_t us_value = 0; // The converted µs value - Used by the FrSky Transmitter screens and Pololu Maestro configuration tool
        uint16_t us_min = 0;  // The minimum µs value for this channel - Used by the FrSky Transmitter screens and Pololu Maestro configuration tool
        uint16_t us_max = 0;  // The maximum µs value for this channel - Used by the FrSky Transmitter screens and Pololu Maestro configuration tool
        bool us_abs_minmax = false; // Default is to use the standard values, this triggers the abs (absolute min/max) values; manual min/max below overrides this entirely

        uint8_t serial_port_out = 0;
        uint8_t maestro_id = 0;
        uint8_t maestro_ch = 0;

        uint8_t print_jitter = 10; // Minimum change in value to report when debugging
        String description;
        oChannelFunction channelFunction = oChannelFunction::SERVO_DIRECT;; // The name of the function that should be called to handle changes in this channel's value. This is used to map SBUS input to specific output functions.
    };

    oChannel ochannel[NUM_OUTPUT_CHANNELS];

    struct Kyperpad {
        uint8_t page = 0;

        uint8_t pages = PAGES;
        uint8_t rows = ROWS;
        uint8_t columns = COLUMNS;
        uint8_t button_count = BUTTON_COUNT;

        uint16_t sbus_value_null = 172; // The SBUS value that corresponds to no buttons being pressed on the KyberPad. This is used to determine when no buttons are active based on the SBUS input.
        //uint16_t sbus_value_null_min = sbus_value_null - JITTER;
        //uint16_t sbus_value_null_max = sbus_value_null + JITTER;
        uint16_t kyberpad_debounce = 300; // Kyberpad sends the channel selection for approx 250ms, so ignore it for that long after a change is detected to prevent spamming the command actions

        boolean serial_print_button_mapping = false; // Flag to indicate whether to print the mapping of SBUS values to KyberPad buttons to the serial console. This can be useful for debugging and verifying that the button mappings are correct, but can be very verbose, so it is disabled by default.
    } kyberpad;

    // We record the NULL/No button pressed value at 0, then buttons 1-15
    struct Kyperpadbuttonvalues {
        uint16_t sbus_value = 0; // The SBUS value that corresponds to this button being pressed. This is used to determine which button is currently active based on the SBUS input.
    } Kyperpadbuttonvalues[BUTTON_COUNT];

    struct KyberpadButton {
        String description; // A human-readable description of the button's function. This is used for debugging and can be used in the future for dynamic mapping of button functions.
        uint16_t audio_file_start = 0; // The starting audio file number associated with this button. This is used to determine which audio file to play when the button is pressed, allowing for multiple buttons to be mapped to different audio files or sets of audio files.
        uint16_t audio_file_end = 0; // The ending audio file number associated with this button. This is used in conjunction with audio_file_start to determine the range of audio files to play when the button is pressed, allowing for multiple buttons to be mapped to different audio files or sets of audio files.
        uint16_t audio_file_current = 0; // The current audio file number to play for this button. This is used to keep track of which audio file in the specified range should be played next when the button is pressed, allowing for cycling through a set of audio files for each button.
        bool audio_randomize = false; // Flag to indicate whether to randomize audio file selection for this button. If true, a random audio file from the specified range will be played each time the button is pressed. If false, audio files will be played in sequential order based on audio_file_current.
        bool kyberpad_stop = false; // Flag to indicate if this channel should stop further processing of KyberPad button inputs when a change is detected. This can be used to prevent certain channels from triggering button actions, such as the page selector channel which can cause unintended button presses when changing pages due to overlapping SBUS value ranges.
    } kyberpadbuttons[PAGES][ROWS][COLUMNS];

};

// Global instance (like a Python singleton dict)
extern Settings settings;

void loadSettings();
void saveSettings();
void resetSettingsDefaults();
uint16_t sbusGetMin(uint8_t channel);
uint16_t sbusGetMax(uint8_t channel);
uint16_t usGetMin(ChannelType type, uint8_t channel);
uint16_t usGetMax(ChannelType type, uint8_t channel);
void printChannel(ChannelType type, uint8_t channel);
