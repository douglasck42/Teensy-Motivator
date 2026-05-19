// Copyright (c) 2026 Douglas Kempthorne (douglas@kempthorne.com)
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Arduino.h>
#include "game_of_life.h"

//----------------------------------------------------
// Game of Life - Initialize grid randomly
//----------------------------------------------------
void gameOfLifeInitGrid(led_gameOfLife &gameOfLife) {
  for (uint16_t y = 0; y < gameOfLife.height; y++) {
    for (uint16_t x = 0; x < gameOfLife.width; x++) {
      gameOfLife.gridAt(y, x) = random(2); // random 0 or 1
    }
  }
}

//----------------------------------------------------
// Game of Life - Count live neighbors (no wrap-around)
//----------------------------------------------------
uint8_t gameOfLifeCountNeighbors(led_gameOfLife &gameOfLife, uint8_t x, uint8_t y) {
    //if (gameOfLife.serialDebug) Serial.println("gameOfLifeCountNeighbors()");

    uint8_t count = 0;
    for (int8_t dy = -1; dy <= 1; dy++) {
      for (int8_t dx = -1; dx <= 1; dx++) {
        if (dx == 0 && dy == 0) continue; // skip self
        int16_t nx = (int16_t)x + dx;
        int16_t ny = (int16_t)y + dy;
        if (nx < 0 || nx >= (int16_t)gameOfLife.width) continue;
        if (ny < 0 || ny >= (int16_t)gameOfLife.height) continue;
        if (gameOfLife.ledMap.at(ny, nx) == -1) continue; // skip cells without LEDs
        count += gameOfLife.gridAt(ny, nx);
      }
    }
    return count;
}

//----------------------------------------------------
// Game of Life - Compute next generation
//----------------------------------------------------
void gameOfLifeStep(led_gameOfLife &gameOfLife) {
  if (gameOfLife.serialDebug) Serial.println("gameOfLifeStep()");

  for (uint8_t y = 0; y < gameOfLife.height; y++) {
    for (uint8_t x = 0; x < gameOfLife.width; x++) {
      if (gameOfLife.ledMap.at(y, x) == -1) {
        gameOfLife.nextAt(y, x) = 0; // no LED here
        continue;
      }
      uint8_t neighbors = gameOfLifeCountNeighbors(gameOfLife, x, y); // fixed: pass gameOfLife
      if (gameOfLife.gridAt(y, x)) { // alive
        gameOfLife.nextAt(y, x) = (neighbors == 2 || neighbors == 3) ? 1 : 0;
      } else { // dead
        gameOfLife.nextAt(y, x) = (neighbors == 3) ? 1 : 0;
      }
    }
  }

  // Copy nextGrid to grid
  for (uint8_t y = 0; y < gameOfLife.height; y++) {
    for (uint8_t x = 0; x < gameOfLife.width; x++) {
      gameOfLife.gridAt(y, x) = gameOfLife.nextAt(y, x);
    }
  }
}

//----------------------------------------------------
// Game of Life - Render grid to LEDs
//----------------------------------------------------
void gameOfLifeRenderGrid(Adafruit_NeoPixel &s, led_gameOfLife &gameOfLife) {
  if (gameOfLife.serialDebug) Serial.println("gameOfLifeRenderGrid()");
  for (uint8_t y = 0; y < gameOfLife.height; y++) {
    for (uint8_t x = 0; x < gameOfLife.width; x++) {
      int8_t idx = gameOfLife.ledMap.at(y, x);
      if (idx < 0) continue; // skip empty cells
      if (gameOfLife.gridAt(y, x)) {
        s.setPixelColor(idx, s.Color(gameOfLife.r, gameOfLife.b, gameOfLife.g, gameOfLife.w)); // white
      } else {
        s.setPixelColor(idx, 0); // off
      }
    }
  }
  // s.show(); // call this from your main loop after rendering
}

//----------------------------------------------------
// Game of Life - Returns true if game is "dead" (all off, or no change)
//----------------------------------------------------
bool gameOfLifeIsDead(led_gameOfLife &gameOfLife) {
  if (gameOfLife.serialDebug) Serial.println("gameOfLifeIsDead()");
  bool allDead = true;
  bool unchanged = true;

  for (uint8_t y = 0; y < gameOfLife.height; y++) {
    for (uint8_t x = 0; x < gameOfLife.width; x++) {
      // skip cells that don't map to LEDs
      if (gameOfLife.ledMap.at(y, x) == -1) continue;

      if (gameOfLife.gridAt(y, x) != 0) {
        allDead = false;  // found a live cell
      }
      if (gameOfLife.gridAt(y, x) != gameOfLife.nextAt(y, x)) {
        unchanged = false;  // found a cell that changed
      }
    }
  }

  return (allDead || unchanged);
}
