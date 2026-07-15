// File browser application for the SD card

#ifndef BROWSER_H
#define BROWSER_H

#include "../driver/switch.h"
#include "../driver/sdCard.h"
#include "app.h"

#define BROWSER_MAX_ITEMS 10
#define BROWSER_VISIBLE 10 // lines visible at once
#define BROWSER_HEADER_Y 0 // y position of header bar
#define BROWSER_LIST_Y 12 // y position where list items start
#define BROWSER_ITEM_MAX_LENGTH 26 // chars that can fit on one line

class BrowserApp : public App {
 public:
  void init() override; // load directory and draw
  void update() override;
  void handleEvent(SwitchEvent e) override;
  void deinit() override;

 private:
  SDItem items[BROWSER_MAX_ITEMS];
  int itemCount = 0;
  int selectedIdx = 1;
  char currentPath[256];

  void loadDirectory(const char* path);
  void drawHeader(void);
  void drawLine(int line);
  void drawScreen(void);
  void enter(void);
  void cursorDown(void);
  void buildChildPath(const char* name);
  void buildChildPath(const char* name, char* out, size_t outSize);
  void buildParentPath(void);
  
};

#endif
