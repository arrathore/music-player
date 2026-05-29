#include "switch.h"

#include <Arduino.h>
#include <esp_system.h>

#define DEBOUNCE_MS 50

// internal state
typedef struct {
  uint8_t pin;
  SwitchEvent event;
  volatile bool triggered; // set by ISR, cleared by update
  unsigned long triggerTime; // millis when ISR fired
} Switch;

static Switch switches[] = {
  { D4, SWITCH_ENTER, false, 0 },
  { D5, SWITCH_DOWN, false, 0},
};
static const int NUM_SWITCHES = sizeof(switches) / sizeof(switches[0]);

static volatile SwitchEvent pendingEvent = SWITCH_NONE;

void IRAM_ATTR enterISR(void) {
  if (!switches[0].triggered) { // debounce
    switches[0].triggered = true;
    switches[0].triggerTime = millis();
  }
}

void IRAM_ATTR downISR(void) {
  if (!switches[1].triggered) { // debounce
    switches[1].triggered = true;
    switches[1].triggerTime = millis();
  }
}

void switch_Update(void) {
  for (int i = 0; i < NUM_SWITCHES; i++) {
    if (switches[i].triggered) {
      // wait for debounce window
      if (millis() - switches[i].triggerTime >= DEBOUNCE_MS) {
	// confirm pin is still in the expected state
	if (digitalRead(switches[i].pin) == LOW) {
	  pendingEvent = switches[i].event;
	}
	switches[i].triggered = false; // clear flag
      }
    }
  }
}

SwitchEvent switch_GetEvent(void) {
  SwitchEvent e = pendingEvent;
  pendingEvent = SWITCH_NONE;
  return e;
}

void switch_Init(void) {
  pinMode(D4, INPUT_PULLDOWN);
  attachInterrupt(D4, enterISR, FALLING);
  pinMode(D5, INPUT_PULLDOWN);
  attachInterrupt(D5, downISR, FALLING);
}

