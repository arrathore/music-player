#include "display.h"

#include <Arduino.h>
#include <esp_system.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// --- Pin definitions ---
#define SD_CS   D3
#define TFT_CS  D2
#define TFT_DC  D1
#define TFT_RST D0

// display object
Adafruit_ST7735 tft = Adafruit_ST7735(&SPI, TFT_CS, TFT_DC, TFT_RST);

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

void display_Init(void) {
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(0); // portrait
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(true);
  tft.setTextSize(1);
  display_SetCursor(0, 0);

  digitalWrite(TFT_CS, HIGH); // deassert TFT CS
  
  display_Print("ST7735R init success", ST77XX_CYAN);
}
