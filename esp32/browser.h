// File browser application for the SD card

#ifndef BROWSER_H
#define BROWSER_H

#include "switch.h"
#include "sdCard.h"

#define BROWSER_MAX_ITEMS 14
#define BROWSER_VISIBLE 14 // lines visible at once
#define BROWSER_HEADER_Y 0 // y position of header bar
#define BROWSER_LIST_Y 12 // y position where list items start

// Load directory and draw
void browser_Init(void);

// Draw a given line in directory listing
void browser_DrawLine(int line);

// Draw the entire screen
void browser_DrawScreen(void);

// Pass switch events into browser
void browser_HandleEvent(SwitchEvent e);

#endif
