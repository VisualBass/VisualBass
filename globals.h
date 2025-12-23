#ifndef GLOBALS_H
#define GLOBALS_H

#include "raylib.h"
#include "cube.h"

#define FRAMES_PER_BUFFER 512
#define DEFAULT_MAX_PARTICLES 1250

extern float gAudioBuffer[FRAMES_PER_BUFFER];
extern volatile int audioDataReady;

extern float glow_value;
extern float hueShift;
extern float hueSpeed;
extern bool autoCycleHue;
extern float brightnessFloor;
extern bool escape_mode;

extern bool enableInterpolation;
extern CubeSettings cubeSettings;
extern const int MAX_PARTICLES;

// --- NEW GLOBAL SETTING ---
extern float globalPump;
// --------------------------

#endif