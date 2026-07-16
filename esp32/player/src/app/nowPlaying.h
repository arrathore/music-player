#ifndef NOWPLAYING_H
#define NOWPLAYING_H

#include "app.h"
#include "../driver/display.h"

#include <Arduino.h>


#define UPDATE_INTERVAL_MS 1000

// Layout constants
#define ROW_HEADER 0
#define ROW_FILENAME 20
#define ROW_TIME 80
#define ROW_BAR 95
#define ROW_STATE 115

#define BAR_X 2
#define BAR_Y ROW_BAR
#define BAR_W DISPLAY_LINE_WIDTH - (2 * BAR_X)
#define BAR_H 8

class NowPlayingApp : public App {
 public:
  void init() override;
  void update() override;
  void handleEvent(SwitchEvent e) override;
  void deinit() override;

 private:
  uint32_t lastUpdateMs = 0; // millis() of last time display was refreshed
  uint32_t lastElapsed = 0; // last elapsed value drawn

  // Draw elements that don't change
  void drawStatic(void);

  // Draw elements that update every second
  void drawDynamic(void);

  void drawProgressBar(uint32_t elapsed, uint32_t duration);

  void formatTime(uint32_t sec, char* buf, size_t bufSize);
};

#endif
