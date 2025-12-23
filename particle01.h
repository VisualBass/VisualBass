#ifndef PARTICLE01_H
#define PARTICLE01_H

#include "raylib.h"
#include <vector>

struct Particle {
    Vector3 position;
    Vector3 velocity;
    float life;
    float size;
    float intensity;
    Color color;
};

class Particle01 {
public:
    Particle01(int maxParticles);
    ~Particle01();

    void Init();
    void Update(float deltaTime, float glowValue, Color orbColor);
    void Draw(Camera3D camera);

private:
    void Spawn3DParticle(Color baseColor, float glowValue);

    std::vector<Particle> particles;
    int maxParticles;
    Texture2D spriteTex;
    bool isInitialized = false;

    float minSpawn = 1.0f;
    float maxSpawn = 15.0f;
    float spawnMultiplier = 2.0f;
};

#endif