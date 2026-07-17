#include "nowPlaying.h"

#include "appManager.h"
#include "../driver/display.h"
#include "../lib/player.h"
#include "../lib/metadata.h"

#include <Arduino.h>
#include <Adafruit_ST7735.h>

/********************
 * APP INTERFACE
 ********************/
void NowPlayingApp::init() {
  lastUpdateMs = 0;
  lastElapsed = UINT32_MAX; // force draw
  drawStatic();
  drawDynamic();
}

void NowPlayingApp::update() {
  // return to browser if track finished
  if (player_GetState() == PLAYER_STOPPED) {
    Serial.println("[nowPlaying] track finished, returning to browser");
    appManager_SwitchTo(appManager_GetBrowser());
    return;
  }

  // check if track changed and redraw if needed
  if (lastElapsed > player_GetElapsedSec()) {
    Serial.println("[nowPlaying] track changed, updating static elements");
    drawStatic();
  }
  
  // refresh dynamic elements once per second
  uint32_t now = millis();
  if (now - lastUpdateMs >= UPDATE_INTERVAL_MS) {
    lastUpdateMs = now;
    drawDynamic();
  }
}

void NowPlayingApp::handleEvent(SwitchEvent e) {
  switch (e) {
    case SWITCH_ENTER: // pause on enter
      player_Pause();
      drawDynamic();
      break;

    case SWITCH_DOWN: // skip on down
      player_Skip();
      break;

    case SWITCH_UP: // prev on up
      player_Prev();
      break;

    case SWITCH_BACK: // stop on back
      player_Stop();
      appManager_SwitchTo(appManager_GetBrowser());
      break;
      
    default:
      break;
  }
}

void NowPlayingApp::deinit() {
  if (player_GetState() != PLAYER_STOPPED)
    player_Stop();
}

/********************
 * DRAWING - Static
 ********************/
void NowPlayingApp::drawStatic() {
  const TrackMetadata* meta = player_GetMetadata();
  
  // header bar
  display_FillRect(0, ROW_HEADER, DISPLAY_LINE_WIDTH, 10, ST77XX_BLUE);
  display_SetCursor(2, ROW_HEADER + 1);
  display_Print("Now Playing", ST77XX_WHITE);

  // clear filename area and print
  display_FillRect(0, ROW_FILENAME, DISPLAY_LINE_WIDTH, ROW_TIME - ROW_FILENAME, ST77XX_BLACK);
  display_SetCursor(2, ROW_FILENAME);
  if (strlen(meta->title) > 0) {
    display_Print(String(meta->title), ST77XX_WHITE);
  } else {
    // fall back to filename
    const char* path = player_GetFilename();
    const char* name = strrchr(path, '/');
    name = (name != nullptr) ? name + 1 : path;

    char displayName[64];
    strncpy(displayName, name, sizeof(displayName) - 1);
    displayName[sizeof(displayName) - 1] = '\0';
    char* dot = strrchr(displayName, '.');
    if (dot != nullptr) *dot = '\0';
    display_Print(String(displayName), ST77XX_WHITE);
  }

  // draw artist if present
  display_SetCursor(2, ROW_FILENAME + 20);
  if (strlen(meta->artist) > 0) {
    display_Print(String(meta->artist), ST77XX_CYAN);
  }
}

/********************
 * DRAWING - Dynamic
 ********************/
void NowPlayingApp::drawDynamic() {
  uint32_t elapsed = player_GetElapsedSec();
  uint32_t duration = player_GetMetadata()->durationSec;
  
  if (elapsed == lastElapsed && player_GetState() == PLAYER_PLAYING) return;
  lastElapsed = elapsed;

  // time display
  char elapsedStr[8], durationStr[8], timeBuf[16];
  formatTime(elapsed, elapsedStr, sizeof(elapsedStr));
  formatTime(duration, durationStr, sizeof(durationStr));
  snprintf(timeBuf, sizeof(timeBuf), "%s / %s", elapsedStr, durationStr);
  
  display_FillRect(0, ROW_TIME, DISPLAY_LINE_WIDTH, 10, ST77XX_BLACK);
  display_SetCursor(2, ROW_TIME);
  display_Print(String(timeBuf), ST77XX_WHITE);

  // progress bar
  drawProgressBar(elapsed, duration);

  // state indicator
  display_FillRect(0, ROW_STATE, DISPLAY_LINE_WIDTH, 10, ST77XX_BLACK);
  display_SetCursor(2, ROW_STATE);
  if (player_GetState() == PLAYER_PLAYING) {
    display_Print("> Playing", ST77XX_GREEN);
  } else if (player_GetState() == PLAYER_PAUSED) {
    display_Print("|| Paused", ST77XX_YELLOW);
  }
}

void NowPlayingApp::drawProgressBar(uint32_t elapsed, uint32_t duration) {
  display_FillRect(BAR_X, ROW_BAR, BAR_W, BAR_H, ST77XX_YELLOW);

  if (duration > 0) {
    int32_t filled = (int32_t)((float)elapsed / duration * BAR_W);
    filled = constrain(filled, 0, BAR_W);
    if (filled > 0) {
      display_FillRect(BAR_X, ROW_BAR, filled, BAR_H, ST77XX_WHITE);
    }
  }
}

/********************
 * HELPERS
 ********************/
void NowPlayingApp::formatTime(uint32_t sec, char* buf, size_t bufSize) {
  uint32_t m = sec / 60;
  uint32_t s = sec % 60;
  snprintf(buf, bufSize, "%02u:%02u", m, s);
}

