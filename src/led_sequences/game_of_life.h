// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <Arduino.h>
#include "led_sequences.h"
#include <Adafruit_NeoPixel.h>
#include <vector>
#include <cstdint>

// Separate struct for the LED layout map
struct LedMap {
    uint16_t width;
    uint16_t height;
    std::vector<int8_t> data;

    LedMap(int w, int h) : width(w), height(h), data(w * h, -1) {
        initDefault();
    }

    int8_t& at(int row, int col) {
        return data[row * width + col];
    }

    const int8_t& at(int row, int col) const {
        return data[row * width + col];
    }

    // Default: simple linear mapping
    void initDefault() {
        int index = 0;
        for (int r = 0; r < height; r++) {
            for (int c = 0; c < width; c++) {
                at(r, c) = index++;
            }
        }
    }

    // Override with a custom layout (e.g. rounded corners with -1 for missing LEDs)
    void initCustom(const std::vector<int8_t>& custom) {
        if (custom.size() == data.size()) {
            data = custom;
        }
    }
};

struct led_gameOfLife : led_animation {
    uint32_t lastUpdateGameOfLife = 0;
    uint32_t gameOfLifeDelay = 750;
    uint32_t lastUpdateGameOfLifeReset = 0;
    uint32_t gameOfLifeReset = 15000;

    uint16_t width;
    uint16_t height;

    LedMap ledMap;

    // Grids stored as flat vectors to support runtime width/height
    std::vector<uint8_t> grid;
    std::vector<uint8_t> nextGrid;

    led_gameOfLife(uint16_t w = 8, uint16_t h = 8)
        : width(w), height(h),
          ledMap(w, h),
          grid(w * h, 0),
          nextGrid(w * h, 0)
    {}

    uint8_t& gridAt(int row, int col)         { return grid[row * width + col]; }
    uint8_t  gridAt(int row, int col) const   { return grid[row * width + col]; }
    uint8_t& nextAt(int row, int col)         { return nextGrid[row * width + col]; }
    uint8_t  nextAt(int row, int col) const   { return nextGrid[row * width + col]; }
};

void gameOfLifeInitGrid(led_gameOfLife &gameOfLife);
uint8_t gameOfLifeCountNeighbors(led_gameOfLife &gameOfLife, uint8_t x, uint8_t y);
void gameOfLifeStep(led_gameOfLife &gameOfLife);
void gameOfLifeRenderGrid(Adafruit_NeoPixel &s, led_gameOfLife &gameOfLife);
bool gameOfLifeIsDead(led_gameOfLife &gameOfLife);

// /// Example of initializing a rounded-corner LED matrix:
// led_gameOfLife gol(8, 8);
// std::vector<int8_t> roundedMap = {
//     -1, -1,  0,  1,  2,  3, -1, -1,
//     -1,  4,  5,  6,  7,  8,  9, -1,
//     10, 11, 12, 13, 14, 15, 16, 17,
//     18, 19, 20, 21, 22, 23, 24, 25,
//     26, 27, 28, 29, 30, 31, 32, 33,
//     34, 35, 36, 37, 38, 39, 40, 41,
//     -1, 42, 43, 44, 45, 46, 47, -1,
//     -1, -1, 48, 49, 50, 51, -1, -1
// };
// gol.ledMap.initCustom(roundedMap);