// Interface with ST7735R SD card reader

#ifndef SDCARD_H
#define SDCARD_H

#include <Arduino.h>
#include <SD.h>

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
  Opens a file for reading. Returns an invalid file object on failure
  Caller must close with sd_CloseFile() when done
*/
File sd_OpenFile(const char* path);

// Close a file opened with sd_OpenFile()
void sd_CloseFile(File& f);
  
/*
  Puts a list of items at this path in items
  returns item count
*/
int sd_ListDir(const char* path, SDItem* items, int maxItems);

bool sd_FileExists(const char* path);

#endif

