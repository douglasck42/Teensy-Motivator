// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later

// JsonDocument doc;
// 
// doc["brightness"] = 128;
// doc["volume"] = 80;
// 
// JsonStorage::write("/config.json", doc);

#include "JsonStorage.h"

bool JsonStorage::_initialized = false;

bool JsonStorage::begin(uint8_t csPin) {
  if (SD.begin(csPin)) {
    _initialized = true;
    return true;
  }
  return false;
}

bool JsonStorage::exists(const char *path) {
  if (!_initialized) return false;
  return SD.exists(path);
}

bool JsonStorage::remove(const char *path) {
  if (!_initialized) return false;
  return SD.remove(path);
}

bool JsonStorage::rename(const char *from, const char *to) {
  if (!_initialized) return false;
  if (SD.exists(to)) SD.remove(to);
  return SD.rename(from, to);
}

bool JsonStorage::read(const char *path, JsonDocument &doc) {
  if (!_initialized) return false;

  File file = SD.open(path, FILE_READ);
  if (!file) return false;

  DeserializationError err = deserializeJson(doc, file);
  file.close();

  return !err;
}

bool JsonStorage::write(const char *path, const JsonDocument &doc, bool pretty) {
  if (!_initialized) return false;

  if (SD.exists(path)) SD.remove(path);

  File file = SD.open(path, FILE_WRITE);
  if (!file) return false;

  size_t written = 0;

  if (pretty) {
    written = serializeJsonPretty(doc, file);
  } else {
    written = serializeJson(doc, file);
  }

  file.close();
  return written > 0;
}
