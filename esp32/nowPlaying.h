#ifndef NOWPLAYING_H
#define NOWPLAYING_H

#include "app.h"
#include <Arduino.h>

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
