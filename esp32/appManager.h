// Application Manager

#ifndef APPMANAGER_H
#define APPMANAGER_H

#include "switch.h"
#include "app.h"

class BrowserApp;

// Initialize manager and launch browser
void appManager_Init(void);

// Drive active app
void appManager_Update(void);

// Route switch events into the active application
void appManager_HandleEvent(SwitchEvent e);

// Switch the current active application to a
void appManager_SwitchTo(App* app);

// App instance accessors
App* appManager_GetBrowser(void);

#endif

