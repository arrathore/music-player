#include "display.h"
#include "../pins.h"

#include <Arduino.h>
#include <esp_system.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SD.h>

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
  tft.fillRect(0, y, DISPLAY_LINE_WIDTH, 12, ST7735_BLACK);
}

void display_Clear(void) {
  tft.fillScreen(ST7735_BLACK);
  display_SetCursor(0, 0);
}

void display_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  tft.fillRect(x, y, w, h, color);
}

void display_DrawBMP(const char* path, int16_t x, int16_t y) {
  File f = SD.open(path);
  if (!f) {
    Serial.printf("[display] drawBMP: could not open %s\n", path);
    return;
  }

  // check BMP header signature
  if (f.read() != 'B' || f.read() != 'M') {
    Serial.println("[display] drawBMP: not a BMP file");
    f.close();
    return;
  }

  // read header fields we need
  // BMP header layout (little-endian):
  // 0x02: file size (4 bytes) — skip
  // 0x06: reserved (4 bytes) — skip
  // 0x0A: pixel data offset (4 bytes)
  // 0x0E: DIB header size (4 bytes) — skip
  // 0x12: width (4 bytes)
  // 0x16: height (4 bytes)
  // 0x1A: color planes (2 bytes) — skip
  // 0x1C: bits per pixel (2 bytes)
  // 0x1E: compression (4 bytes) — must be 0 (none)

  auto read16 = [&]() -> uint16_t {
    uint8_t b[2]; f.read(b, 2);
    return b[0] | (b[1] << 8);
  };
  auto read32 = [&]() -> uint32_t {
    uint8_t b[4]; f.read(b, 4);
    return b[0] | (b[1]<<8) | (b[2]<<16) | (b[3]<<24);
  };
  auto skip = [&](int n) { f.seek(f.position() + n); };

  skip(4);                       // file size
  skip(4);                       // reserved
  uint32_t dataOffset = read32(); // pixel data offset
  skip(4);                       // DIB header size
  int32_t  bmpW       = read32();
  int32_t  bmpH       = read32();
  skip(2);                       // color planes
  uint16_t bpp        = read16();
  uint32_t compression = read32();

  if (bpp != 24 || compression != 0) {
    Serial.printf("[display] drawBMP: unsupported format (bpp=%d compression=%d)\n", bpp, compression);
    f.close();
    return;
  }

  // BMP rows are padded to 4-byte boundaries
  // for 24-bit: row size = width * 3, padded up to multiple of 4
  uint32_t rowSize = ((bmpW * 3) + 3) & ~3;

  // seek to start of each row from bottom
  for (int row = bmpH - 1; row >= 0; row--) {
    f.seek(dataOffset + row * rowSize);

    for (int col = 0; col < bmpW; col++) {
      uint8_t b = f.read();   // blue
      uint8_t g = f.read();   // green
      uint8_t r = f.read();   // red

      // convert 24-bit RGB to RGB565
      uint16_t color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
      tft.drawPixel(x + col, y + (bmpH - 1 - row), color);
    }
  }

  f.close();
}

void display_Init(void) {
  tft.initR(INITR_BLACKTAB);
  // tft.setRotation(0); // portrait
  tft.setRotation(3); // landscape
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(true);
  tft.setTextSize(1);
  display_SetCursor(0, 0);

  digitalWrite(PIN_TFT_CS, HIGH); // deassert TFT CS
  
  display_Print("ST7735R init success\n", ST77XX_CYAN);
}
