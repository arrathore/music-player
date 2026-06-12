#include "player.h"
#include "pins.h"

#include <Arduino.h>
#include <AudioGeneratorWAV.h>
#include <AudioFileSourceSD.h>
#include <AudioOutputI2S.h>

// ESP8266Audio objects
static AudioGeneratorWAV* wav = nullptr;
static AudioFileSourceSD* source = nullptr;
static AudioOutputI2S* out = nullptr;

// internal state
static PlayerState state = PLAYER_STOPPED;
static char filename[256] = "";
static uint32_t startMs = 0; // millis() when playback started
static uint32_t pausedMs = 0; // accumulated paused time
static uint32_t pauseStart = 0; // millis() when pause began
static uint32_t durationSec = 0;

PlayerResult player_Init(void) {
  out = new AudioOutputI2S();
  if (!out) return PLAYER_ERR_INIT;

  // set i2s pins
  out->SetPinout(PIN_I2S_BCLK, PIN_I2S_LRCLK, PIN_I2S_DATA);

  // set output to stereo
  out->SetChannels(2);

  Serial.println("[player] init OK");
  return PLAYER_OK;
}

void player_Stop(void) {
  if (wav) {
    wav->stop();
    delete wav; wav = nullptr;
  }

  if (source) {
    delete source; source = nullptr;
  }

  if (out) {
    out->stop();
  }

  state = PLAYER_STOPPED;
  filename[0] = '\0';
  startMs = 0;
  pausedMs - 0;
  pauseStart = 0;
  durationSec = 0;

  Serial.println("[player] stopped");
}

PlayerResult player_Open(const char* path) {
  Serial.printf("[player] player_Open called with path %s\n", path);
  
  // stop before starting anything
  player_Stop();

  // create new source from SD file
  source = new AudioFileSourceSD(path);
  if (!source || !source->isOpen()) {
    Serial.println("[player] failed to open file!");
    delete source; source = nullptr;
    return PLAYER_ERR_FILE;
  }

  // create wav generator
  wav = new AudioGeneratorWAV();
  if (!wav) {
    delete source; source = nullptr;
    return PLAYER_ERR_INIT;
  }

  if (!wav->begin(source, out)) {
    Serial.println("[player] wav->begin failed!");
    delete wav; wav = nullptr;
    delete source; source = nullptr;
    return PLAYER_ERR_FILE;
  }

  // store filename
  strncpy(filename, path, sizeof(filename) - 1);
  filename[sizeof(filename) - 1] = '\0';

  // reset timing
  startMs = millis();
  pausedMs = 0;
  pauseStart = 0;
  durationSec = 0;

  state = PLAYER_PLAYING;
  Serial.println("[player] playback started");
  return PLAYER_OK;
}

void player_Update(void) {
  if (state != PLAYER_PLAYING) return;
  if (!wav || !wav->isRunning()) {
    // track finished
    if (state == PLAYER_PLAYING) {
      Serial.println("[player] EOF - stopping");
      player_Stop();
    }
    return;
  }

  // feed data from sd to i2s
  if (!wav->loop()) {
    Serial.println("[player] wav->loop() returned false - stopping");
    player_Stop();
  }
}

void player_Pause(void) {
  if (state == PLAYER_PLAYING) {
    state = PLAYER_PAUSED;
    pauseStart = millis();
    // stop feeding loop() and zero output
    out->stop();
    Serial.println("[player] paused");

  } else if (state == PLAYER_PAUSED) {
    state = PLAYER_PLAYING;
    pausedMs += (millis() - pauseStart);
    out->begin();
    Serial.println("[player] resumed");
  }
}

PlayerState player_GetState(void) {
  return state;
}

uint32_t player_GetElapsedSec(void) {
  if (state == PLAYER_STOPPED) return 0;
  uint32_t elapsed = millis() - startMs - pausedMs;
  if (state == PLAYER_PAUSED) {
    elapsed -= (millis() - pauseStart);
  }
  return elapsed / 1000;
}

uint32_t player_GetDurationSec(void) {
  return durationSec;
}

const char* player_GetFilename(void) {
  return filename;
}

