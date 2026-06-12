#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>

// Playback state
typedef enum {
  PLAYER_STOPPED = 0,
  PLAYER_PLAYING = 1,
  PLAYER_PAUSED = 2,
} PlayerState;

// Error codes
typedef enum {
  PLAYER_OK = 0,
  PLAYER_ERR_INIT = 1, // I2S init failed
  PLAYER_ERR_FILE = 2, // problem with file
} PlayerResult;

// Initialize I2S
PlayerResult player_Init(void);

// Open and play WAV file from SD card
PlayerResult player_Open(const char* path);

// Pause / resume toggle
void player_Pause(void);

// Stop playback and release resources
void player_Stop(void);

// Drive audio pipeline, called every loop()
void player_Update(void);

// Status query
PlayerState player_GetState(void);

// Returns elapsed playback time in seconds
uint32_t player_GetElapsedSec(void);

// Returns total track duration in seconds from header
uint32_t player_GetDurationSec(void);

// Returns filename of current track, empty string if none
const char* player_GetFilename(void);

#endif

  
