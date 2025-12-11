#include "globals.h"
#include "Particle01.h"  // Make sure this path is correct

// Define the global variables
volatile int audioDataReady = 0;
float gAudioBuffer[FRAMES_PER_BUFFER];
float glow_value = 0.0f;
bool escape_mode = false;
float hueShift = 0.0f;
bool autoCycleHue = true;  // true by default; user can disable it via the menu.
int numOrbs = 4000;  // Default number of orbs

// Define orbColor globally, initialized to a default color (e.g., white)
Color orbColor = WHITE;

// Define MAX_PARTICLES as a constant integer, not an array
const int MAX_PARTICLES = PARTICLES;  // Set MAX_PARTICLES to 3000

const char* MODE_NAMES[] = {
    "Waveform",
    "Gravity Orbs",
    "Cube Field",
    "Particle System"
};