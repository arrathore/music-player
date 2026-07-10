#include "metadata.h"

#include "../driver/sdCard.h"

#include <Arduino.h>

#include "AudioTools.h"
#include "AudioMetaData/MetaDataID3.h"

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

/********************
 * ID3 metadata parser
 ********************/

// temp struct for callback to populate
static TrackMetadata* id3Target = nullptr;

static void id3Callback(MetaDataType type, const char* str, int len) {
  if (id3Target == nullptr) return;

  // copy into the right field, clamping to buffer size
  switch (type) {
    case Title:
      strncpy(id3Target->title, str, sizeof(id3Target->title) - 1);
      break;
    case Artist:
      strncpy(id3Target->artist, str, sizeof(id3Target->artist) - 1);
      break;
    case Album:
      strncpy(id3Target->album, str, sizeof(id3Target->album) - 1);
      break;
    default:
      break;
  }
}

// get duration from Xing/Info header
static uint32_t readXingDuration(File& f, uint32_t id3Size) {
  f.seek(id3Size);

  uint8_t buf[256];
  int bytesRead = f.read(buf, sizeof(buf));
  if (bytesRead < 40) return 0;

  // find MP3 sync word
  int syncPos = -1;
  for (int i = 0; i < bytesRead - 1; i++) {
    if (buf[i] == 0xFF && (buf[i + 1] & 0xE0) == 0xE0) {
      syncPos = i;
      break;
    }
  }
  if (syncPos < 0) return 0;

  // parse MP3 frame header to get sample rate and layer
  uint8_t* h = &buf[syncPos];
  uint8_t bitrateIdx = (h[2] >> 4) & 0x0F;
  uint8_t sampleIdx = (h[2] >> 2) & 0x03;
  uint8_t channelMode = (h[3] >> 6) & 0x03;

  const uint32_t sampleRates[] = {44100, 48000, 32000, 0};
  const uint32_t sampleRate = sampleRates[sampleIdx];
  if (sampleRate == 0) return 0;

  // Xing/Info header starts 32 bytes into frame for stereo
  // 17 bytes for mono
  int xingOffset = syncPos + 4 + (channelMode == 3 ? 17: 32);
  if (xingOffset + 12 > bytesRead) return 0;

  uint8_t* xing = &buf[xingOffset];

  // check for "Xing" or "Info" marker
  if (memcmp(xing, "Xing", 4) != 0 && memcmp(xing, "Info", 4) != 0)
    return 0;

  // Xing flags field tells us which optional fields are present
  uint32_t flags = ((uint32_t)xing[4] << 24) | ((uint32_t)xing[5] << 16) |
    ((uint32_t)xing[6] << 8) | xing[7];

  if (!(flags & 0x01)) return 0; // frame count field not present

  uint32_t frameCount = ((uint32_t)xing[8] << 24) | ((uint32_t)xing[9] << 16) |
    ((uint32_t)xing[10] << 8) | xing[11];

  // each MPEG1 Layer3 frame contains 1152 samples
  return (frameCount * 1152) / sampleRate;
}

// get rough duration estimate from the file size
// fallback if both other methods fail
static uint32_t estimateDurationFromFileSize(File& f, uint32_t id3Size) {
  uint32_t audioBytes = f.size() - id3Size;
  const uint32_t ASSUMED_BITRATE_BPS = 128000; // assume 128kpbs
  return (audioBytes * 8) / ASSUMED_BITRATE_BPS;
}

// read size from ID3v2 header
static uint32_t readID3v2Size(File& f) {
  /*
    bytes 0-2: "ID3"
    bytes 3-4: version
    byte 5: flags
    bytes 6-9: size
  */
  uint8_t header[10];
  f.seek(0);
  if (f.read(header, 10) < 10) return 0;

  // check ID3 marker
  if (header[0] != 'I' || header[1] != 'D' || header[2] != '3') return 0;

  // synchsafe integer
  uint32_t size = ((uint32_t)(header[6] & 0x7F) << 21) |
    ((uint32_t)(header[7] & 0x7F) << 14) |
    ((uint32_t)(header[8] & 0x7F) <<  7) |
    (uint32_t)(header[9] & 0x7F);

  return size + 10;  // +10 for the header itself  

}

static MetaResult readID3Meta(const char* path, TrackMetadata* meta) {
  File f = sd_OpenFile(path);
  if (!f) return META_ERR_OPEN;

  // get ID3 block size
  uint32_t id3Size = readID3v2Size(f);
  Serial.printf("[metadata] ID3v2 block size: %lu\n", id3Size);

  // point the static target at meta struct for callback
  f.seek(0);
  id3Target = meta;

  MetaDataID3 id3;
  id3.setCallback(id3Callback);
  id3.begin();

  // buffer for data
  const int CHUNK = 256;
  uint8_t buf[CHUNK];
  int totalRead = 0;
  const int MAX_READ = 16384; // 16kb

  while (f.available() && totalRead < MAX_READ) {
    int bytesRead = f.read(buf, CHUNK);
    if (bytesRead <= 0) break;
    id3.write(buf, bytesRead);
    totalRead += bytesRead;

    // stop when we get all fields
    if (strlen(meta->title) > 0 &&
	strlen(meta->artist) > 0 &&
	strlen(meta->album) > 0) break;
  }

  //  uint32_t id3Size = totalRead;
  id3.end();

  // duration fallback chain
  if (meta->durationSec == 0) {
    // try Xing/Info header
    meta->durationSec = readXingDuration(f, id3Size);
    Serial.printf("[metadata] Xing duration: %lus\n", meta->durationSec);
  }
  if (meta->durationSec == 0) {
    // file size estimate
    meta->durationSec = estimateDurationFromFileSize(f, id3Size);
    Serial.printf("[metadata] estimated duration: %lus\n", meta->durationSec);
  }
  
  sd_CloseFile(f);
  id3Target = nullptr;

  // fall back to filename if not title found
  if (strlen(meta->title) == 0) titleFromPath(path, meta);

  Serial.printf("[metadata] %s | title: %s | artist: %s | album: %s | duration: %lus\n",
		path, meta->title, meta->artist, meta->album, meta->durationSec);
  
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

