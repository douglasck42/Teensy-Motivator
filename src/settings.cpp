// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settings.h"
#include <LittleFS.h>

#define zIdx(x) ((x) - 1)

Settings settings;

//static const char* FILE_PATH = "/config.json";

// -------------------- DEFAULTS --------------------
void resetSettingsDefaults() {
    settings = Settings(); // resets to struct defaults
}

// Channel Functions Definitions
const iFunctionMap IFUNCTION_MAP[] = {
    {"kyberpad",      iChannelFunction::KYBERPAD},
    {"kyberpad_page", iChannelFunction::KYBERPAD_PAGE},
    {"volume",        iChannelFunction::VOLUME},
    {"servo_direct",  iChannelFunction::SERVO_DIRECT},
};
iChannelFunction iLookupFunction(const String& name) {
    for (const auto& entry : IFUNCTION_MAP) {
        if (name == entry.name) return entry.function;
    }
    return iChannelFunction::NONE;
}

const oFunctionMap OFUNCTION_MAP[] = {
    {"servo_direct",  oChannelFunction::SERVO_DIRECT},
};
oChannelFunction oLookupFunction(const String& name) {
    for (const auto& entry : OFUNCTION_MAP) {
        if (name == entry.name) return entry.function;
    }
    return oChannelFunction::NONE;
}

void printSettings();

// -------------------- LOAD --------------------
void loadSettingsDefaults() {
    Serial.println("Settings: loadSettingsDefaults()");
    settings.system.debug_sbus = false;
    Serial.printf("  Default volume: %d\n", settings.audio.volume);
    Serial.printf("  Debug SBUS: %s\n", settings.system.debug_sbus ? "ON" : "OFF");

    settings.system.debug_io_mapping = true;

    Serial.println("Configuring Input Channels");

    {
        auto& ch = settings.ichannel[zIdx(1)];
        ch.enabled = true;
        ch.description = "Throttle";
        ch.sbus_abs_minmax = false;
        ch.us_min = 496;
        ch.us_max = 2496;
        ch.serial_debug_output = false;
    }

    {
        auto& ch = settings.ichannel[zIdx(2)];
        ch.enabled = true;
        ch.description = "Rudder";
        ch.sbus_abs_minmax = false;
        ch.us_min = 496;
        ch.us_max = 2496;
        ch.serial_debug_output = false;
    }

    {
        auto& ch = settings.ichannel[zIdx(3)];
        ch.enabled = true;
        ch.description = "Left Twist";
    }

    {
        auto& ch = settings.ichannel[zIdx(4)];
        ch.enabled = true;
        ch.description = "Elevator";
    }
    {
        auto& ch = settings.ichannel[zIdx(5)];
        ch.enabled = true;
        ch.description = "Aileron";
    }
    {
        auto& ch = settings.ichannel[zIdx(6)];
        ch.enabled = true;
        ch.description = "Right Twist";
    }
    {
        auto& ch = settings.ichannel[zIdx(7)];
        ch.enabled = true;
        ch.description = "SB 3 Position Switch";
    }
    {
        auto& ch = settings.ichannel[zIdx(8)];
        ch.enabled = true;
        ch.description = "SF 2 Position Switch";
    }

    {
        auto& ch = settings.ichannel[zIdx(9)];
        ch.enabled = false;
        ch.description = "Unused";
    }
    {
        auto& ch = settings.ichannel[zIdx(10)];
        ch.enabled = false;
        ch.description = "Unused";
    }
    {
        auto& ch = settings.ichannel[zIdx(11)];
        ch.enabled = false;
        ch.description = "Unused";
    }
    {
        auto& ch = settings.ichannel[zIdx(12)];
        ch.enabled = false;
        ch.description = "Unused";
    }
    {
        auto& ch = settings.ichannel[zIdx(13)];
        ch.enabled = false;
        ch.description = "Unused";
    }
    {
        auto& ch = settings.ichannel[zIdx(14)];
        ch.enabled = true;
        ch.description = "Slider Left";
    }
    {
        auto& ch = settings.ichannel[zIdx(15)];
        ch.enabled = true;
        ch.description = "Slider Right";
    }

    Serial.println("Configuring Input Channels: KyberPad and volume");
    // Human Channel 22, SBUS Channel 21 (0-indexed in code), is the volume control, which is mapped to a continuous range of volume levels from 0 to 30.
    {
        auto& ch = settings.ichannel[zIdx(22)];
        ch.enabled = true;
        ch.description = "Volume Pot 2 Right Dial";
        ch.channelFunction = iChannelFunction::VOLUME;
    }

    // Human Channel 23, SBUS Channel 22 (0-indexed in code), is the KyberPad page selector, which uses the same SBUS range as volume control but is mapped to discrete page states instead of a continuous volume level.
    {
        auto& ch = settings.ichannel[zIdx(23)];
        ch.enabled = true;
        ch.description = "Kyperpad Page SA";
        ch.channelFunction = iChannelFunction::KYBERPAD_PAGE;
    }

    // Human Channel 24, SBUS Channel 23 (0-indexed in code), is the KyberPad software buttons themselves
    {
        auto& ch = settings.ichannel[zIdx(24)];
        ch.enabled = true;
        ch.description = "Kyperpad Buttons";
        ch.channelFunction = iChannelFunction::KYBERPAD;
    }

    Serial.println("Configuring KyberPad Button Channels...");
    //settings.kyperpadbuttonvalues[0].sbus_value = 172;   // No buttons pressed value
    settings.kyperpadbuttonvalues[0].sbus_value = 274;   // Row 1 Column 1
    settings.kyperpadbuttonvalues[1].sbus_value = 376;   // Row 1 Column 2
    settings.kyperpadbuttonvalues[2].sbus_value = 478;   // Row 1 Column 3
    settings.kyperpadbuttonvalues[3].sbus_value = 580;   // Row 1 Column 4
    settings.kyperpadbuttonvalues[4].sbus_value = 682;   // Row 1 Column 5
    settings.kyperpadbuttonvalues[5].sbus_value = 784;   // Row 2 Column 1
    settings.kyperpadbuttonvalues[6].sbus_value = 886;   // Row 2 Column 2
    settings.kyperpadbuttonvalues[7].sbus_value = 988;   // Row 2 Column 3
    settings.kyperpadbuttonvalues[8].sbus_value = 1090;  // Row 2 Column 4
    settings.kyperpadbuttonvalues[9].sbus_value = 1192; // Row 2 Column 5
    settings.kyperpadbuttonvalues[10].sbus_value = 1294; // Row 3 Column 1
    settings.kyperpadbuttonvalues[11].sbus_value = 1396; // Row 3 Column 2
    settings.kyperpadbuttonvalues[12].sbus_value = 1498; // Row 3 Column 3
    settings.kyperpadbuttonvalues[13].sbus_value = 1600; // Row 3 Column 4
    settings.kyperpadbuttonvalues[14].sbus_value = 1702; // Row 3 Column 5


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

    Serial.println("Configuring Output Channels");

    {
        auto& ch = settings.ochannel[0];    // We'll probably actually call this Channel 0 on the Webpage
        ch.enabled = true;
        ch.description = "Left Throttle";
        ch.us_min = 496;
        ch.us_max = 2496;
        ch.serial_debug_output = false;
        ch.ichannel_enabled = true;
        ch.ichannel = zIdx(1);              // Human channel 1
        ch.serial_port_out = 7;
        ch.maestro_id = 1;
        ch.maestro_ch = 0;                  // Maestro starts at Channel 0 too
    }

    {
        auto& ch = settings.ochannel[1];
        ch.enabled = true;
        ch.description = "Left Rudder";
        ch.serial_debug_output = false;
        ch.ichannel_enabled = true;
        ch.ichannel = zIdx(2);              // Human channel 2
        ch.serial_port_out = 7;
        ch.maestro_id = 1;
        ch.maestro_ch = 1;                  // Maestro starts at Channel 0 too
    }

    {
        auto& ch = settings.ochannel[2];
        ch.enabled = true;
        ch.description = "Left Twist";
        ch.serial_debug_output = false;
        ch.ichannel_enabled = true;
        ch.us_abs_minmax = true;
        ch.ichannel = zIdx(3);              // Human channel 3
        ch.serial_port_out = 7;
        ch.maestro_id = 1;
        ch.maestro_ch = 2;                  // Maestro starts at Channel 0 too
    }
    
    {
        auto& ch = settings.ochannel[4];    // We'll probably actually call this Channel 0 on the Webpage
        ch.enabled = true;
        ch.description = "Right Elevator";
        //ch.us_min = 496;
        //ch.us_max = 2496;
        ch.serial_debug_output = false;
        ch.ichannel_enabled = true;
        ch.ichannel = zIdx(4);              // Human channel 4
        ch.serial_port_out = 7;
        ch.maestro_id = 1;
        ch.maestro_ch = 4;                  // Maestro starts at Channel 0 too
    }

    {
        auto& ch = settings.ochannel[5];
        ch.enabled = true;
        ch.description = "Right Aileron";
        ch.serial_debug_output = false;
        ch.ichannel_enabled = true;
        ch.ichannel = zIdx(5);              // Human channel 2
        ch.serial_port_out = 7;
        ch.maestro_id = 1;
        ch.maestro_ch = 5;                  // Maestro starts at Channel 0 too
    }

    {
        auto& ch = settings.ochannel[6];
        ch.enabled = true;
        ch.description = "Right Twist";
        ch.serial_debug_output = false;
        ch.ichannel_enabled = true;
        ch.ichannel = zIdx(6);              // Human channel 3
        ch.serial_port_out = 7;
        ch.maestro_id = 1;
        ch.maestro_ch = 6;                  // Maestro starts at Channel 0 too
    }
    
    printSettings();

}

void printSettings() {
    Serial.println("KyberPad Buttons:");
    for (uint8_t page = 0; page < settings.kyberpad.pages; page++) {
        for (int row = 0; row < settings.kyberpad.rows; row++) {
            for (uint8_t col = 0; col < settings.kyberpad.columns; col++) {
                // int button_index = (page - 1) * settings.kyberpad.rows * settings.kyberpad.columns + (row - 1) * settings.kyberpad.columns + col;
                if (settings.kyberpad.serial_print_button_mapping) {
                    Serial.printf("  KyberPad Button Page %d Row %d Column %d: %s\n", page + 1, row + 1, col + 1, settings.kyberpadbuttons[page][row][col].description.c_str());
                }
                //settings.kyberpadbuttons[page][row][col].audio_file_current = settings.kyberpadbuttons[page][row][col].audio_file_start; // Initialize current audio file to start of range for each button
            }
        }
    }

    Serial.println("\nInput Channels:");
    Serial.println("  Ch  Description               µs Min  µs Max  sbus Min  sbus Max");
    Serial.println("  --  -----------------------   ------  ------  --------  --------");
    for (uint8_t current_channel = 0; current_channel < settings.system.num_ichannels; current_channel++) {
        auto& ch = settings.ichannel[current_channel];
        uint16_t us_min = 0;
        uint16_t us_max = 0;
        uint16_t sbus_min = 0;
        uint16_t sbus_max = 0;
        String description = "[DISABLED]";
        if (ch.enabled) {
            us_min = usGetMin(CH_IN, current_channel);
            us_max = usGetMax(CH_IN, current_channel);
            sbus_min = sbusGetMin(current_channel);
            sbus_max = sbusGetMax(current_channel);
            description = ch.description.c_str();
        }
        Serial.printf("  %-3d %-24s  %-6d  %-6d  %-8d  %-8d\n",
            current_channel + 1,
            description.c_str(),
            us_min,
            us_max,
            sbus_min,
            sbus_max
            );
    }   


    Serial.println("\nOutput Channels:");
    Serial.println("  Ch  Description               I-CH  µs Min  µs Max  Serial    M-ID  M-CH");
    Serial.println("  --  -----------------------   ----  ------  ------  --------  ----  ----");
    for (uint8_t channel = 0; channel < settings.system.num_ochannels; channel++) {
        auto& ch = settings.ochannel[channel];
        if (ch.enabled) {
            Serial.printf("  %-3d %-24s  %-4d  %-6d  %-6d  Serial %-1d  %-4d  %-4d\n",
                channel,
                ch.description.c_str(),
                ch.ichannel + 1,
                usGetMin(CH_OUT, channel),
                usGetMax(CH_OUT, channel),
                ch.serial_port_out,
                ch.maestro_id,
                ch.maestro_ch
                );
        }
    }   

}

// ============================================================
//  settingsSave
//  Serializes the full Settings struct to a JSON file on SD.
//  Delegates all field-level work to each struct's to_json().
// ============================================================
bool settingsSave(const char *path, const Settings &cfg, bool pretty) {
    Serial.println("Saving settings to SD...");
    JsonDocument doc;

    cfg.audio.to_json(doc["audio"].to<JsonObject>());
    cfg.system.to_json(doc["system"].to<JsonObject>());

    // Input channels
    JsonArray ichs = doc["ichannels"].to<JsonArray>();
    for (uint8_t i = 0; i < NUM_INPUT_CHANNELS; i++) {
        cfg.ichannel[i].to_json(ichs.add<JsonObject>());
    }

    // Output channels
    JsonArray ochs = doc["ochannels"].to<JsonArray>();
    for (uint8_t i = 0; i < NUM_OUTPUT_CHANNELS; i++) {
        cfg.ochannel[i].to_json(ochs.add<JsonObject>());
    }

    // Kyberpad config
    cfg.kyberpad.to_json(doc["kyberpad"].to<JsonObject>());

    // Kyberpad raw SBUS values — flat array, index 0 = null/no-press
    JsonArray bvals = doc["kyberpad_sbus_values"].to<JsonArray>();
    for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
        bvals.add(cfg.kyperpadbuttonvalues[i].sbus_value);
    }

    // Kyberpad button definitions [page][row][col]
    JsonArray pages = doc["kyberpad_buttons"].to<JsonArray>();
    for (uint8_t p = 0; p < PAGES; p++) {
        JsonArray rows = pages.add<JsonArray>();
        for (uint8_t r = 0; r < ROWS; r++) {
            JsonArray cols = rows.add<JsonArray>();
            for (uint8_t c = 0; c < COLUMNS; c++) {
                cfg.kyberpadbuttons[p][r][c].to_json(cols.add<JsonObject>());
            }
        }
    }

    return JsonStorage::write(path, doc, pretty);
}


// ============================================================
//  settingsLoad
//  Deserializes a JSON file from SD into the Settings struct.
//  Missing keys leave struct defaults untouched (safe partial load).
//  Delegates all field-level work to each struct's from_json().
// ============================================================
bool settingsLoad(const char *path, Settings &cfg) {
    Serial.println("Loading settings from SD...");
    JsonDocument doc;
    if (!JsonStorage::read(path, doc)) return false;

    if (doc["audio"].is<JsonObjectConst>())
        cfg.audio.from_json(doc["audio"].as<JsonObjectConst>());

    if (doc["system"].is<JsonObjectConst>())
        cfg.system.from_json(doc["system"].as<JsonObjectConst>());

    // Input channels
    if (doc["ichannels"].is<JsonArrayConst>()) {
        JsonArrayConst ichs = doc["ichannels"].as<JsonArrayConst>();
        uint8_t i = 0;
        for (JsonObjectConst ch : ichs) {
            if (i >= NUM_INPUT_CHANNELS) break;
            cfg.ichannel[i++].from_json(ch);
        }
    }

    // Output channels
    if (doc["ochannels"].is<JsonArrayConst>()) {
        JsonArrayConst ochs = doc["ochannels"].as<JsonArrayConst>();
        uint8_t i = 0;
        for (JsonObjectConst ch : ochs) {
            if (i >= NUM_OUTPUT_CHANNELS) break;
            cfg.ochannel[i++].from_json(ch);
        }
    }

    // Kyberpad config
    if (doc["kyberpad"].is<JsonObjectConst>())
        cfg.kyberpad.from_json(doc["kyberpad"].as<JsonObjectConst>());

    // Kyberpad raw SBUS values
    if (doc["kyberpad_sbus_values"].is<JsonArrayConst>()) {
        JsonArrayConst bvals = doc["kyberpad_sbus_values"].as<JsonArrayConst>();
        uint8_t i = 0;
        for (uint16_t v : bvals) {
            if (i >= BUTTON_COUNT) break;
            cfg.kyperpadbuttonvalues[i++].sbus_value = v;
        }
    }

    // Kyberpad button definitions [page][row][col]
    if (doc["kyberpad_buttons"].is<JsonArrayConst>()) {
        JsonArrayConst pages = doc["kyberpad_buttons"].as<JsonArrayConst>();
        uint8_t p = 0;
        for (JsonArrayConst rows : pages) {
            if (p >= PAGES) break;
            uint8_t r = 0;
            for (JsonArrayConst cols : rows) {
                if (r >= ROWS) break;
                uint8_t c = 0;
                for (JsonObjectConst btn : cols) {
                    if (c >= COLUMNS) break;
                    cfg.kyberpadbuttons[p][r][c].from_json(btn);
                    c++;
                }
                r++;
            }
            p++;
        }
    }

    return true;
}

// sbus minimum value, considering per-channel and system-wide defaults
// only used on input channels
uint16_t sbusGetMin(uint8_t channel) {
    auto& ch = settings.ichannel[channel];
    if (ch.sbus_min > 0)      return ch.sbus_min;
    if (ch.sbus_abs_minmax)   return settings.system.sbus_abs_min;
    return settings.system.sbus_standard_min;
}

// sbus maximum value, considering per-channel and system-wide defaults
// only used on input channels
uint16_t sbusGetMax(uint8_t channel) {
    auto& ch = settings.ichannel[channel];
    if (ch.sbus_max > 0)      return ch.sbus_max;
    if (ch.sbus_abs_minmax)   return settings.system.sbus_abs_max;
    return settings.system.sbus_standard_max;
}

// µs minimum value, considering per-channel and system-wide defaults
// used on input and output channels
uint16_t usGetMin(ChannelType type, uint8_t channel) {
    if (type == ChannelType::iCHANNEL) {
        auto& ch = settings.ichannel[channel];
        if (ch.us_min > 0)        return ch.us_min;
        if (ch.us_abs_minmax)     return settings.system.us_abs_min;
        return settings.system.us_standard_min;
    } else {
        auto& ch = settings.ochannel[channel];
        if (ch.us_min > 0)        return ch.us_min;
        if (ch.us_abs_minmax)     return settings.system.us_abs_min;
        return settings.system.us_standard_min;
    }
}

// µs maximum value, considering per-channel and system-wide defaults
// used on input and output channels
uint16_t usGetMax(ChannelType type, uint8_t channel) {
    if (type == ChannelType::iCHANNEL) {
        auto& ch = settings.ichannel[channel];
        if (ch.us_max > 0)        return ch.us_max;
        if (ch.us_abs_minmax)     return settings.system.us_abs_max;
        return settings.system.us_standard_max;
    } else {
        auto& ch = settings.ochannel[channel];
        if (ch.us_max > 0)        return ch.us_max;
        if (ch.us_abs_minmax)     return settings.system.us_abs_max;
        return settings.system.us_standard_max;
    }
}

// µs maximum value, considering per-channel and system-wide defaults
// used on input and output channels
void printChannel(ChannelType type, uint8_t channel) {
    if (type == ChannelType::iCHANNEL) {
        auto& ch = settings.ichannel[channel];
        uint8_t us_percent = map(ch.us_value, usGetMin(type, channel), usGetMax(type, channel), 0, 100);
        uint8_t sbus_percent = map(ch.sbus_value, sbusGetMin(channel), sbusGetMax(channel), 0, 100);
        Serial.printf("Input Ch %d: %dµs (%d%%) sbus=%d (%d%%)\n", channel + 1, ch.us_value, us_percent, ch.sbus_value, sbus_percent);
    } else {
        auto& ch = settings.ochannel[channel];
        uint8_t us_percent = map(ch.us_value, usGetMin(type, channel), usGetMax(type, channel), 0, 100);
        Serial.printf("Output Ch %d: %dµs (%d%%)\n", channel + 1, ch.us_value, us_percent);
    }
}

void ioPrintChannel(uint8_t ochannel, uint8_t ichannel) {
    auto& ich = settings.ichannel[ichannel];
    auto& och = settings.ochannel[ochannel];
    Serial.printf("IO Mapped Output Ch %d to %dµs from iCh %d %dµs\n", ochannel, och.us_value, ichannel+1, ich.us_value);
}