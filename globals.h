#ifndef GLOBALS_H
#define GLOBALS_H
#include "Particle01.h"  // Make sure this path is correct

// Define FRAMES_PER_BUFFER
#define FRAMES_PER_BUFFER   2048
#define PARTICLES 2000

// Global variables for audio processing and mode control
extern volatile int audioDataReady;
extern float gAudioBuffer[FRAMES_PER_BUFFER];
extern float glow_value;
extern bool escape_mode;
extern float hueShift;
extern bool autoCycleHue;
extern const char* MODE_NAMES[]; // Declare a global list of names

extern int numOrbs;  // Declare it globally, so you can use it throughout the program

// Declare MAX_PARTICLES as a constant integer
extern const int MAX_PARTICLES;  // No need for an array, just an integer

// Declare the particle system globally
extern Particle01 particleSystem;

#endif // GLOBALS_H
