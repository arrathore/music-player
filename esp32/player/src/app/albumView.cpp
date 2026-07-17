#include "albumView.h"

#include "appManager.h"
#include "../driver/display.h"
#include "../driver/sdCard.h"
#include "../lib/player.h"

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#define LIGHTGREY 0xC618
#define DARKGREY  0x7BEF

void AlbumViewApp::setPath(const char* path) {
  strncpy(albumPath, path, sizeof(albumPath) - 1);
  albumPath[sizeof(albumPath) - 1] = '\0';
}

/********************
 * APP INTERFACE
 ********************/
void AlbumViewApp::init() {
  // reset state
  title[0] = '\0';
  artist[0] = '\0';
  genre[0] = '\0';
  year[0] = '\0';
  trackCount = 0;
  selectedIdx = 0;
  scrollOffset = 0;

  // parse meta.txt
  if (!parseMeta()) {
    Serial.printf("[albumView] failed to parse meta.txt in %s\n", albumPath);

    // fall back to folder name as title
    const char* name = strrchr(albumPath, '/');
    strncpy(title, name ? name + 1 : albumPath, sizeof(title) - 1);
  }

  // draw the full screen
  display_Clear();
  drawHeader();
  drawDivider();
  drawTrackList();
}

void AlbumViewApp::update() {
  // no per-frame logic
}

void AlbumViewApp::deinit() {
  // nothing to do
}

void AlbumViewApp::handleEvent(SwitchEvent e) {
  switch (e) {
    case SWITCH_ENTER: enter(); break;
    case SWITCH_DOWN: cursorDown(); break;
    case SWITCH_UP: cursorUp(); break;
    case SWITCH_BACK: exit(); break;
    default: break;
  }
}

/********************
 * PARSING
 ********************/
bool AlbumViewApp::parseMeta() {
  // build path to meta.txt
  char metaPath[270];
  if (strcmp(albumPath, "/") == 0)
    snprintf(metaPath, sizeof(metaPath), "/meta.txt");
  else
    snprintf(metaPath, sizeof(metaPath), "%s/meta.txt", albumPath);

  File f = sd_OpenFile(metaPath);
  if (!f) {
    Serial.printf("[albumView] meta.txt not found: %s\n", metaPath);
    return false;
  }

  typedef enum { SECTION_NONE, SECTION_META, SECTION_TRACKS } Section;
  Section section = SECTION_NONE;

  while (f.available()) {
    // read a line
    char line[128];
    int len = 0;
    while (f.available() && len < (int)sizeof(line) - 1) {
      char c = f.read();
      if (c == '\n') break;
      if (c != '\r') line[len++] = c;
    }
    line[len] = '\0';

    // skip empty lines and comments
    if (len == 0 || line[0] == '#') continue;

    // section headers
    if (strcmp(line, "[meta]") == 0) {
      section = SECTION_META;
      continue;
    }

    if (strcmp(line, "[tracks]") == 0) {
      section = SECTION_TRACKS;
      continue;
    }

    if (section == SECTION_META) {
      // parse key: value
      char* colon = strchr(line, ':');
      if (colon == nullptr) continue;

      // split into key and value, trim whitespace
      *colon = '\0';
      char* key = line;
      char* value = colon + 1;

      // trim leading whitespace from value
      while (*value == ' ') value++;

      // trim trailing whitespace from key
      char* keyEnd = key + strlen(key) - 1;
      while (keyEnd > key && *keyEnd == ' ') *keyEnd-- = '\0';

      // case-insensitive key match
      if (strcasecmp(key, "Title")  == 0) strncpy(title, value, sizeof(title)  - 1);
      else if (strcasecmp(key, "Artist") == 0) strncpy(artist, value, sizeof(artist) - 1);
      else if (strcasecmp(key, "Genre")  == 0) strncpy(genre, value, sizeof(genre) - 1);
      else if (strcasecmp(key, "Year")   == 0) strncpy(year, value, sizeof(year) - 1);
      
    } else if (section == SECTION_TRACKS) {
      // each line is a filename
      if (trackCount < ALBUM_MAX_TRACKS) {
	strncpy(tracks[trackCount], line, sizeof(tracks[trackCount]) - 1);
	tracks[trackCount][sizeof(tracks[trackCount]) - 1] = '\0';
	trackCount++;
      }
    }
  }

  sd_CloseFile(f);

  Serial.printf("[albumView] parsed: title=%s artist=%s year=%s tracks=%d\n",
		title, artist, year, trackCount);

  return true;
}

/********************
 * DRAWING
 ********************/
void AlbumViewApp::drawCover() {
  char coverPath[270];
  if (strcmp(albumPath, "/") == 0)
    snprintf(coverPath, sizeof(coverPath), "/cover.bmp");
  else
    snprintf(coverPath, sizeof(coverPath), "%s/cover.bmp", albumPath);

  display_DrawBMP(coverPath, AV_COVER_X, AV_COVER_Y);

  // placeholder if cover is missing
  if (!sd_FileExists(coverPath)) {
    display_FillRect(AV_COVER_X, AV_COVER_Y, AV_COVER_W, AV_COVER_H, DARKGREY);
    display_SetCursor(AV_COVER_X + 20, AV_COVER_Y + 28);
    display_Print("?", ST77XX_WHITE);
  }
}

void AlbumViewApp::drawHeader() {
  // cover image
  drawCover();

  // title
  String titleStr = String(title);
  if (titleStr.length() <= 15) {
    // fits on one line
    display_SetCursor(AV_TEXT_X, AV_TITLE_Y);
    display_Print(titleStr, ST77XX_WHITE);
  } else {
    // split across two lines at a word boundary if possible
    int splitPos = 15;
    // search backwards for a space to break on
    for (int i = 15; i > 0; i--) {
      if (titleStr[i] == ' ') { splitPos = i; break; }
    }
    display_SetCursor(AV_TEXT_X, AV_TITLE_Y);
    display_Print(titleStr.substring(0, splitPos), ST77XX_WHITE);
    display_SetCursor(AV_TEXT_X, AV_TITLE_Y + 8);
    // truncate second line with - if still too long
    String line2 = titleStr.substring(splitPos + 1);
    if (line2.length() > 15) line2 = line2.substring(0, 14) + "-";
    display_Print(line2, ST77XX_WHITE);
  }

  // artist
  int artistY = (strlen(title) > 15) ? AV_ARTIST_Y + 22 : AV_TITLE_Y + 12;
  display_SetCursor(AV_TEXT_X, artistY);
  String artistStr = String(artist);
  if (artistStr.length() > 15) artistStr = artistStr.substring(0, 14) + "-";
  display_Print(artistStr, ST77XX_YELLOW);

  // year
  if (strlen(year) > 0) {
    display_SetCursor(AV_TEXT_X, artistY + 12);
    display_Print(String(year), ST77XX_YELLOW);
  }
}

void AlbumViewApp::drawDivider() {
  display_FillRect(0, AV_DIVIDER_Y, 160, 1, ST77XX_WHITE);
}

void AlbumViewApp::drawScrollbar() {
  if (trackCount <= ALBUM_VISIBLE_TRACKS) return; // no scrollbar required

  int barAreaH = ALBUM_VISIBLE_TRACKS * AV_TRACK_H;
  int barH = max(4, barAreaH * ALBUM_VISIBLE_TRACKS / trackCount);
  int barY = AV_TRACK_START_Y + barAreaH * scrollOffset / trackCount;

  // background
  display_FillRect(AV_SCROLLBAR_X, AV_TRACK_START_Y, 3, barAreaH, ST77XX_YELLOW);
  // filled portion
  display_FillRect(AV_SCROLLBAR_X, barY, 3, barH, ST77XX_WHITE);
}

void AlbumViewApp::drawTrack(int trackIdx, int screenRow) {
  int y = AV_TRACK_START_Y + screenRow * AV_TRACK_H;
  display_ClearLine(y);
  display_SetCursor(0, y);
 
  // cursor indicator
  if (trackIdx == selectedIdx) {
    display_Print(">", ST77XX_MAGENTA);
  } else {
    display_Print(" ", ST77XX_WHITE);
  }
 
  // strip extension
  char displayName[64];
  strncpy(displayName, tracks[trackIdx], sizeof(displayName) - 1);
  displayName[sizeof(displayName) - 1] = '\0';
  char* dot = strrchr(displayName, '.');
  if (dot != nullptr) *dot = '\0';

  // truncate to fit
  int maxChars = (trackCount > ALBUM_VISIBLE_TRACKS) ? 24 : 25;
  
  String nameStr = String(displayName);
  if (nameStr.length() > maxChars) nameStr = nameStr.substring(0, maxChars - 1) + "-";
 
  uint16_t color = (trackIdx == selectedIdx) ? ST77XX_WHITE : LIGHTGREY;
  display_Print(nameStr, color);

  drawScrollbar();
}
 
void AlbumViewApp::drawTrackList() {
  int visibleCount = min(ALBUM_VISIBLE_TRACKS, trackCount - scrollOffset);
  for (int row = 0; row < visibleCount; row++) {
    drawTrack(scrollOffset + row, row);
  }
 
  // scrollbar if content overflows
  drawScrollbar();
}

/********************
 * NAVIGATION
 ********************/

void AlbumViewApp::clampScroll() {
  if (selectedIdx < scrollOffset) {
    scrollOffset = selectedIdx;
  } else if (selectedIdx >= scrollOffset + ALBUM_VISIBLE_TRACKS) {
    scrollOffset = selectedIdx - ALBUM_VISIBLE_TRACKS + 1;
  }
}

void AlbumViewApp::cursorDown() {
  if (trackCount == 0) return;
  int prev = selectedIdx;
  int prevScroll = scrollOffset; // save before clamp
  
  selectedIdx = (selectedIdx + 1) % trackCount; // wrap around
  clampScroll();

  // handle scrolling
  if (scrollOffset != prevScroll) {
    drawTrackList();
  } else { // update changed rows
    drawTrack(prev, prev - scrollOffset);
    drawTrack(selectedIdx, selectedIdx - scrollOffset);
  }
}

void AlbumViewApp::cursorUp() {
  if (trackCount == 0) return;
  int prev = selectedIdx;
  int prevScroll = scrollOffset;
  
  selectedIdx = (selectedIdx - 1 + trackCount) % trackCount; // wrap around
  clampScroll();

  if (scrollOffset != prevScroll) {
    drawTrackList();
  } else {
    drawTrack(prev, prev - scrollOffset);
    drawTrack(selectedIdx, selectedIdx - scrollOffset);
  }
}

// start playing album in order from user selection
void AlbumViewApp::enter() {
  if (trackCount == 0) return;
  Serial.printf("[albumView] selected track: %s/%s\n", albumPath, tracks[selectedIdx]);

  player_SetQueue(albumPath, tracks, trackCount, selectedIdx);
  appManager_SwitchTo(appManager_GetNowPlaying());
}

void AlbumViewApp::exit() {
  Serial.println("[albumView] exiting");
  appManager_SwitchTo(appManager_GetBrowser());
}

