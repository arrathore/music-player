#include "appManager.h"

#include "../driver/switch.h"
#include "../driver/display.h"
#include "app.h"

#include "browser.h"
#include "nowPlaying.h"
#include "albumView.h"

#include <Arduino.h>

// app instances
static BrowserApp browserApp;
static NowPlayingApp nowPlayingApp;
static AlbumViewApp albumViewApp;

// active app pointer
static App* activeApp = nullptr;

/********************
 * PUBLIC INTERFACE
 ********************/

void appManager_SwitchTo(App* app) {
  if (app == nullptr) return;

  // deinit current app
  if (activeApp) {
    activeApp->deinit();
    Serial.println("[appManager] deinit current app");
  }
  
  display_Clear(); // clear display for new application

  // init new app
  activeApp = app;
  activeApp->init();
  Serial.println("[appManager] switched app");
}

void appManager_Init(void) {
  appManager_SwitchTo(&browserApp);
}

void appManager_Update(void) {
  if (activeApp) activeApp->update();
}

void appManager_HandleEvent(SwitchEvent e) {
  if (e == SWITCH_NONE) return;
  if (activeApp) activeApp->handleEvent(e);
}

// instance accessors
App* appManager_GetBrowser(void) {
  return &browserApp;
}

App* appManager_GetNowPlaying(void) {
  return &nowPlayingApp;
}

App* appManager_GetAlbumView(void) {
  return &albumViewApp;
}

