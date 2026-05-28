#include "sdCard.h"

#include <Arduino.h>
#include <esp_system.h>

#include <SPI.h>
#include <SD.h>

int sd_Init(void) {
  SPI.begin(D8, D9, D10);
  delay(500); // give SD card time to power up
  
  if (!SD.begin(D3, SPI, 4000000)) { // 4 MHz
    return 1; // initialization failed
  }
  return 0;
}

