#include "sdCard.h"
#include "display.h"

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

  // initialize display
  display_Init();

  display_Print("Initializing SD card... ");

  if (sd_Init() != 0) {
    display_Print("initialization failed!\n");
    while (1);
  }
  display_Print("initialization done.\n");
  

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


}

void loop() {
  // put your main code here, to run repeatedly:

}
