#include "settings.h"
#include <LittleFS.h>
//#include <ArduinoJson.h>
#define zIdx(x) ((x) - 1)

Settings settings;

//static const char* FILE_PATH = "/config.json";

// -------------------- DEFAULTS --------------------
void resetSettingsDefaults() {
    settings = Settings(); // resets to struct defaults
}

// -------------------- LOAD --------------------
void loadSettings() {
    Serial.println("Settings: loadSettings()");

    Serial.printf("  Default volume: %d\n", settings.audio.volume);
    Serial.printf("  Debug SBUS: %s\n", settings.system.debug_sbus ? "ON" : "OFF");

    settings.system.debug_io_mapping = true;

    Serial.println("Configuring KyberPad settings...");
    // Human Channel 22, SBUS Channel 21 (0-indexed in code), is the volume control, which is mapped to a continuous range of volume levels from 0 to 30.
    settings.channel[22 - 1].enabled = true;
    settings.channel[22 - 1].description = "Volume";
    settings.channel[22 - 1].short_description = "Vol";
    settings.channel[22 - 1].volume_channel = true;
    settings.channel[22 - 1].min = 172;
    settings.channel[22 - 1].max = 1811;

    // Human Channel 23, SBUS Channel 22 (0-indexed in code), is the KyberPad page selector, which uses the same SBUS range as volume control but is mapped to discrete page states instead of a continuous volume level.
    settings.channel[23 - 1].enabled = true;
    settings.channel[23 - 1].description = "Kyperpad Page Toggle";
    settings.channel[23 - 1].short_description = "kPg";
    settings.channel[23 - 1].kyberpad_page_channel = true;
    settings.channel[23 - 1].min = 172;
    settings.channel[23 - 1].max = 1811;

    // Human Channel 24, SBUS Channel 23 (0-indexed in code), is the KyberPad software buttons themselves
    settings.channel[24 - 1].enabled = true;
    settings.channel[24 - 1].description = "Kyperpad Software Buttons";
    settings.channel[24 - 1].short_description = "kSb";
    settings.channel[24 - 1].kyberpad_channel = true;
    settings.channel[24 - 1].min = 172;
    settings.channel[24 - 1].max = 1811;

    Serial.println("Configuring KyberPad Button Channels...");
    //settings.Kyperpadbuttonvalues[0].sbus_value = 172;   // No buttons pressed value
    settings.Kyperpadbuttonvalues[0].sbus_value = 274;   // Row 1 Column 1
    settings.Kyperpadbuttonvalues[1].sbus_value = 376;   // Row 1 Column 2
    settings.Kyperpadbuttonvalues[2].sbus_value = 478;   // Row 1 Column 3
    settings.Kyperpadbuttonvalues[3].sbus_value = 580;   // Row 1 Column 4
    settings.Kyperpadbuttonvalues[4].sbus_value = 682;   // Row 1 Column 5
    settings.Kyperpadbuttonvalues[5].sbus_value = 784;   // Row 2 Column 1
    settings.Kyperpadbuttonvalues[6].sbus_value = 886;   // Row 2 Column 2
    settings.Kyperpadbuttonvalues[7].sbus_value = 988;   // Row 2 Column 3
    settings.Kyperpadbuttonvalues[8].sbus_value = 1090;  // Row 2 Column 4
    settings.Kyperpadbuttonvalues[9].sbus_value = 1192; // Row 2 Column 5
    settings.Kyperpadbuttonvalues[10].sbus_value = 1294; // Row 3 Column 1
    settings.Kyperpadbuttonvalues[11].sbus_value = 1396; // Row 3 Column 2
    settings.Kyperpadbuttonvalues[12].sbus_value = 1498; // Row 3 Column 3
    settings.Kyperpadbuttonvalues[13].sbus_value = 1600; // Row 3 Column 4
    settings.Kyperpadbuttonvalues[14].sbus_value = 1702; // Row 3 Column 5


    Serial.println("Configuring KyberPad STOP Buttons...");
    settings.kyberpadbuttons[zIdx(1)][zIdx(3)][zIdx(5)].description = "STOP P1R3C5";
    settings.kyberpadbuttons[zIdx(1)][zIdx(3)][zIdx(5)].audio_file_start = 0;
    settings.kyberpadbuttons[zIdx(1)][zIdx(3)][zIdx(5)].audio_file_end = 0;
    settings.kyberpadbuttons[zIdx(1)][zIdx(3)][zIdx(5)].kyberpad_stop = true;

    settings.kyberpadbuttons[zIdx(2)][zIdx(3)][zIdx(5)].description = "STOP P2R3C5";
    settings.kyberpadbuttons[zIdx(2)][zIdx(3)][zIdx(5)].audio_file_start = 0;
    settings.kyberpadbuttons[zIdx(2)][zIdx(3)][zIdx(5)].audio_file_end = 0;
    settings.kyberpadbuttons[zIdx(2)][zIdx(3)][zIdx(5)].kyberpad_stop = true;

    settings.kyberpadbuttons[zIdx(3)][zIdx(3)][zIdx(5)].description = "STOP P3R3C5";
    settings.kyberpadbuttons[zIdx(3)][zIdx(3)][zIdx(5)].audio_file_start = 0;
    settings.kyberpadbuttons[zIdx(3)][zIdx(3)][zIdx(5)].audio_file_end = 0;
    settings.kyberpadbuttons[zIdx(3)][zIdx(3)][zIdx(5)].kyberpad_stop = true;

    Serial.println("Configuring KyberPad Buttons...");
    // ---------------------------- PAGE 1 ----------------------------
    // ---------------------------- PAGE 1 ----------------------------
    // Page 1, Row 1, Column 1
    settings.kyberpadbuttons[zIdx(1)][zIdx(1)][zIdx(1)].description = "Page 1 Row 1 Column 1";
    settings.kyberpadbuttons[zIdx(1)][zIdx(1)][zIdx(1)].audio_file_start = 1;
    settings.kyberpadbuttons[zIdx(1)][zIdx(1)][zIdx(1)].audio_file_end = 5;
    settings.kyberpadbuttons[zIdx(1)][zIdx(1)][zIdx(1)].audio_randomize = false;

    // Page 1, Row 1, Column 2
    settings.kyberpadbuttons[zIdx(1)][zIdx(1)][zIdx(2)].description = "Page 1 Row 1 Column 2";
    settings.kyberpadbuttons[zIdx(1)][zIdx(1)][zIdx(2)].audio_file_start = 20;
    settings.kyberpadbuttons[zIdx(1)][zIdx(1)][zIdx(2)].audio_file_end = 20;
    settings.kyberpadbuttons[zIdx(1)][zIdx(1)][zIdx(2)].audio_randomize = false;

    // Page 1, Row 1, Column 3
    settings.kyberpadbuttons[zIdx(1)][zIdx(1)][zIdx(3)].description = "MOVE SW Theme";
    settings.kyberpadbuttons[zIdx(1)][zIdx(1)][zIdx(3)].audio_file_start = 500;
    settings.kyberpadbuttons[zIdx(1)][zIdx(1)][zIdx(3)].audio_file_end = 500;

    // Page 1, Row 1, Column 4

    // Page 1, Row 1, Column 5

    // Page 1, Row 2, Column 1
    settings.kyberpadbuttons[zIdx(1)][zIdx(2)][zIdx(1)].description = "Page 1 Row 2 Column 1";
    settings.kyberpadbuttons[zIdx(1)][zIdx(2)][zIdx(1)].audio_file_start = 21;
    settings.kyberpadbuttons[zIdx(1)][zIdx(2)][zIdx(1)].audio_file_end = 21;
    settings.kyberpadbuttons[zIdx(1)][zIdx(2)][zIdx(1)].audio_randomize = false;

    // Page 1, Row 2, Column 2
    // Page 1, Row 2, Column 3
    // Page 1, Row 2, Column 4
    // Page 1, Row 2, Column 5

    // Page 1, Row 3, Column 1
    // Page 1, Row 3, Column 2
    // Page 1, Row 3, Column 3
    // Page 1, Row 3, Column 4
    // Page 1, Row 3, Column 5

    // ---------------------------- PAGE 2 ----------------------------
    // ---------------------------- PAGE 2 ----------------------------

    // Page 2, Row 1, Column 1
    settings.kyberpadbuttons[zIdx(2)][zIdx(1)][zIdx(1)].description = "Page 2 Row 1 Column 1";
    settings.kyberpadbuttons[zIdx(2)][zIdx(1)][zIdx(1)].audio_file_start = 1;
    settings.kyberpadbuttons[zIdx(2)][zIdx(1)][zIdx(1)].audio_file_end = 5;
    settings.kyberpadbuttons[zIdx(2)][zIdx(1)][zIdx(1)].audio_randomize = false;

    // Page 2, Row 1, Column 2
    // Page 2, Row 1, Column 3
    // Page 2, Row 1, Column 4
    // Page 2, Row 1, Column 5

    // Page 2, Row 2, Column 1
    // Page 2, Row 2, Column 2
    // Page 2, Row 2, Column 3
    // Page 2, Row 2, Column 4
    // Page 2, Row 2, Column 5

    // Page 2, Row 3, Column 1
    // Page 2, Row 3, Column 2
    // Page 2, Row 3, Column 3
    // Page 2, Row 3, Column 4
    // Page 2, Row 3, Column 5

    // ---------------------------- PAGE 3 ----------------------------
    // ---------------------------- PAGE 3 ----------------------------
    // Page 3, Row 1, Column 1
    settings.kyberpadbuttons[zIdx(3)][zIdx(1)][zIdx(1)].description = "Page 3 Row 1 Column 1";
    settings.kyberpadbuttons[zIdx(3)][zIdx(1)][zIdx(1)].audio_file_start = 1;
    settings.kyberpadbuttons[zIdx(3)][zIdx(1)][zIdx(1)].audio_file_end = 5;
    settings.kyberpadbuttons[zIdx(3)][zIdx(1)][zIdx(1)].audio_randomize = false;

    // Page 3, Row 1, Column 2
    // Page 3, Row 1, Column 3
    // Page 3, Row 1, Column 4
    // Page 3, Row 1, Column 5
    settings.kyberpadbuttons[zIdx(3)][zIdx(1)][zIdx(5)].description = "Golden";
    settings.kyberpadbuttons[zIdx(3)][zIdx(1)][zIdx(5)].audio_file_start = 699;
    settings.kyberpadbuttons[zIdx(3)][zIdx(1)][zIdx(5)].audio_file_end = 699;
    settings.kyberpadbuttons[zIdx(3)][zIdx(1)][zIdx(5)].audio_randomize = false;

    // Page 3, Row 2, Column 1
    // Page 3, Row 2, Column 2
    // Page 3, Row 2, Column 3
    // Page 3, Row 2, Column 4
    // Page 3, Row 2, Column 5
    settings.kyberpadbuttons[zIdx(3)][zIdx(2)][zIdx(5)].description = "Random Music";
    settings.kyberpadbuttons[zIdx(3)][zIdx(2)][zIdx(5)].audio_file_start = 540;
    settings.kyberpadbuttons[zIdx(3)][zIdx(2)][zIdx(5)].audio_file_end = 566;
    settings.kyberpadbuttons[zIdx(3)][zIdx(2)][zIdx(5)].audio_randomize = true;

    // Page 3, Row 3, Column 1
    settings.kyberpadbuttons[zIdx(3)][zIdx(3)][zIdx(1)].description = "Droid World";
    settings.kyberpadbuttons[zIdx(3)][zIdx(3)][zIdx(1)].audio_file_start = 547;
    settings.kyberpadbuttons[zIdx(3)][zIdx(3)][zIdx(1)].audio_file_end = 547;
    settings.kyberpadbuttons[zIdx(3)][zIdx(3)][zIdx(1)].audio_randomize = false;
    // Page 3, Row 3, Column 2
    // Page 3, Row 3, Column 3
    // Page 3, Row 3, Column 4
    // Page 3, Row 3, Column 5


    Serial.println("KyberPad Buttons:");
    for (int page = 0; page < settings.kyberpad.pages; page++) {
        for (int row = 0; row < settings.kyberpad.rows; row++) {
            for (int col = 0; col < settings.kyberpad.columns; col++) {
                // int button_index = (page - 1) * settings.kyberpad.rows * settings.kyberpad.columns + (row - 1) * settings.kyberpad.columns + col;
                if (settings.kyberpad.serial_print_button_mapping) {
                    Serial.printf("  KyberPad Button Page %d Row %d Column %d: %s\n", page + 1, row + 1, col + 1, settings.kyberpadbuttons[page][row][col].description.c_str());
                }
                //settings.kyberpadbuttons[page][row][col].audio_file_current = settings.kyberpadbuttons[page][row][col].audio_file_start; // Initialize current audio file to start of range for each button
            }
        }
    }

    Serial.println("KyberPad Channels:");
    for (int i = 0; i < settings.system.num_channels; i++) {
        if (settings.channel[i].enabled) {
            Serial.printf("  Channel %d: %s\n", i + 1, settings.channel[i].description.c_str());
        }
    }   
//    if (!LittleFS.exists(FILE_PATH)) {
//        Serial.println("No config found, creating defaults...");
//        resetSettingsDefaults();
//        saveSettings();
//        return;
//    }

//    File file = LittleFS.open(FILE_PATH, "r");
//    if (!file) {
//        Serial.println("Failed to open config, using defaults");
//        resetSettingsDefaults();
//        return;
//    }

//    StaticJsonDocument<512> doc;
//    DeserializationError err = deserializeJson(doc, file);
//    file.close();
//
//    if (err) {
//        Serial.println("JSON parse failed, resetting");
//        resetSettingsDefaults();
//        return;
//    }
//
//    // -------- AUDIO --------
//    settings.audio.volume = doc["audio"]["volume"] | 15;
//    settings.audio.mute   = doc["audio"]["mute"]   | false;
//
//    // -------- MOTION --------
//    settings.motion.speed = doc["motion"]["speed"] | 80;
//    settings.motion.accel = doc["motion"]["accel"] | 50;
//
//    // -------- SYSTEM --------
//    settings.system.debug = doc["system"]["debug"] | false;
//
//    Serial.println("Settings loaded");
}

// -------------------- SAVE --------------------
void saveSettings() {
    Serial.println("Settings: saveSettings() not yet implemented!");
//
//    StaticJsonDocument<512> doc;
//
//    doc["audio"]["volume"] = settings.audio.volume;
//    doc["audio"]["mute"]   = settings.audio.mute;
//
//    doc["motion"]["speed"] = settings.motion.speed;
//    doc["motion"]["accel"] = settings.motion.accel;
//
//    doc["system"]["debug"] = settings.system.debug;
//
//    File file = LittleFS.open(FILE_PATH, "w");
//    if (!file) {
//        Serial.println("Failed to write config");
//        return;
//    }
//
//    serializeJsonPretty(doc, file);
//    file.close();
//
//    Serial.println("Settings saved");
}