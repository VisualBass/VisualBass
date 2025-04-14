#include "globals.h"
<<<<<<< HEAD
#include "Particle01.h"  // Make sure this path is correct
=======
>>>>>>> e3acf683c19700d64f5c0b8033b00208fd966701

// Define the global variables
volatile int audioDataReady = 0;
float gAudioBuffer[FRAMES_PER_BUFFER];
float glow_value = 0.0f;
bool escape_mode = false;
float hueShift = 0.0f;
bool autoCycleHue = true;  // true by default; user can disable it via the menu.
<<<<<<< HEAD
int numOrbs = 400;  // Default number of orbs

// Define orbColor globally, initialized to a default color (e.g., white)
Color orbColor = WHITE;

// Define MAX_PARTICLES as a constant integer, not an array
const int MAX_PARTICLES = PARTICLES;  // Set MAX_PARTICLES to 3000
=======

>>>>>>> e3acf683c19700d64f5c0b8033b00208fd966701
