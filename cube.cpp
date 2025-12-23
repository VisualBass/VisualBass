#include "cube.h"
#include "raymath.h"
#include "rlgl.h"
#include <cmath>

Cube::Cube(int x, int y, int z)
    : gridIndex(Vector3{ (float)x, (float)y, (float)z }),
      currentPosition(Vector3Zero()),
      currentSize(1.0f),
      currentRotationAngle(0.0f),
      color(WHITE)
{
}

void Cube::Update(float totalTime, float glow, float hue, const CubeSettings& settings)
{
    float baseSize = 1.0f;

    float offX = (settings.gridX - 1) * 0.5f;
    float offY = (settings.gridY - 1) * 0.5f;
    float offZ = (settings.gridZ - 1) * 0.5f;

    // OLD PUMP LOGIC: Spacing is a direct multiplier of intensity and glow
    float currentSpacing = 1.0f + (glow * settings.spacingIntensity);

    float lx = (gridIndex.x - offX) * currentSpacing;
    float ly = (gridIndex.y - offY) * currentSpacing;
    float lz = (gridIndex.z - offZ) * currentSpacing;

    Vector3 localPos = { lx, ly, lz };

    float swivelAngleRad = sinf(totalTime * settings.swivelSpeed) * (PI / 4.0f);
    currentRotationAngle = swivelAngleRad * RAD2DEG;

    currentPosition = Vector3RotateByAxisAngle(localPos, Vector3{ 0.0f, 1.0f, 0.0f }, swivelAngleRad);

    color = ColorFromHSV(hue * 360.0f, 0.8f, 0.9f);
    currentSize = baseSize;
}

void Cube::Draw() const {
    rlPushMatrix();
        rlTranslatef(currentPosition.x, currentPosition.y, currentPosition.z);
        rlRotatef(currentRotationAngle, 0.0f, 1.0f, 0.0f);
        DrawCube(Vector3Zero(), currentSize, currentSize, currentSize, color);
        DrawCubeWires(Vector3Zero(), currentSize, currentSize, currentSize, Fade(BLACK, 0.5f));
    rlPopMatrix();
}

std::vector<Cube> GenerateCubeField(const CubeSettings& settings) {
    std::vector<Cube> field;
    field.reserve(settings.gridX * settings.gridY * settings.gridZ);
    for (int y = 0; y < settings.gridY; ++y) {
        for (int z = 0; z < settings.gridZ; ++z) {
            for (int x = 0; x < settings.gridX; ++x) {
                field.emplace_back(x, y, z);
            }
        }
    }
    return field;
}