#ifndef PINS_H
#define PINS_H

#if defined(ARDUINO_XIAO_ESP32C6)
// ESP32C6 pin bindings
  #define PIN_SPI_SCK D8
  #define PIN_SPI_MISO D9
  #define PIN_SPI_MOSI D10
  #define PIN_SD_CS D3
  #define PIN_BTN_ENTER D4
  #define PIN_BTN_DOWN D5

// #elif defined(ARDUINO_ESP32_DEV)

#else
  #error "Unknown board - add pin definitions to pins.h"
#endif

#endif
