// Interface with ST7735R LCD display

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// Print text to the display
void display_Print(const String &msg, uint16_t color = ST77XX_WHITE);

// Set the cursor
void display_SetCursor(int16_t x, int16_t y);

// Clear one line to black
void display_ClearLine(int16_t y);

// Clear the display to black and set cursor to 0
void display_Clear(void);

// Fill a rectangle of color at (x, y) of size w x h
void display_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

// Perform initialization tasks
void display_Init(void);

#endif
