#ifndef GLOBALS_H
#define GLOBALS_H

// Define FRAMES_PER_BUFFER
#define FRAMES_PER_BUFFER   2048

// Global variables for audio processing and mode control
extern volatile int audioDataReady;
extern float gAudioBuffer[FRAMES_PER_BUFFER];
extern float glow_value;
extern bool escape_mode;
extern float hueShift;
extern bool autoCycleHue;

// Add other global variables here...

#endif // GLOBALS_H
