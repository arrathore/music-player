#include "sdCard.h"
#include "display.h"
#include "switch.h"
#include "browser.h";

#include <SPI.h>
#include <SD.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

File myFile;

void setup() {
  // init serial communication
  Serial.begin(9600);
  while (!Serial);

  // initialize shared SPI bus
  SPI.begin(D8, D9, D10);

  display_Init();
  display_Print("Init SD card... ");
  if (sd_Init() != 0) {
    display_Print("fail!\n");
    while (1);
  }
  display_Print("done.\n");
  
  /*
  // open file
  myFile = SD.open("/hello.txt", FILE_WRITE);

  if (myFile) {
    display_Print("Writing to hello.txt... ");
    myFile.println("hello from esp32!");
    // close file
    myFile.close();
    display_Print("done.\n");
  } else { // file failed to open
    display_Print("error opening hello.txt!\n");
  }

  // re-open file for reading
  myFile = SD.open("/hello.txt");
  if (myFile) {
    display_Print("hello.txt:\n");

    // read entire file
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close file
    myFile.close();
  } else { // file failed to open
    display_Print("error opening hello.txt!\n");
  }
  */

  display_Print("Init switches... ");
  switch_Init();
  display_Print("done.\n");
  // while (1);
  display_Clear();

/*
  SDItem items[20];
  int len = sd_ListDir("/", items, 20);
  for (int i = 0; i < 20; i++) {
    if (items[i].type == ITEM_DIR) // print dirs in green
      display_Print(items[i].name, ST77XX_GREEN);
    else
      display_Print(items[i].name);

    display_Print("\n");
  }
*/
  browser_Init();
}

void loop() {
  // handle switches
  switch_Update();
  browser_HandleEvent(switch_GetEvent());
  /*
  SwitchEvent e = switch_GetEvent();
  switch (e) {
    case SWITCH_ENTER:
      Serial.println("enter");
      break;
    case SWITCH_DOWN:
      Serial.println("down");
      break;
    case SWITCH_NONE:
    default:
      break;
  }
  */

}
