#include "player.h"

#include "../pins.h"
#include "metadata.h"
#include "../driver/sdCard.h"
#include "../app/albumView.h"

#include <Arduino.h>

#include "AudioTools.h"
#include "AudioLibs/AudioSourceSDFAT.h"

#include "AudioCodecs/CodecWAV.h"
#include "AudioCodecs/CodecMP3Helix.h"

// AudioTools objects
static I2SStream i2s;
static SdSpiConfig sdConfig(PIN_SD_CS, DEDICATED_SPI, SD_SCK_MHZ(4));
static SdFat32 sd;
static File file;
static WAVDecoder wavDecoder;
static MP3DecoderHelix mp3Decoder;
static EncodedAudioStream decStream;
static StreamCopy copier(decStream, file);
static bool fileOpen = false;

// internal state
static PlayerState state = PLAYER_STOPPED;
static char filename[256] = "";
static TrackMetadata currentMeta;

static char queue[ALBUM_MAX_TRACKS][128];
static int queueCount = 0;
static int queueIdx = 0;

static uint32_t startMs = 0; // millis() when playback started
static uint32_t pausedMs = 0; // accumulated paused time
static uint32_t pauseStart = 0; // millis() when pause began
static uint32_t durationSec = 0;


PlayerResult player_Init(void) {
  // configure I2S output
  auto cfg = i2s.defaultConfig(TX_MODE);
  cfg.pin_bck = PIN_I2S_BCLK;
  cfg.pin_ws = PIN_I2S_LRCLK;
  cfg.pin_data = PIN_I2S_DATA;
  cfg.sample_rate = 44100;
  cfg.channels = 2;
  cfg.bits_per_sample = 16;

  if (!i2s.begin(cfg)) {
    Serial.println("[player] I2S init failed");
    return PLAYER_ERR_INIT;
  }

  Serial.println("[player] init OK");
  return PLAYER_OK;
}

// detect format from extension and select decoder
static PlayerResult getDecoderForFile(const char* path, AudioDecoder** decoder) {
  // get extension
  const char* ext = strrchr(path, '.');
  if (ext == nullptr) {
    Serial.println("[player] no file extension");
    return PLAYER_ERR_FILE;
  }

  // set decoder
  if (strcasecmp(ext, ".wav") == 0) *decoder = &wavDecoder;
  else if (strcasecmp(ext, ".mp3") == 0) *decoder = &mp3Decoder;
  else {
    Serial.printf("[player] unsupported format: %s\n", ext);
    return PLAYER_ERR_FILE;
  }

  return PLAYER_OK;
}

PlayerResult player_Open(const char* path) {
  Serial.printf("[player] player_Open called with path %s\n", path);
  
  // stop before starting anything
  player_Stop();

  // read metadata
  MetaResult mr = metadata_Read(path, &currentMeta);
  if (mr != META_OK)
    Serial.printf("[player] metadata failed: %s\n", metadata_ErrorStr(mr));

  // open file
  file = sd_OpenFile(path);
  if (!file) {
    Serial.printf("[player] failed to open: %s\n", path);
    return PLAYER_ERR_FILE;
  }
  fileOpen = true;

  // select decoder
  AudioDecoder* decoder = nullptr;
  if (getDecoderForFile(path, &decoder) != PLAYER_OK) {
    Serial.printf("[player] failed to get decoder");
    file.close();
    fileOpen = false;
    return PLAYER_ERR_FILE;
  }

  // configure stream with selected decoder
  decStream.setStream(&i2s);
  decStream.setDecoder(decoder);
  
  // begin decoder and I2S
  decStream.begin();

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

// called every loop()
void player_Update(void) {
  if (state != PLAYER_PLAYING) return;

  // feed audio from SD to decoder to I2S
  if (!copier.copy()) {
    Serial.println("[player] EOF");

    if (queueIdx >= queueCount)
      player_StopAndEnd();
    else
      player_Open(queue[++queueIdx]);
  }
}

void player_Pause(void) {
  if (state == PLAYER_PLAYING) {
    state = PLAYER_PAUSED;
    pauseStart = millis();
    i2s.writeSilence(256); // flush with silence to avoid held note
    Serial.println("[player] paused");

  } else if (state == PLAYER_PAUSED) {
    pausedMs += (millis() - pauseStart);
    state = PLAYER_PLAYING;
    Serial.println("[player] resumed");
  }
}

void player_Skip(void) {
  if (queueIdx < queueCount) {
    player_Stop();
    player_Open(queue[++queueIdx]);
    
  } else // else, reached end of queue, stop and end
    player_StopAndEnd();
}

void player_Prev(void) {
  player_Stop();
  if (queueIdx != 0)
    player_Open(queue[--queueIdx]);
  else
    player_Open(queue[queueIdx]); // restart this track
}

void player_Stop(void) {
  if (fileOpen) {
    sd_CloseFile(file);
    fileOpen = false;
  }

  decStream.end();
  state = PLAYER_STOPPED;
  filename[0] = '\0';
  startMs = 0;
  pausedMs = 0;
  pauseStart = 0;

  Serial.println("[player] stopped");
}

void player_StopAndEnd(void) {
  player_Stop();
  queueCount = 0;
  queueIdx = 0;
}

PlayerResult player_SetQueue(const char* albumPath, char tracks[][128], int trackCount, int startIdx) {
  // reset queue
  queueCount = 0;
  queueIdx = 0;

  // build queue
  for (int i = startIdx, j = 0; i < trackCount; i++, j++) {
    snprintf(queue[j], 128, "%s/%s", albumPath, tracks[i]);
    queueCount++;
  }

  player_Open(queue[queueIdx]);
  return PLAYER_OK;
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

const TrackMetadata* player_GetMetadata(void) {
  return &currentMeta;
}
  
