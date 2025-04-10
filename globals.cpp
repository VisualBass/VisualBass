#include "globals.h"

// Define the global variables
volatile int audioDataReady = 0;
float gAudioBuffer[FRAMES_PER_BUFFER];
float glow_value = 0.0f;
bool escape_mode = false;
float hueShift = 0.0f;
bool autoCycleHue = true;  // true by default; user can disable it via the menu.

