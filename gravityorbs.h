// gravityorbs.h
#ifndef GRAVITYORBS_H
#define GRAVITYORBS_H

#include "raylib.h"  // For Vector2, Color, and drawing functions

// Orb structure definition
typedef struct {
    Vector2 pos;
    float radius;
    int opacity;
    Color color;
} Orb;

// Function declarations
void RespawnOrb(Orb *orb);
void InitOrbs(int maxOrbs);
void UpdateOrbs(float intensity, bool escape_mode, Color orbColor);
void DrawOrbs();

// Extern declarations for global variables
extern Orb* orbs;     // Pointer to an array of orbs
extern int orbCount;  // Number of orbs

// --- NEW EDITABLE SETTINGS ---
extern float gravityStrength; // Was GRAVITY_RAMP
extern float orbMaxSize;      // Was ORB_MAX_SIZE
extern float mouseRepelForce; // Was MOUSE_REPEL_FORCE

#endif // GRAVITYORBS_H