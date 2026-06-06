#ifndef PINS_H
#define PINS_H

#include <Arduino.h>
#include <esp_system.h>

// ESP32C6 pin bindings
#if defined(ARDUINO_XIAO_ESP32C6)
  // SPI
  #define PIN_SPI_SCK   D8
  #define PIN_SPI_MISO  D9
  #define PIN_SPI_MOSI  D10
  #define PIN_SD_CS     D3

  // display
  #define PIN_TFT_CS  D2
  #define PIN_TFT_DC  D1
  #define PIN_TFT_RST D0

  // buttons
  #define PIN_BTN_ENTER D4
  #define PIN_BTN_DOWN  D5

// ESP-WROOM-32 pin bindings
#elif defined(ARDUINO_ESP32_DEV)
  // SPI
  #define PIN_SPI_SCK   18
  #define PIN_SPI_MISO  19
  #define PIN_SPI_MOSI  23
  #define PIN_SD_CS      5

  // display
  #define PIN_TFT_CS  22
  #define PIN_TFT_DC  21
  #define PIN_TFT_RST  4

  // buttons
  #define PIN_BTN_ENTER 32
  #define PIN_BTN_DOWN  33

  // I2S
  #define PIN_I2S_BCLK  26
  #define PIN_I2S_LRCLK 25
  #define PIN_I2S_DATA  27

#else
  #error "Unknown board - add pin definitions to pins.h"
#endif

#endif
