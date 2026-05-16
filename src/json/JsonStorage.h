// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Arduino.h>
#include <SD.h>
#include <ArduinoJson.h>

class JsonStorage {
public:
  // Initialize SD (call in setup)
  static bool begin(uint8_t csPin);

  // Read JSON file into a document
  static bool read(const char *path, JsonDocument &doc);

  // Write JSON document to file
  static bool write(const char *path, const JsonDocument &doc, bool pretty = true);

  // Check if file exists
  static bool exists(const char *path);

  // Delete file
  static bool remove(const char *path);

  // Rename/move file
  static bool rename(const char *from, const char *to);

private:
  static bool _initialized;
};