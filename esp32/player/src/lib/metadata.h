#ifndef METADATA_H
#define METADATA_H

#include <Arduino.h>

// Metadata struct
typedef struct {
  char title[64];
  char artist[64];
  char album[64];
  uint32_t durationSec;
  uint32_t sampleRate;
  uint8_t channels;
  uint8_t bitsPerSample;
} TrackMetadata;

typedef enum {
  META_OK = 0,
  META_ERR_OPEN = 1, // File couldn't be opened
  META_ERR_UNKNOWN = 2, // Format is not recognized
  META_ERR_INVALID = 3, // File is recognized but malformed
} MetaResult;

// Read metadata from any supported file
MetaResult metadata_Read(const char* path, TrackMetadata* meta);

// Return a human-readable error string for a MetaResult
const char* metadata_ErrorStr(MetaResult result);

#endif

