# Music Player
A standalone device to play music files.

## Requirements and Development Phases

### Phase 1: Basics
* Read files (wav) from SD card
* Have some basic UI and controls for file selection
* Play audio through headphone jack

### Phase 2: More formats better UI
* Use encoded formats (MP3, OGG)
  - hardware or software encoding
* Metadata tagging
* Improved, mp3 player style UI
* Full feature wired mp3 player

### Phase 3: Bluetooth
* Stream audio wirelessly
  - built-in with ESP32 or dedicated module

### Phase 4: Wi-Fi and online management
* Manage music library from locally hosted portal
* Playlists
* Load new files wirelessly

## Architecture
Microcontroller: ESP-WROOM-32 / ESP32C6
SD Card: ST7735R
Display: ST7735R
DAC: PCM5102A
Amplifier: MAX98357A
Controls: Tactile buttons



