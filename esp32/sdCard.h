// Interface with ST7735R SD card reader

#ifndef SDCARD_H
#define SDCARD_H

#include <Arduino.h>

typedef enum {
  ITEM_FILE = 0,
  ITEM_DIR = 1,
} ItemType;

typedef struct {
  String name;
  ItemType type;
} SDItem;

/*
  Perform initialization tasks
  returns 0 if successfully initialized
*/
int sd_Init(void);

/*
  Puts a list of items at this path in items
  returns item count
*/
int sd_ListDir(const char* path, SDItem* items, int maxItems);

#endif

