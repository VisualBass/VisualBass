#ifndef CUBE_H
#define CUBE_H

#include "raylib.h"
#include <vector>

// --- SETTINGS STRUCT ---
struct CubeSettings {
    int gridX = 4;          // Width
    int gridY = 4;          // Height
    int gridZ = 4;          // Depth
    float swivelSpeed = 1.0f;
    float spacingIntensity = 1.5f; // How much they spread when bass hits

    // Helper to keep values safe
    void Clamp() {
        if (gridX < 1) gridX = 1; if (gridX > 16) gridX = 16;
        if (gridY < 1) gridY = 1; if (gridY > 16) gridY = 16;
        if (gridZ < 1) gridZ = 1; if (gridZ > 16) gridZ = 16;
        if (swivelSpeed < 0.0f) swivelSpeed = 0.0f;
        if (swivelSpeed > 3.0f) swivelSpeed = 3.0f;
    }
};

struct Cube {
    // Grid Index (where it belongs)
    Vector3 gridIndex;

    // Render State
    Vector3 currentPosition;
    float currentSize;
    float currentRotationAngle;
    Color color;

    // Constructor
    Cube(int x, int y, int z);

    // Update with Settings
    void Update(float totalTime, float glow, float hue, const CubeSettings& settings);

    // Draw
    void Draw() const;
};

// Generator Helper
std::vector<Cube> GenerateCubeField(const CubeSettings& settings);

#endif