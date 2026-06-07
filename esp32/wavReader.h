#ifndef WAVREADER_H
#define WAVREADER_H

#include <SD.h>

// WAV header data from file
typedef struct {
  uint16_t numChannels;
  uint32_t sampleRate;
  uint16_t bitsPerSample;
  uint32_t dataSize;
  uint32_t dataOffset;
} WavHeader;

// error codes
typedef enum {
  WAV_OK,
  WAV_ERR_OPEN,
  WAV_ERR_NOT_WAV,
  WAV_ERR_NO_FMT,
  WAV_ERR_NO_DATA,
  WAV_ERR_FORMAT
} WavResult;

/*
  Open a WAV file and parse its header
  file remains open and positioned at start of PCM data
  returns WAV_OK on success and stores header
*/
WavResult wav_Open(const char* path, File* file, WavHeader* header);

/*
  Read up to bufLen bytes of PCM data into buf
  returns number of bytes read
*/
int wav_Read(File* file, uint8_t* buf, int bufLen);

// Rewind to start of PCM data  
void wav_Rewind(File* file, const WavHeader* header);

// Close file
void wav_Close(File* file);

// Error messages
const char* wav_ErrorStr(WavResult result);

#endif

