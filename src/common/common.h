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

void print_memory_info();
