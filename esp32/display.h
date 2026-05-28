// Interface with ST7735R LCD display

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// Print text to the display
void display_Print(const String &msg, uint16_t color = ST77XX_WHITE);

// Perform initialization tasks
void display_Init(void);

#endif
