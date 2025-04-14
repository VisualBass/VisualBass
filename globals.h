#ifndef GLOBALS_H
#define GLOBALS_H
<<<<<<< HEAD
#include "Particle01.h"  // Make sure this path is correct

// Define FRAMES_PER_BUFFER
#define FRAMES_PER_BUFFER   2048
#define PARTICLES 2000
=======

// Define FRAMES_PER_BUFFER
#define FRAMES_PER_BUFFER   2048
>>>>>>> e3acf683c19700d64f5c0b8033b00208fd966701

// Global variables for audio processing and mode control
extern volatile int audioDataReady;
extern float gAudioBuffer[FRAMES_PER_BUFFER];
extern float glow_value;
extern bool escape_mode;
extern float hueShift;
extern bool autoCycleHue;

<<<<<<< HEAD
extern int numOrbs;  // Declare it globally, so you can use it throughout the program

// Declare MAX_PARTICLES as a constant integer
extern const int MAX_PARTICLES;  // No need for an array, just an integer

// Declare the particle system globally
extern Particle01 particleSystem;
=======
// Add other global variables here...
>>>>>>> e3acf683c19700d64f5c0b8033b00208fd966701

#endif // GLOBALS_H
