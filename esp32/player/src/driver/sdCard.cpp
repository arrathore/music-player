#include "sdCard.h"
#include "../pins.h"

#include <Arduino.h>
#include <esp_system.h>

#include <SPI.h>
#include <SD.h>

int sd_Init(void) {
  // SPI.begin(D8, D9, D10);
  delay(500); // give SD card time to power up
  
  if (!SD.begin(PIN_SD_CS, SPI, 4000000)) { // 4 MHz
    return 1; // initialization failed
  }
  return 0;
}

File sd_OpenFile(const char* path) {
  return SD.open(path);
}

void sd_CloseFile(File& f) {
  if (f) f.close();
}

int sd_ListDir(const char* path, SDItem* items, int maxItems) {
  int count = 0;

  // Add ".." for non-root directories
  if (strcmp(path, "/") != 0 && count < maxItems) {
    items[count].name = "..";
    items[count].type = ITEM_DIR;
    count++;
  }

  File root = SD.open(path);
  if (!root || !root.isDirectory()) {
    return count;
  }

  File entry = root.openNextFile();
  while (entry && count < maxItems) {
    const char* name = entry.name();

    // skip hidden files starting with .
    if (name[0] != '.') {
      items[count].name = String(entry.name());
      items[count].type = entry.isDirectory() ? ITEM_DIR : ITEM_FILE;
      count++;
    }
    
    entry.close();
    entry = root.openNextFile();
  }
  root.close();

  return count;
}

bool sd_FileExists(const char* path) {
  return SD.exists(path);
}

