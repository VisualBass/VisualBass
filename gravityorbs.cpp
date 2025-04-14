#include "gravityorbs.h"
#include "raylib.h"  // For Vector2, Color, and drawing functions
#include "raymath.h" // For Vector2 math functions
#include <stdlib.h>   // For rand() and srand()
#include <math.h>     // For mathematical functions like powf

// Constants for orb behavior
#define GRAVITY_RAMP        18.0f
#define CENTER_SUCK_RADIUS  40.0f
#define MOUSE_REPEL_RADIUS  250.0f
#define MOUSE_REPEL_FORCE   5.0f
#define ORB_RESPAWN_MARGIN  40.0f
#define ORB_GLOW_RAMP       1.0f
#define ORB_MAX_SIZE        20.0f
#define ORB_MIN_SIZE        5.0f
#define ORB_RESPAWN         true // Ensure this is defined

// Global variables (defined here)
Orb* orbs = nullptr;  // Pointer to an array of orbs
int orbCount = 0;     // Number of orbs

// Respawn an orb at a random position on screen
void RespawnOrb(Orb *orb) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int side = GetRandomValue(0, 3);
    float x, y;

    switch (side) {
        case 0: x = GetRandomValue(-ORB_RESPAWN_MARGIN, screenWidth + ORB_RESPAWN_MARGIN); y = -ORB_RESPAWN_MARGIN; break;
        case 1: x = GetRandomValue(-ORB_RESPAWN_MARGIN, screenWidth + ORB_RESPAWN_MARGIN); y = screenHeight + ORB_RESPAWN_MARGIN; break;
        case 2: x = -ORB_RESPAWN_MARGIN; y = GetRandomValue(-ORB_RESPAWN_MARGIN, screenHeight + ORB_RESPAWN_MARGIN); break;
        case 3: x = screenWidth + ORB_RESPAWN_MARGIN; y = GetRandomValue(-ORB_RESPAWN_MARGIN, screenHeight + ORB_RESPAWN_MARGIN); break;
    }

    orb->pos = (Vector2){ x, y };
    orb->radius = ORB_MIN_SIZE;
    orb->opacity = 255;
    orb->color = WHITE;
}

// Initialize all orbs
void InitOrbs(int maxOrbs) {
    orbCount = maxOrbs;
    orbs = (Orb*)malloc(sizeof(Orb) * orbCount);

    for (int i = 0; i < orbCount; i++) {
        RespawnOrb(&orbs[i]);
    }
}

// Update orb positions and properties based on intensity
void UpdateOrbs(float intensity, bool escape_mode, Color orbColor) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    Vector2 center = { screenWidth / 2.0f, screenHeight / 2.0f };

    // Get mouse position
    Vector2 mouse = GetMousePosition();
    bool mouseActive = IsCursorOnScreen();  // Check if mouse is within the window bounds

    for (int i = 0; i < orbCount; i++) {
        Orb *orb = &orbs[i];

        // Calculate the direction from the orb to the center of the screen
        Vector2 dir = Vector2Subtract(center, orb->pos);
        float dist = Vector2Length(dir);
        if (dist == 0) dist = 1;
        dir = Vector2Scale(dir, 1.0f / dist); // Normalize the direction

        // Apply gravitational pull towards the center
        float pullStrength = GRAVITY_RAMP * intensity;
        orb->pos = Vector2Add(orb->pos, Vector2Scale(dir, pullStrength));

        // Mouse repulsion effect
        if (mouseActive) {
            float mouseDist = Vector2Distance(orb->pos, mouse);
            if (mouseDist < MOUSE_REPEL_RADIUS) {
                // Calculate the direction from the orb to the mouse
                Vector2 repelDir = Vector2Subtract(orb->pos, mouse); // Orb should repel away from the mouse
                float repelStrength = (1.0f - (mouseDist / MOUSE_REPEL_RADIUS)) * MOUSE_REPEL_FORCE;
                orb->pos = Vector2Add(orb->pos, Vector2Scale(Vector2Normalize(repelDir), repelStrength));
            }
        }

        // If the orb is within the sucking radius, respawn it
        if (Vector2Distance(orb->pos, center) < CENTER_SUCK_RADIUS) {
            if (ORB_RESPAWN) RespawnOrb(orb);  // Respawn orb if it gets close to center
            else orb->opacity = orb->radius = 0;  // Hide the orb if not respawning
        }

        // Update orb size based on intensity
        orb->radius = ORB_MIN_SIZE + intensity * ORB_MAX_SIZE;
        orb->opacity = (int)(intensity * 255);
        orb->color = orbColor;
    }
}


// Draw all orbs on the screen
void DrawOrbs() {
    for (int i = 0; i < orbCount; i++) {
        Orb orb = orbs[i];
        if (orb.opacity > 0 && orb.radius > 0.5f) {
            Color c = { orb.color.r, orb.color.g, orb.color.b, (unsigned char)orb.opacity };
            DrawCircleV(orb.pos, orb.radius, c);
        }
    }
}
