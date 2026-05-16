// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later
#include <Arduino.h>
#include <malloc.h>

void print_memory_info() {
#if defined(ESP32)
    Serial.printf("  Heap size:     %6lu bytes\n", (unsigned long)ESP.getHeapSize());
    Serial.printf("  Heap free:     %6lu bytes\n", (unsigned long)ESP.getFreeHeap());
    Serial.printf("  Heap free min: %6lu bytes\n", (unsigned long)ESP.getMinFreeHeap());
    Serial.printf("  PSRAM free:    %6lu bytes\n", (unsigned long)ESP.getFreePsram());
#elif defined(__IMXRT1062__)  // Teensy 4.x
    extern char _end;      // linker: end of BSS / start of heap
    extern char _estack;   // linker: top of stack
    char stack_now;
    struct mallinfo mi = mallinfo();
    Serial.printf("  Heap used:  %6d bytes\n", mi.uordblks);
    Serial.printf("  Heap free:  %6d bytes\n", mi.fordblks);
    Serial.printf("  Stack used: %6d bytes\n", (int)(&_estack - &stack_now));
#else
    Serial.println("  print_memory_info: unsupported platform");
#endif
}
