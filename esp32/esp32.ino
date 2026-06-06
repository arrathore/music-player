#include "sdCard.h"
#include "display.h"
#include "switch.h"
#include "browser.h"
#include "pins.h"

#include <SPI.h>
#include <SD.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#include <math.h>
#include <driver/i2s.h>

// --- Audio config ---
#define SAMPLE_RATE     44100
#define BITS_PER_SAMPLE 16
#define NUM_CHANNELS    2           // stereo
#define TONE_HZ         440         // concert A
#define AMPLITUDE       28000       // 0–32767; below max to avoid clipping
#define DMA_BUF_COUNT   8
#define DMA_BUF_LEN     64

// Precomputed sine table for efficiency (one full cycle, 256 steps)
#define SINE_TABLE_SIZE 256
static int16_t sineTable[SINE_TABLE_SIZE];

void buildSineTable() {
  for (int i = 0; i < SINE_TABLE_SIZE; i++) {
    sineTable[i] = (int16_t)(AMPLITUDE * sin(2.0 * M_PI * i / SINE_TABLE_SIZE));
  }
}

void i2s_Init() {
  i2s_config_t config = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate          = SAMPLE_RATE,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format       = I2S_CHANNEL_FMT_RIGHT_LEFT,  // stereo
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = DMA_BUF_COUNT,
    .dma_buf_len          = DMA_BUF_LEN,
    .use_apll             = false,
    .tx_desc_auto_clear   = true,   // output silence on underrun
  };

  i2s_pin_config_t pins = {
    .bck_io_num   = PIN_I2S_BCLK,
    .ws_io_num    = PIN_I2S_LRCLK,
    .data_out_num = PIN_I2S_DATA,
    .data_in_num  = I2S_PIN_NO_CHANGE,
  };

  i2s_driver_install(I2S_NUM_0, &config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pins);
  i2s_zero_dma_buffer(I2S_NUM_0);
}

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

  display_Print("I2S tone test...\n");

  buildSineTable();
  i2s_Init();

  /*
  // while (1);
  display_Clear();

  browser_Init();
  */
}

void loop() {

    // Each call fills one DMA buffer worth of stereo samples
  // Buffer: [L, R, L, R, ...] as int16_t pairs
  static uint32_t sampleCount = 0;

  // How many sine table steps to advance per sample to produce TONE_HZ
  // steps/sample = (SINE_TABLE_SIZE * TONE_HZ) / SAMPLE_RATE
  const float stepSize = (float)SINE_TABLE_SIZE * TONE_HZ / SAMPLE_RATE;

  int16_t buf[DMA_BUF_LEN * NUM_CHANNELS];

  for (int i = 0; i < DMA_BUF_LEN; i++) {
    int tableIdx = (int)(sampleCount * stepSize) % SINE_TABLE_SIZE;
    int16_t sample = sineTable[tableIdx];

    buf[i * 2]     = sample;  // left channel
    buf[i * 2 + 1] = sample;  // right channel (same — mono tone to both ears)

    sampleCount++;
  }

  size_t bytesWritten = 0;
  i2s_write(I2S_NUM_0, buf, sizeof(buf), &bytesWritten, portMAX_DELAY);

  /*
  // handle switches
  switch_Update();
  browser_HandleEvent(switch_GetEvent());
  */

}
