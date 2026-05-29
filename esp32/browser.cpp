#include "browser.h"
#include "display.h"
#include "sdCard.h"
#include "switch.h"

#include <Arduino.h>
#include <esp_system.h>

// State
static SDItem items[BROWSER_MAX_ITEMS]; // list of current files/folders
static int itemCount = 0;
static int selectedIdx = 1; // index of cursor
static char currentPath[256]; // current directory path

// load items[] from the SD card for the given path
static void loadDirectory(const char* path) {
  itemCount = sd_ListDir(path, items, BROWSER_MAX_ITEMS);
}

// draw the header at the top of the screen displaying the current path
void browser_DrawHeader(void) {
  display_SetCursor(0, 0);
  display_Print(currentPath, ST77XX_YELLOW);
}

void browser_DrawLine(int line) {
  // line = screen line
  if (line < 1) return; // 1st line is first in file list, line 0 is header
  
  // move display cursor to correct line and clear
  display_SetCursor(0, line * BROWSER_LIST_Y);
  display_ClearLine(line * BROWSER_LIST_Y);
  
  // print cursor if needed
  if (line == selectedIdx) display_Print("*", ST77XX_MAGENTA);

  // print name
  if (items[line - 1].type == ITEM_DIR)
    display_Print(items[line - 1].name, ST77XX_GREEN);
  else
    display_Print(items[line - 1].name);
  
  display_Print("\n");
}

void browser_DrawScreen(void) {
  browser_DrawHeader();
  for (int i = 1; i <= itemCount; i++) {
    browser_DrawLine(i);
  }
}

void browser_Init(void) {
  loadDirectory("/");
  currentPath[0] = '/';
  browser_DrawScreen();
}

void browser_CursorDown(void) {
  if (selectedIdx >= itemCount) {
    selectedIdx = 1;
    // redraw old line
    browser_DrawLine(itemCount);
  } else {
    selectedIdx++;
    // redraw old line
    browser_DrawLine(selectedIdx - 1);
  }

  // redraw new line
  browser_DrawLine(selectedIdx);
  Serial.print(selectedIdx);
  Serial.print("/");
  Serial.println(itemCount);
}

void browser_HandleEvent(SwitchEvent e) {
  switch (e) {
    case SWITCH_DOWN:
      browser_CursorDown();
      break;
    case SWITCH_ENTER:
      // TODO
      break;
    default:
      break;
  }
}

