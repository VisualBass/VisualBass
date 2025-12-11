#ifndef GLOBALS_H
#define GLOBALS_H

#include "Particle01.h"
#include "raylib.h" // Required for Color struct

// NOTE: We do NOT include <windows.h> here to avoid conflicts with Raylib.

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
extern Color orbColor; // Global Color Picker value

// --- LIFX SETTINGS ---
// Controlled by UI, used by networking.cpp
extern float brightnessFloor;

// Declare MAX_PARTICLES as a constant integer
extern const int MAX_PARTICLES;

// Declare the particle system globally
extern Particle01 particleSystem;

// --- LIFX / External Process Control ---
// FIX: Use 'void*' instead of 'HANDLE' to avoid heavy Windows headers here
extern void* hLifxProcess;

void LaunchLIFX();
void StopLIFX();
void UpdateLIFX(float glowValue, float hueValue); // Sends data to Python

// Global Mode Names for UI
extern const char* MODE_NAMES[];

#endif // GLOBALS_H