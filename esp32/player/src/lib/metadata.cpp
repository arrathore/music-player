#include "metadata.h"

#include <Arduino.h>
#include "../driver/sdCard.h"

/********************
 * HELPERS - little endian reads
 ********************/

static uint16_t readU16(File& f) {
  uint8_t buf[2];
  f.read(buf, 2);
  return (uint16_t)(buf[0] | (buf[1] << 8));
}

static uint32_t readU32(File& f) {
  uint8_t buf[4];
  f.read(buf, 4);
  return (uint32_t)(buf[0] | (buf[1] << 8) | (buf[3] << 24));
}

static void readChunkID(File& f, char id[4]) {
  f.read((uint8_t*)id, 4);
}

static bool chunkIs(const char id[4], const char* label) {
  return memcmp(id, label, 4) == 0;
}

/********************
 * WAV INFO chunk parser
 ********************/
static void parseInfoChunk(File& f, uint32_t chunkSize, TrackMetadata* meta) {
  // read LIST type - should be "INFO"
  char listType[4];
  readChunkID(f, listType);
  if (!chunkIs(listType, "INFO")) {
    // not INFO, skip remaining bytes
    f.seek(f.position() + chunkSize - 4);
    return;
  }
 
  uint32_t bytesRead = 4;  // already read the type
 
  while (bytesRead < chunkSize) {
    if (f.available() < 8) break;
 
    char subID[4];
    readChunkID(f, subID);
    uint32_t subSize = readU32(f);
    bytesRead += 8;
 
    // clamp subSize to remaining chunk space
    uint32_t remaining = chunkSize - bytesRead;
    if (subSize > remaining) subSize = remaining;
 
    if (chunkIs(subID, "INAM") || chunkIs(subID, "IART") || chunkIs(subID, "IPRD")) {
      // read string value
      char value[64] = {0};
      uint32_t readLen = min((uint32_t)sizeof(value) - 1, subSize);
      f.read((uint8_t*)value, readLen);
 
      // skip remaining bytes in this subchunk
      if (subSize > readLen) {
        f.seek(f.position() + (subSize - readLen));
      }
 
      // remove trailing whitespace
      for (int i = strlen(value) - 1; i >= 0 && (value[i] == '\0' || value[i] == ' '); i--) {
        value[i] = '\0';
      }
 
      if (chunkIs(subID, "INAM")) {
        strncpy(meta->title,  value, sizeof(meta->title)  - 1);
      } else if (chunkIs(subID, "IART")) {
        strncpy(meta->artist, value, sizeof(meta->artist) - 1);
      } else if (chunkIs(subID, "IPRD")) {
        strncpy(meta->album,  value, sizeof(meta->album)  - 1);
      }
 
    } else {
      // skip unknown subchunk
      f.seek(f.position() + subSize);
    }
 
    bytesRead += subSize;
    // skip padding byte if odd size
    if (subSize & 1) {
      f.seek(f.position() + 1);
      bytesRead++;
    }
  }
}

/********************
 * WAV metadata parser
 ********************/
static void titleFromPath(const char* path, TrackMetadata* meta) {
  const char* name = strrchr(path, '/');
  name = (name != nullptr) ? name + 1 : path;
  strncpy(meta->title, name, sizeof(meta->title) - 1);
  meta->title[sizeof(meta->title) - 1] = '\0';
 
  // Strip extension
  char* dot = strrchr(meta->title, '.');
  if (dot != nullptr) *dot = '\0';
}


static MetaResult readWavMeta(const char* path, TrackMetadata* meta) {
  File f = SD.open(path);
  if (!f) return META_ERR_OPEN;
 
  // RIFF descriptor
  char chunkID[4];
  readChunkID(f, chunkID);
  if (!chunkIs(chunkID, "RIFF")) {
    f.close();
    return META_ERR_INVALID;
  }
 
  readU32(f);  // skip file size
 
  char format[4];
  readChunkID(f, format);
  if (!chunkIs(format, "WAVE")) {
    f.close();
    return META_ERR_INVALID;
  }
 
  bool fmtFound  = false;
  bool dataFound = false;
 
  while (f.available()) {
    char id[4];
    readChunkID(f, id);
    uint32_t chunkSize = readU32(f);
 
    if (chunkIs(id, "fmt ")) {
      uint16_t audioFormat = readU16(f);
      meta->channels = (uint8_t)readU16(f);
      meta->sampleRate = readU32(f);
      readU32(f); // skip byte rate
      readU16(f); // skip block align
      meta->bitsPerSample = (uint8_t)readU16(f);
 
      // only PCM (format 1) supported for now
      if (audioFormat != 1) {
        Serial.printf("[metadata] unsupported WAV format: %u\n", audioFormat);
      }
 
      // skip extended fmt bytes if present
      if (chunkSize > 16) {
        f.seek(f.position() + (chunkSize - 16));
      }
 
      fmtFound = true;
 
    } else if (chunkIs(id, "data")) {
      // duration = data size / byte rate
      // byte rate = sampleRate * channels * (bitsPerSample / 8)
      if (meta->sampleRate > 0 && meta->channels > 0 && meta->bitsPerSample > 0) {
        uint32_t byteRate = meta->sampleRate * meta->channels * (meta->bitsPerSample / 8);
        meta->durationSec = chunkSize / byteRate;
      }
      dataFound = true;
 
      // no need to read past the data chunk
      break;
 
    } else if (chunkIs(id, "LIST")) {
      // may contain INFO metadata
      parseInfoChunk(f, chunkSize, meta);
 
    } else {
      // skip unknown chunks
      uint32_t skip = chunkSize + (chunkSize & 1);
      f.seek(f.position() + skip);
    }
  }
 
  f.close();
 
  if (!fmtFound || !dataFound) return META_ERR_INVALID;
 
  // fall back to filename as title if no INFO chunk provided one
  if (strlen(meta->title) == 0) {
    titleFromPath(path, meta);
  }
 
  Serial.printf("[metadata] %s | title: %s | artist: %s | album: %s | duration: %lus\n",
    path, meta->title, meta->artist, meta->album, meta->durationSec);
 
  return META_OK;
}

static MetaResult readID3Meta(const char* path, TrackMetadata* meta) {
  // TODO: implement ID3 parsing
  // fall back to filename for now
  titleFromPath(path, meta);
  return META_OK;
}

/********************
 * PUBLIC API
 ********************/

MetaResult metadata_Read(const char* path, TrackMetadata* meta) {
  // zero out struct
  memset(meta, 0, sizeof(TrackMetadata));

  // dispatch by extension
  const char* ext = strrchr(path, '.');
  if (ext == nullptr) return META_ERR_UNKNOWN;
  if (strcasecmp(ext, ".wav") == 0) return readWavMeta(path, meta);
  if (strcasecmp(ext, ".mp3") == 0) return readID3Meta(path, meta);
  // TODO: add more formats
  else return META_ERR_UNKNOWN;
}

const char* metadata_ErrorStr(MetaResult result) {
  switch (result) {
    case META_OK: return "OK";
    case META_ERR_OPEN: return "Could not open file";
    case META_ERR_UNKNOWN: return "Unknown format";
    case META_ERR_INVALID: return "Invalid or malformed file";
    default: return "Unknown error";
  }
}

