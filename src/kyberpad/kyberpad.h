#pragma once
#include <Arduino.h>
#include "settings.h"

void ioKyberpadButtons(uint8_t channel, unsigned long now) ;
void ioKyberpadPage(uint8_t channel);
void ioKyberpadButtonExecute(uint8_t page, uint8_t row, uint8_t column);
