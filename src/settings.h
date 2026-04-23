#pragma once
#include <Arduino.h>

#define NUM_CHANNELS 24     // SBUS may have 16 or 24, we don't really care here which

// Kyberpad Screen Fixes Values
#define PAGES 3           // 3 Pages
#define ROWS 3            // 3 rows
#define COLUMNS 5         // 5 columns
#define BUTTON_COUNT 16   // 15 buttons in total on a screen plus a null value for no buttons pressed

struct Settings {

    // ---------- AUDIO ----------
    struct Audio {
        int volume = 15;
        int min = 0;
        int initial = 15;
        int max = 30;
        bool mute = false;
        int sbus_min = 586;    // The minimum SBUS value that corresponds to volume 0. This is used for mapping the SBUS input range to the volume range.
        int sbus_max = 1811;   // The maximum SBUS value that corresponds to the maximum volume. This is used for mapping the SBUS input range to the volume range.
        bool dfp_debug = false; // Flag to indicate whether to print DFPlayer-related debug information to the serial console. This can be useful for verifying that audio controls are working as expected, but can be very verbose, so it is disabled by default.
    } audio;

    // ---------- SYSTEM ----------
    struct System {
        bool debug = false;                 // generic debug flag for miscellaneous serial output
        bool debug_sbus = false;            // debug flag for SBUS input values. If true, will print out all SBUS channel values on each read. Can be very verbose, so use with caution.        
        bool debug_io_mapping = false;      // debug flag for input/output mapping. If true, will print out the channel and value of each SBUS input that triggers a mapping function. Useful for verifying that inputs are being read and mapped correctly.
        bool debug_audio = false;           // debug flag for audio-related output. If true, will print out information related to audio control, such as volume changes. Useful for verifying that audio controls are working as expected.
        int num_channels = NUM_CHANNELS;
    } system;

    struct Channel {
        bool enabled = false; // Flag to indicate if this channel is enabled for processing. This can be used to ignore certain channels without having to change the code logic for reading SBUS input.
        int value = 0; // The raw SBUS value read from the channel. This is used for debugging and mapping purposes.
        int min = 100; // The minimum SBUS value for this channel, values below this will be treated as min
        int max = 2100; // The maximum SBUS value for this channel, values above this will be treated as max
        int min_us = 992;  // The minimum µs value for this channel, per the Maestro GUI
        int max_us = 2000; // The maximum µs value for this channel, per the Maestro GUI
        int print_jitter = 10; // Minimum change in SBUS value to report when debugging. This is used to prevent spamming the serial output with minor fluctuations in SBUS input.
        String description;
        String short_description;
        String output_function; // The name of the function that should be called to handle changes in this channel's value. This is used to map SBUS input to specific output functions.
        bool kyberpad_channel = false; // Flag to indicate if this channel is used for KyberPad button input. This is used to determine if the channel value should be processed as KyberPad button states.
        bool kyberpad_page_channel = false; // Flag to indicate if this channel is used for KyberPad page selector input. This is used to determine if the channel value should be processed as KyberPad page states.
        bool volume_channel = false; // Flag to indicate if this channel is used for volume control input. This is used to determine if the channel value should be processed as volume control changes.
    };

    Channel channel[NUM_CHANNELS];

    struct Kyperpad {
        int page_0 = 172;  // The SBUS value that corresponds to page 0 on the KyberPad. This is used to determine which page is currently active based on the SBUS input.
        int page_1 = 991;  // The SBUS value that corresponds to page 1 on the KyberPad. This is used to determine which page is currently active based on the SBUS input.
        int page_2 = 1811; // The SBUS value that corresponds to page 2 on the KyberPad. This is used to determine which page is currently active based on the SBUS input.

        int jitter = 20; // SBUS jitter value threshold for the page selector and software buttons. +/- above values will lock into the above values and the screen values
        int page = page_0; // The current page of the KyberPad, determined by the SBUS input value. This is used to determine which set of button states to read from the SBUS input and how to interpret them.
        int page_human = 1; // Human readable page number

        int pages = PAGES;
        int rows = ROWS;
        int columns = COLUMNS;
        int button_count = BUTTON_COUNT;

        int sbus_value_null = 172; // The SBUS value that corresponds to no buttons being pressed on the KyberPad. This is used to determine when no buttons are active based on the SBUS input.
        int sbus_value_null_min = 172 - 20;
        int sbus_value_null_max = 172 + 20;
        unsigned long kyberpad_debounce = 300; // Kyberpad sends the channel selection for approx 250ms, so ignore it for that long after a change is detected to prevent spamming the command actions

        boolean serial_print_button_mapping = false; // Flag to indicate whether to print the mapping of SBUS values to KyberPad buttons to the serial console. This can be useful for debugging and verifying that the button mappings are correct, but can be very verbose, so it is disabled by default.
    } kyberpad;

    // We record the NULL/No button pressed value at 0, then buttons 1-15
    struct Kyperpadbuttonvalues {
        int sbus_value = 0; // The SBUS value that corresponds to this button being pressed. This is used to determine which button is currently active based on the SBUS input.
    } Kyperpadbuttonvalues[BUTTON_COUNT];

    struct KyberpadButton {
        String description; // A human-readable description of the button's function. This is used for debugging and can be used in the future for dynamic mapping of button functions.
        int audio_file_start = 0; // The starting audio file number associated with this button. This is used to determine which audio file to play when the button is pressed, allowing for multiple buttons to be mapped to different audio files or sets of audio files.
        int audio_file_end = 0; // The ending audio file number associated with this button. This is used in conjunction with audio_file_start to determine the range of audio files to play when the button is pressed, allowing for multiple buttons to be mapped to different audio files or sets of audio files.
        int audio_file_current = 0; // The current audio file number to play for this button. This is used to keep track of which audio file in the specified range should be played next when the button is pressed, allowing for cycling through a set of audio files for each button.
        bool audio_randomize = false; // Flag to indicate whether to randomize audio file selection for this button. If true, a random audio file from the specified range will be played each time the button is pressed. If false, audio files will be played in sequential order based on audio_file_current.
        bool kyberpad_stop = false; // Flag to indicate if this channel should stop further processing of KyberPad button inputs when a change is detected. This can be used to prevent certain channels from triggering button actions, such as the page selector channel which can cause unintended button presses when changing pages due to overlapping SBUS value ranges.
    } kyberpadbuttons[PAGES][ROWS][COLUMNS];

};

// Global instance (like a Python singleton dict)
extern Settings settings;

void loadSettings();
void saveSettings();
void resetSettingsDefaults();
