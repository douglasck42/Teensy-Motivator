#pragma once
#include <Arduino.h>
#include "settings.h"

void ioKyberpadButtons(int channel, int16_t value, unsigned long now) ;
void ioKyberpadPage(int channel, int16_t value);
void ioKyberpadButtonExecute(int page, int row, int column);
