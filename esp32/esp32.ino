#include "sdCard.h"
#include "display.h"
#include "switch.h"
#include "browser.h"
#include "pins.h"
#include "player.h"

#include <SPI.h>
#include <SD.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#include <math.h>
#include <driver/i2s.h>

void setup() {
  // init serial communication
  Serial.begin(9600);
  while (!Serial);

  // initialize shared SPI bus
  SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI);

  // initialize display
  display_Init();

  // initialize sd card reader 
  display_Print("Init SD card... ");
  if (sd_Init() != 0) {
    display_Print("fail!\n");
    while (1);
  }
  display_Print("done.\n");

  // initialize switches
  display_Print("Init switch... ");
  switch_Init();
  display_Print("done.\n");

  // initialize player and I2S
  display_Print("Init player... ");
  if (player_Init() != 0) {
    display_Print("fail!\n");
    while (1);
  }
  display_Print("done.\n");

  // while (1);
  display_Clear();

  browser_Init();
  
}

void loop() {
  // handle switches
  switch_Update();
  player_Update();
  browser_HandleEvent(switch_GetEvent());
}
