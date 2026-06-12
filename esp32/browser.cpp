#include "browser.h"
#include "display.h"
#include "sdCard.h"
#include "switch.h"
#include "player.h"

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
static void drawHeader(void) {
  display_SetCursor(0, 0);
  display_Print(currentPath, ST77XX_YELLOW);
}

static void drawLine(int line) {
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
  display_Clear();
  
  drawHeader();
  for (int i = 1; i <= itemCount; i++) {
    drawLine(i);
  }
}

void browser_Init(void) {
  loadDirectory("/");
  currentPath[0] = '/'; currentPath[1] = '\0';
  browser_DrawScreen();
}


// set currentPath to a directory name under the current directory
// cd name
static void buildChildPath(const char* name) {
  char temp[256];
  if (strcmp(currentPath, "/") == 0)
    snprintf(temp, sizeof(temp), "/%s", name);
  else
    snprintf(temp, sizeof(temp), "%s/%s", currentPath, name);
 
  strncpy(currentPath, temp, sizeof(currentPath) - 1);
  currentPath[sizeof(currentPath) - 1] = '\0';
}

// get the child path without changing directory
static void buildChildPath(const char* name, char* out, size_t outSize) {
  if (strcmp(currentPath, "/") == 0) {
    snprintf(out, outSize, "/%s", name);
  } else {
    snprintf(out, outSize, "%s/%s", currentPath, name);
  }
}

// set currentPath to the parent pathname
// cd ..
static void buildParentPath(void) {
  char* lastSlash = strrchr(currentPath, '/');
  if (lastSlash == currentPath) // parent is root
    currentPath[1] = '\0';
  else if (lastSlash != nullptr)
    *lastSlash = '\0';
}

static void enter(void) {
  Serial.println("[browser] got enter");
  SDItem selection = items[selectedIdx - 1];
  if (selection.type == ITEM_FILE) {
    char fullPath[256];
    buildChildPath(selection.name.c_str(), fullPath, sizeof(fullPath));
    Serial.printf("[browser] now opening %s in player\n", fullPath);
    player_Open(fullPath);
  } else { // directory
    if (selection.name == "..") {
      buildParentPath(); // cd ..
    } else {
      buildChildPath(selection.name.c_str()); // cd selection
    }

    loadDirectory(currentPath);
    selectedIdx = 1;
    browser_DrawScreen();
  }
}

static void cursorDown(void) {
  if (selectedIdx >= itemCount) {
    selectedIdx = 1;
    // redraw old line
    drawLine(itemCount);
  } else {
    selectedIdx++;
    // redraw old line
    drawLine(selectedIdx - 1);
  }

  // redraw new line
  drawLine(selectedIdx);
  /*
  Serial.print(selectedIdx);
  Serial.print("/");
  Serial.println(itemCount);
  */
}

void browser_HandleEvent(SwitchEvent e) {
  switch (e) {
    case SWITCH_DOWN:
      cursorDown();
      break;
    case SWITCH_ENTER:
      enter();
      break;
    default:
      break;
  }
}

