// Abstract base class for applications
#ifndef APP_H
#define APP_H

#include "switch.h"

class App {
 public:
  virtual ~App() {}
  virtual void init() = 0; // called when app becomes active
  virtual void update() = 0; // called every loop()
  virtual void handleEvent(SwitchEvent) = 0; // handle button events
  virtual void deinit() = 0; // called when app closes
};

#endif
