#include "display.h"
#include "../pins.h"

#include <Arduino.h>
#include <esp_system.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// display object
Adafruit_ST7735 tft = Adafruit_ST7735(&SPI, PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);

void display_Print(const String &msg, uint16_t color) {
  //  Serial.println(msg);
  tft.setTextColor(color);
  tft.print(msg);
}

void display_SetCursor(int16_t x, int16_t y) {
  tft.setCursor(x, y);
}

void display_ClearLine(int16_t y) {
  tft.fillRect(0, y, 128, 12, ST7735_BLACK);
}

void display_Clear(void) {
  tft.fillScreen(ST7735_BLACK);
  display_SetCursor(0, 0);
}

void display_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  tft.fillRect(x, y, w, h, color);
}

void display_Init(void) {
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(0); // portrait
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(true);
  tft.setTextSize(1);
  display_SetCursor(0, 0);

  digitalWrite(PIN_TFT_CS, HIGH); // deassert TFT CS
  
  display_Print("ST7735R init success\n", ST77XX_CYAN);
}
