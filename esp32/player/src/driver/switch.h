// Interface with switches

#ifndef SWITCH_H
#define SWITCH_H

#include <stdint.h>

// Initialize interrupts for buttons
void switch_Init(void);
void switch_Update(void); // called every loop()

// Action codes
typedef enum {
  SWITCH_NONE = 0,
  SWITCH_ENTER = 1,
  SWITCH_DOWN = 2,
  SWITCH_UP = 3,
  SWITCH_BACK = 4,
} SwitchEvent;

// Returns the latest event and clears
SwitchEvent switch_GetEvent(void);

#endif

