#include "cube.h"
#include "raymath.h"
#include <cmath>

// Constructor
Cube::Cube(Vector3 pos, float size, Color color, float rotationSpeed)
    : position(pos),
      size(size),
      baseSize(size),
      color(color),
      rotation(0.0f),
      rotationSpeed(rotationSpeed)
{
}

void Cube::Update(float deltaTime, float glow, float hue) {
    rotation += rotationSpeed * deltaTime;

    // Resize based on glow (bass intensity)
    size = baseSize + glow * 1.00f;

    // Rotate position around center (Y-axis)
    float angle = deltaTime * rotationSpeed;
    float cosA = cosf(angle);
    float sinA = sinf(angle);

    float x = position.x;
    float z = position.z;

    // Rotate around Y axis (XZ plane)
    position.x = x * cosA - z * sinA;
    position.z = x * sinA + z * cosA;

    // Update color based on hue
    float s = 1.0f, v = 1.0f;
    float h = hue;
    float r, g, b;
    int i = int(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);
    switch (i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }
    color = Color{ (unsigned char)(r * 255), (unsigned char)(g * 255), (unsigned char)(b * 255), 255 };
}

// Draw cube
void Cube::Draw() const {
    DrawCube(position, size, size, size, color);
    DrawCubeWires(position, size, size, size, Fade(WHITE, 0.4f));
}

// Generate a field of cubes
std::vector<Cube> GenerateCubeField(int gridRadius, float spacing) {
    std::vector<Cube> cubeField;
    for (int x = -gridRadius; x <= gridRadius; ++x) {
        for (int z = -gridRadius; z <= gridRadius; ++z) {
            cubeField.emplace_back(
                Vector3{ x * spacing, 1.0f, z * spacing },
                1.0f,
                WHITE,
                1.0f
            );
        }
    }
    return cubeField;
}
