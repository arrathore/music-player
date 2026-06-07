#include "wavReader.h"

#include <Arduino.h>
#include <SD.h>

// helpers to read litle-endian integers
static uint16_t readU16(File* f) {
  uint8_t buf[2];
  f->read(buf, 2);
  return (uint16_t)(buf[0] | buf[1] << 8);
}

static uint32_t readU32(File* f) {
  uint8_t buf[4];
  f->read(buf, 4);
  return (uint32_t(buf[0] | buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24));
}

// read 4-byte chunk ID into char[4]
static void readChunkID(File* f, char id[4]) {
  f->read((uint8_t*)id, 4);
}

// compare chunk ID to string
static bool chunkIs(const char id[4], const char* label) {
  return memcmp(id, label, 4) == 0;
}

WavResult wav_Open(const char* path, File* file, WavHeader* header) {
  *file = SD.open(path);
  if (!*file) {
    return WAV_ERR_OPEN;
  }

  // RIFF descriptor - 12 bytes
  char chunkID[4];
  readChunkID(file, chunkID);
  if (!chunkIs(chunkID, "RIFF")) { // didn't get RIFF
    file->close();
    return WAV_ERR_NOT_WAV;
  }

  readU32(file); // skip file size

  char format[4];
  readChunkID(file, format);
  if (!chunkIs(format, "WAVE")) { // format is not WAVE
    file->close();
    return WAV_ERR_NOT_WAV;
  }

  // search for fmt and data chunks
  bool fmtFound = false;
  bool dataFound = false;

  memset(header, 0, sizeof(WavHeader));

  while (file->available()) {
    char id[4];
    readChunkID(file, id);
    uint32_t chunkSize = readU32(file); // size of this chunk payload

    if (chunkIs(id, "fmt ")) { // fmt chunk
      uint16_t audioFormat = readU16(file); // 1 = PCM
      if (audioFormat != 1) { // non-PCM (compressed) not supported
	file->close();
	return WAV_ERR_FORMAT;
      }

      header->numChannels = readU16(file);
      header->sampleRate = readU32(file);
      readU32(file); // skip byte rate
      readU16(file); // skip block align
      header->bitsPerSample = readU16(file);

      // skip extra fmt bytes
      if (chunkSize > 16) file->seek(file->position() + (chunkSize - 16));

      fmtFound = true;
      
    } else if (chunkIs(id, "data")) { // data chunk
      header->dataSize = chunkSize;
      header->dataOffset = file->position(); // PCM data start
      dataFound = true;
      break; // data chunk is always last
      
    } else { // unknown chunk
      // skip and continue
      uint32_t skip = chunkSize + (chunkSize & 1);
      file->seek(file->position() + skip);
    }
  }

  if (!fmtFound) {
    file->close();
    return WAV_ERR_NO_FMT;
  }

  if (!dataFound) {
    file->close();
    return WAV_ERR_NO_DATA;
  }

  return WAV_OK;
}

int wav_Read(File* file, uint8_t* buf, int bufLen) {
  if (!file->available()) return 0;
  return file->read(buf, bufLen);
}

void wav_Rewind(File* file, const WavHeader* header) {
  file->seek(header->dataOffset);
}

void wav_Close(File* file) {
  if (*file) file->close();
}

const char* wav_ErrorStr(WavResult result) {
  switch(result) {
    case WAV_OK:          return "OK";
    case WAV_ERR_OPEN:    return "Could not open file";
    case WAV_ERR_NOT_WAV: return "Not a WAV file";
    case WAV_ERR_NO_FMT:  return "fmt chunk not found";
    case WAV_ERR_NO_DATA: return "data chunk not found";
    case WAV_ERR_FORMAT:  return "Unsupported format (non-PCM)";
    default:              return "Unknown error";
  }
}
