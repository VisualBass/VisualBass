#ifndef GLOBALS_H
#define GLOBALS_H

#include "Particle01.h"
#include "raylib.h"

// Define FRAMES_PER_BUFFER
#define FRAMES_PER_BUFFER 2048
#define PARTICLES 2000

// Global variables for audio processing and mode control
extern volatile int audioDataReady;
extern float gAudioBuffer[FRAMES_PER_BUFFER];

// Visual Settings
extern float glow_value;
extern bool escape_mode;
extern float hueShift;
extern bool autoCycleHue;
extern int numOrbs;
extern Color orbColor;

// --- SHARED SETTINGS ---
// Moved here so Main and Menu share the EXACT same variables
extern float hueSpeed;         // Controls the speed of the rainbow cycle
extern float brightnessFloor;  // Controls minimum brightness

// Declare MAX_PARTICLES as a constant integer
extern const int MAX_PARTICLES;

// Declare the particle system globally
extern Particle01 particleSystem;

// --- LIFX / External Process Control ---
extern void* hLifxProcess;

void LaunchLIFX();
void StopLIFX();
void UpdateLIFX(float glowValue, float hueValue);

// Global Mode Names for UI
extern const char* MODE_NAMES[];

#endif // GLOBALS_H