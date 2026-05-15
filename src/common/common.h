// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <Arduino.h>

// ========================= WATCHDOG =========================
#ifndef WATCHDOG_ENABLED
#define WATCHDOG_ENABLED 1
#endif
#ifndef WATCHDOG_TIMEOUT_SECONDS
#define WATCHDOG_TIMEOUT_SECONDS 10
#endif
#if WATCHDOG_ENABLED
#include <esp_task_wdt.h>
#endif

// ========================= HEARTBEAT SETTINGS =========================
#ifndef HEARTBEAT_INTERVAL_MS
#define HEARTBEAT_INTERVAL_MS 5000
#endif

// ========================= SCOMP SETTINGS =========================
#ifndef DEBUG_SCOMP_RX
#define DEBUG_SCOMP_RX 0
#endif
#ifndef SCOMP_BAUD_RATE
#define SCOMP_BAUD_RATE 115200
#endif
#define SCOMP_DEADZONE_MS (HEARTBEAT_INTERVAL_MS * 3)  // if we haven't heard from the Teensy in this long, consider it dead/silent
#ifndef SCOMP_SEND_INTERVAL_MS
#define SCOMP_SEND_INTERVAL_MS 50   // 20 Hz channel updates to ESP32
#endif

char* formatUptime(unsigned long ms);
