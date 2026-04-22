#include "settings.h"
#include "dfplayer/dfp.h"


void ioKyberpadButtonExecute(int page, int row, int column) {
    Settings::KyberpadButton button = settings.kyberpadbuttons[page][row][column];
    if (button.kyberpad_stop) {
        Serial.printf("KyberPad: STOP Page %d Row %d Column %d\n", page + 1, row + 1, column + 1);
        dfpStop(); // Stop any currently playing audio to prevent overlapping audio files when changing pages or pressing buttons that are set to stop further processing of button inputs
        return; // Exit the function if this button is set to stop further processing of button inputs
    }
    if (button.audio_file_start == 0) {
        Serial.printf("KyberPad: Button on Page %d Row %d Column %d has no audio file mapped, skipping execution\n", page + 1, row + 1, column + 1);
        return; // Exit the function if there is no audio file mapped to this button
    }
    if (button.audio_randomize) {
        if (button.audio_file_end < button.audio_file_start) {
            button.audio_file_current = random(button.audio_file_start, button.audio_file_end);
            Serial.printf("KyberPad: Randomizing audio file for Button on Page %d Row %d Column %d - audio_file_start: %d, audio_file_end: %d, selected audio_file_current: %d\n", page + 1, row + 1, column + 1, button.audio_file_start, button.audio_file_end, button.audio_file_current);
        } else {
            // invalid random sequence, just play the start file
            Serial.printf("KyberPad: Invalid audio file range for Button on Page %d Row %d Column %d - audio_file_end (%d) is less than audio_file_start (%d)\n", page + 1, row + 1, column + 1, button.audio_file_end, button.audio_file_start);
            button.audio_file_current = button.audio_file_start;
        }
    }
    if (button.audio_file_current == 0) {
        button.audio_file_current = button.audio_file_start; // Initialize the current audio file to the start of the range if it hasn't been set yet
    }
    if (button.audio_file_current > button.audio_file_end) {
        button.audio_file_current = button.audio_file_start; // Loop back to the start of the range if we've exceeded the end
    }

    Serial.printf("KyberPad: Executing Button on Page %d Row %d Column %d - Playing Audio File %d\n", page + 1, row + 1, column + 1, button.audio_file_current);
    dfpPlay(button.audio_file_current); // Play the selected audio file for this button
    button.audio_file_current++;
    settings.kyberpadbuttons[page][row][column].audio_file_current = button.audio_file_current; // Update the current audio file for this button in the settings
}

void ioKyberpadPage(int channel, int16_t value) {
    //Serial.printf("kyberpadPage: set to %d\n", value);
    if (value >= settings.kyberpad.page_0 - settings.kyberpad.jitter && value <= settings.kyberpad.page_0 + settings.kyberpad.jitter) {
        if (settings.kyberpad.page != settings.kyberpad.page_0) {
            Serial.println("KyberPad: Page 1");
            settings.kyberpad.page = settings.kyberpad.page_0;
            settings.kyberpad.page_human = 1;
            settings.channel[channel].value = settings.kyberpad.page_0; // Update the channel value to the locked page value to prevent jitter around the page selector threshold
        }
    } else if (value >= settings.kyberpad.page_1 - settings.kyberpad.jitter && value <= settings.kyberpad.page_1 + settings.kyberpad.jitter) {
        if (settings.kyberpad.page != settings.kyberpad.page_1) {
            Serial.println("KyberPad: Page 2");
            settings.kyberpad.page = settings.kyberpad.page_1;
            settings.kyberpad.page_human = 2;
            settings.channel[channel].value = settings.kyberpad.page_1; // Update the channel value to the locked page value to prevent jitter around the page selector threshold
        }
    } else if (value >= settings.kyberpad.page_2 - settings.kyberpad.jitter && value <= settings.kyberpad.page_2 + settings.kyberpad.jitter) {
        if (settings.kyberpad.page != settings.kyberpad.page_2) {
            Serial.println("KyberPad: Page 3");
            settings.kyberpad.page = settings.kyberpad.page_2;
            settings.kyberpad.page_human = 3;
            settings.channel[channel].value = settings.kyberpad.page_2; // Update the channel value to the locked page value to prevent jitter around the page selector threshold
        }
    } else {
        Serial.println("KyberPad: Invalid page value: " + String(value));
    }
}

void ioKyberpadButtons(int channel, int16_t value, unsigned long now) {
    static uint32_t millis_lastKyberDebounce = 0;

    // Null value for the KyberPad, do nothing and get out of here quickly
    if (value >= settings.kyberpad.sbus_value_null_min && value <= settings.kyberpad.sbus_value_null_max) {
        return; 
    }

    // Kyberpad LUA script sends the button press value for approx 250ms, so we only want to process it once per change and ignore it for that duration to prevent spamming the button actions
    if (now - millis_lastKyberDebounce >= settings.kyberpad.kyberpad_debounce) {
        // We're actually GOOD to process and continue, so update the debounce timer to now to prevent processing again until the debounce duration has passed
        millis_lastKyberDebounce = now;
    } else {
        // We're inside the debouce window, return without processing to prevent spamming the button actions
        return;
    }

    int kyber_button_index = -1;
    for (int button_index = 0; button_index <= settings.kyberpad.button_count; button_index++) {
        int sbus_min = settings.Kyperpadbuttonvalues[button_index].sbus_value - settings.kyberpad.jitter;
        int sbus_max = settings.Kyperpadbuttonvalues[button_index].sbus_value + settings.kyberpad.jitter;
        if (value >= sbus_min && value <= sbus_max) {
            kyber_button_index = button_index;
            int page = settings.kyberpad.page_human - 1;
            int row = button_index / 5;
            int column = button_index % 5;
            if (settings.system.debug_io_mapping) {
                Serial.printf("KyberPad: Page %d Row %d Column %d Pressed\n", page + 1, row + 1, column + 1);
            }
            break; // Exit the loop once we've found the matching button value
        }
    }

    if (kyber_button_index == -1) {
        Serial.println("KyberPad: No button detected with SBUS value: " + String(value));
        return; // Exit the function if no button was detected
    }

    ioKyberpadButtonExecute(settings.kyberpad.page_human - 1, kyber_button_index / 5, kyber_button_index % 5); // Execute the button action based on the current page and the detected button index
}

