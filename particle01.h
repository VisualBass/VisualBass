#ifndef PARTICLE01_H
#define PARTICLE01_H

#include "raylib.h"
#include <vector>

struct Particle {
    Vector3 position;
    Vector3 velocity;
    Color color;
    float life;
    float size;
    float intensity;
};

class Particle01 {
public:
    Particle01(int maxParticles);
    ~Particle01();

    // --- NEW: CONTROL VARIABLES ---
    // These behave like settings you can change from main.cpp
    float minSpawn = 0.0f;
    float maxSpawn = 10.0f;
    float spawnMultiplier = 1.0f;     // Moved here too
    float forceSensitivity = 1.5f;    // Moved here too

    // --- SIMPLIFIED UPDATE ---
    // We removed the 4 arguments (multiplier, sensitivity, min, max)
    // because they are now part of the class above!
    void Update(float deltaTime, float glowValue, Color orbColor);

    void Draw();

private:
    int maxParticles;
    std::vector<Particle> particles;

    void Spawn3DParticle(Color color, float glowValue);
};

#endif