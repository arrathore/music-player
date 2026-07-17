#ifndef ALBUMVIEW_H
#define ALBUMVIEW_H

#include "App.h"
#include "../driver/switch.h"

#define ALBUM_MAX_TRACKS 32
#define ALBUM_VISIBLE_TRACKS 4

// Layout constants
#define AV_COVER_X       0
#define AV_COVER_Y       0
#define AV_COVER_W       64
#define AV_COVER_H       64
#define AV_TEXT_X        68   // right of cover, 4px gap
#define AV_TITLE_Y       4
#define AV_ARTIST_Y      18
#define AV_YEAR_Y        32
#define AV_DIVIDER_Y     66  // horizontal line between header and tracklist
#define AV_TRACK_START_Y 70  // y of first track row
#define AV_TRACK_H       15  // px per track row
#define AV_SCROLLBAR_X   157

class AlbumViewApp : public App {
 public:
  // Called by browser when entering a folder with meta.txt
  // path is the album folder path
  void setPath(const char* path);

  void init() override;
  void update() override;
  void handleEvent(SwitchEvent e) override;
  void deinit() override;

 private:
  // Album metadata from meta.txt
  char albumPath[256];
  char title[64];
  char artist[64];
  char genre[32];
  char year[8];
  char tracks[ALBUM_MAX_TRACKS][128];
  int trackCount = 0;

  // UI state
  int selectedIdx = 0; // 0-based index of cursor in tracks[]
  int scrollOffset = 0; // index of first visible tracks

  // Parsing
  bool parseMeta(void);

  // Drawing
  void drawHeader(void);
  void drawCover(void);
  void drawScrollbar(void);
  void drawTrackList(void);
  void drawTrack(int trackIdx, int screenRow);
  void drawDivider(void);

  // Navigation
  void clampScroll(void);

  void cursorDown(void);
  void cursorUp(void);
  void enter(void);
  void exit(void);
};

#endif

