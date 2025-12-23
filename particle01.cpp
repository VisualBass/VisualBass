#include "particle01.h"
#include "raymath.h"
#include "rlgl.h"
#include "globals.h"
#include <algorithm>
#include <cmath>

Particle01::Particle01(int maxParticles) {
    this->maxParticles = maxParticles;
    particles.reserve(maxParticles);
    this->isInitialized = false;
}

Particle01::~Particle01() {
    particles.clear();
    if (isInitialized) {
        UnloadTexture(this->spriteTex);
    }
}

void Particle01::Init() {
    if (isInitialized) return;
    Image img = GenImageGradientRadial(32, 32, 0.0f, WHITE, BLANK);
    this->spriteTex = LoadTextureFromImage(img);
    UnloadImage(img);
    isInitialized = true;
}

void Particle01::Spawn3DParticle(Color baseColor, float glowValue) {
    if (particles.size() >= (size_t)maxParticles) return;

    Particle p;
    p.position = { 0.0f, 0.0f, 0.0f };
    p.life = 1.0f;

    // Sensitivity linked to globalPump
    float pumpEffect = globalPump * globalPump;
    p.size = 0.4f * (1.0f + (glowValue * pumpEffect));
    p.intensity = glowValue;
    p.color = baseColor;

    float theta = (float)GetRandomValue(0, 360) * DEG2RAD;
    float phi = (float)GetRandomValue(10, 150) * DEG2RAD;

    Vector3 dir = {
        sinf(phi) * cosf(theta),
        cosf(phi),
        sinf(phi) * sinf(theta)
    };

    float speed = 0.5f + (glowValue * 12.0f * pumpEffect);
    p.velocity = Vector3Scale(dir, speed);

    particles.push_back(p);
}

void Particle01::Update(float deltaTime, float glowValue, Color orbColor) {
    if (!isInitialized) return;

    float pumpEffect = globalPump * globalPump;
    float currentSpawnRate = this->minSpawn + (this->maxSpawn - this->minSpawn) * (glowValue * pumpEffect);
    int finalSpawnCount = (int)(currentSpawnRate * this->spawnMultiplier);

    for(int i = 0; i < finalSpawnCount; i++) {
        Spawn3DParticle(orbColor, glowValue);
    }

    for (auto it = particles.begin(); it != particles.end();) {
        // Gravity remains active
        it->velocity.y -= 12.0f * deltaTime;

        // --- COLLISION PHYSICS REMOVED ---
        // Particles now travel along their velocity vectors without hitting barriers.

        it->position = Vector3Add(it->position, Vector3Scale(it->velocity, deltaTime));

        // Basic Floor Bounce
        if (it->position.y <= -5.0f) {
            it->position.y = -5.0f;
            it->velocity.y *= -0.4f;
        }

        it->life -= deltaTime * 0.6f;

        // Sync intensity with current glow for the black-out effect
        it->intensity = glowValue;

        if (it->life <= 0.0f) it = particles.erase(it);
        else ++it;
    }
}

void Particle01::Draw(Camera3D camera) {
    if (!isInitialized) return;

    rlDisableDepthMask();
    BeginBlendMode(BLEND_ADDITIVE);

    for (const auto& p : particles) {
        // Apply intensity scaling (Particles become black when glow is 0)
        Color drawColor = p.color;
        drawColor.r = (unsigned char)(p.color.r * p.intensity);
        drawColor.g = (unsigned char)(p.color.g * p.intensity);
        drawColor.b = (unsigned char)(p.color.b * p.intensity);
        drawColor.a = (unsigned char)(255.0f * p.life);

        DrawBillboard(camera, this->spriteTex, p.position, p.size, drawColor);
    }

    EndBlendMode();
    rlEnableDepthMask();
}