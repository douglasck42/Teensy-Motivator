// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later
#include <Arduino.h>

char* formatUptime(unsigned long ms) {
    static char buf[32];
    unsigned long seconds = ms / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours   = minutes / 60;
    unsigned long days    = hours   / 24;

    seconds %= 60;
    minutes %= 60;
    hours   %= 24;

    if (days > 0)
        sprintf(buf, "%lud %02luh %02lum %02lus", days, hours, minutes, seconds);
    else if (hours > 0)
        sprintf(buf, "%02luh %02lum %02lus", hours, minutes, seconds);
    else if (minutes > 0)
        sprintf(buf, "%02lum %02lus", minutes, seconds);
    else
        sprintf(buf, "%02lus", seconds);

    return buf;
}
