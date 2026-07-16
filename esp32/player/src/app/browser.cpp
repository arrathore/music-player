#include "browser.h"

#include "../driver/display.h"
#include "../driver/sdCard.h"
#include "../driver/switch.h"
#include "../lib/player.h"

#include "appManager.h"
#include "albumView.h"

#include <Arduino.h>
#include <esp_system.h>

/********************
 * APP INTERFACE
 ********************/

void BrowserApp::init() {
  loadDirectory("/");
  currentPath[0] = '/'; currentPath[1] = '\0';
  selectedIdx = 1;
  drawScreen();
}

void BrowserApp::update() {
  // no per-frame logic
}

void BrowserApp::deinit() {
  // needs no de-init procedure
}

void BrowserApp::handleEvent(SwitchEvent e) {
  switch (e) {
    case SWITCH_ENTER: enter(); break;
    case SWITCH_DOWN: cursorDown(); break;
    case SWITCH_UP: cursorUp(); break;
    case SWITCH_BACK: backout(); break;
    default: break;
  }
}

/********************
 * DRAWING
 ********************/

// draw the header at the top of the screen displaying the current path
void BrowserApp::drawHeader(void) {
  display_SetCursor(0, 0);
  display_Print(currentPath, ST77XX_YELLOW);
}

void BrowserApp::drawLine(int line) {
  // line = screen line
  if (line < 1) return; // 1st line is first in file list, line 0 is header
  
  // move display cursor to correct line and clear
  display_SetCursor(0, line * BROWSER_LIST_Y);
  display_ClearLine(line * BROWSER_LIST_Y);
  
  // print cursor if needed
  if (line == selectedIdx) display_Print("*", ST77XX_MAGENTA);

  // truncate name if needed
  String name = String(items[line - 1].name);
  if (name.length() > BROWSER_ITEM_MAX_LENGTH) name = name.substring(0, BROWSER_ITEM_MAX_LENGTH - 2) + "-";
  
  // print name
  if (items[line - 1].type == ITEM_DIR)
    display_Print(name, ST77XX_GREEN);
  else
    display_Print(name);
  
  display_Print("\n");
}

void BrowserApp::drawScreen(void) {
  display_Clear();
  
  drawHeader();
  for (int i = 1; i <= itemCount; i++) {
    drawLine(i);
  }
}

/********************
 * NAVIGATION
 ********************/

// load items[] from the SD card for the given path
void BrowserApp::loadDirectory(const char* path) {
  itemCount = sd_ListDir(path, items, BROWSER_MAX_ITEMS);
}

// set currentPath to a directory name under the current directory
// cd name
void BrowserApp::buildChildPath(const char* name) {
  char temp[256];
  if (strcmp(currentPath, "/") == 0)
    snprintf(temp, sizeof(temp), "/%s", name);
  else
    snprintf(temp, sizeof(temp), "%s/%s", currentPath, name);
 
  strncpy(currentPath, temp, sizeof(currentPath) - 1);
  currentPath[sizeof(currentPath) - 1] = '\0';
}

// get the child path without changing directory
void BrowserApp::buildChildPath(const char* name, char* out, size_t outSize) {
  if (strcmp(currentPath, "/") == 0) {
    snprintf(out, outSize, "/%s", name);
  } else {
    snprintf(out, outSize, "%s/%s", currentPath, name);
  }
}

// set currentPath to the parent pathname
// cd ..
void BrowserApp::buildParentPath(void) {
  char* lastSlash = strrchr(currentPath, '/');
  if (lastSlash == currentPath) // parent is root
    currentPath[1] = '\0';
  else if (lastSlash != nullptr)
    *lastSlash = '\0';
}

void BrowserApp::cursorDown(void) {
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

void BrowserApp::cursorUp(void) {
  Serial.println("[browser] got up");
  if (selectedIdx <= 1) {
    selectedIdx = itemCount;
    // redraw old line
    drawLine(1);
  } else {
    selectedIdx--;
    // redraw old line
    drawLine(selectedIdx + 1);
  }

  // redraw new line
  drawLine(selectedIdx);
}

void BrowserApp::backout(void) {
  Serial.println("[browser] got back");
  buildParentPath();
  loadDirectory(currentPath);
  selectedIdx = 1;
  drawScreen();
}

void BrowserApp::enter(void) {
  Serial.println("[browser] got enter");
  SDItem selection = items[selectedIdx - 1];
  if (selection.type == ITEM_FILE) {
    char fullPath[256];
    buildChildPath(selection.name.c_str(), fullPath, sizeof(fullPath));
    Serial.printf("[browser] now opening %s in player\n", fullPath);
    player_Open(fullPath);
    appManager_SwitchTo(appManager_GetNowPlaying());

  } else { // directory
    if (selection.name == "..") {
      buildParentPath(); // cd ..
    } else {
      buildChildPath(selection.name.c_str()); // cd selection
    }

    // check for meta.txt in the new directory
    char metaPath[270];
    snprintf(metaPath, sizeof(metaPath), "%s/meta.txt", currentPath);
    if (sd_FileExists(metaPath)) {
      // launch albumView
      static_cast<AlbumViewApp*>(appManager_GetAlbumView())->setPath(currentPath);
      appManager_SwitchTo(appManager_GetAlbumView());
    } else { // standard behavior
      loadDirectory(currentPath);
      selectedIdx = 1;
      drawScreen();
    }
  }
}

