#ifndef CUBE_H
#define CUBE_H

#include "raylib.h"
#include <vector>

class Cube {
public:
    Cube(Vector3 pos, float size, Color color, float rotationSpeed = 1.0f);

    void Update(float deltaTime, float glow, float hue);
    void Draw() const;

private:
    Vector3 position;        // World-space position of cube
    float size;              // Current scaled size
    float baseSize;          // Original size (before glow scaling)
    Color color;             // Cube's color
    float rotation;          // Current rotation angle (radians)
    float rotationSpeed;     // Rotation speed (radians/sec)
    Vector3 rotationAxis;    // Unique rotation axis per cube
};

// Generates a grid of cubes centered around origin
std::vector<Cube> GenerateCubeField(int gridRadius = 5, float spacing = 2.5f);

#endif // CUBE_H
