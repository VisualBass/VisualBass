#include "particle01.h"
#include "raymath.h"
#include <algorithm>
#include <cmath>

// --- SETTINGS (CONSTANTS) ---
// Keep gravity/physics here because you likely won't change these often
const float GRAVITY = 12.0f;
const float BASE_SPEED = 0.5f;
const float MAX_SPEED_FACTOR = 7.0f;
const float PARTICLE_SIZE = 0.15f;
const float DECAY_RATE = 0.5f;
const float FLOOR_LEVEL = -5.0f;
const float BOUNCE_DAMPING = -0.5f;
const float SPEAKER_KICK = 8.0f;

Particle01::Particle01(int maxParticles) {
    this->maxParticles = maxParticles;
    particles.reserve(maxParticles);
}

Particle01::~Particle01() {
    particles.clear();
}

// Removed 'forceSensitivity' arg; using class variable instead
void Particle01::Spawn3DParticle(Color baseColor, float glowValue) {
    if (particles.size() >= maxParticles) return;

    Particle p;
    p.position = { 0.0f, 0.0f, 0.0f };
    p.life = 1.0f;
    p.size = PARTICLE_SIZE + (glowValue * 0.15f);
    p.intensity = glowValue;
    p.color = baseColor;

    float theta = GetRandomValue(0, 360) * DEG2RAD;
    float phi = GetRandomValue(10, 120) * DEG2RAD;

    Vector3 dir;
    dir.x = sinf(phi) * cosf(theta);
    dir.y = cosf(phi);
    dir.z = sinf(phi) * sinf(theta);

    // USE CLASS VARIABLE HERE
    float effSensitivity = this->forceSensitivity * this->forceSensitivity;

    float speed = BASE_SPEED + (glowValue * MAX_SPEED_FACTOR * effSensitivity);
    p.velocity = Vector3Scale(dir, speed);

    particles.push_back(p);
}

// SIMPLIFIED UPDATE
void Particle01::Update(float deltaTime, float glowValue, Color orbColor) {

    // 1. DYNAMIC SPAWN LOGIC
    // USE CLASS VARIABLES HERE (minSpawn, maxSpawn, spawnMultiplier)
    float currentSpawnRate = this->minSpawn + (this->maxSpawn - this->minSpawn) * glowValue;
    int finalSpawnCount = (int)(currentSpawnRate * this->spawnMultiplier);

    for(int i=0; i < finalSpawnCount; i++) {
        Spawn3DParticle(orbColor, glowValue);
    }

    float effSensitivity = this->forceSensitivity * this->forceSensitivity;

    // 2. PARTICLE LOOP
    for (auto it = particles.begin(); it != particles.end();) {
        it->intensity = fminf(it->intensity, glowValue);
        it->velocity.y -= GRAVITY * deltaTime;

        if(glowValue > 0.2f) {
            float noise = glowValue * 0.5f * effSensitivity;
            it->velocity.x += (GetRandomValue(-10, 10) / 20.0f) * noise;
            it->velocity.z += (GetRandomValue(-10, 10) / 20.0f) * noise;
        }

        it->position = Vector3Add(it->position, Vector3Scale(it->velocity, deltaTime));

        if (it->position.y <= FLOOR_LEVEL) {
            it->position.y = FLOOR_LEVEL;
            it->velocity.y *= BOUNCE_DAMPING;

            if (glowValue > 0.05f) {
                float kick = glowValue * SPEAKER_KICK * effSensitivity;
                it->velocity.y += kick;
                float spread = kick * 0.3f;
                it->velocity.x += (GetRandomValue(-10, 10) / 10.0f) * spread;
                it->velocity.z += (GetRandomValue(-10, 10) / 10.0f) * spread;
            }
        }

        it->life -= deltaTime * DECAY_RATE;

        if (it->life <= 0.0f) {
            it = particles.erase(it);
        } else {
            ++it;
        }
    }
}

void Particle01::Draw() {
    for (const auto& p : particles) {
        Color drawColor = p.color;
        float finalBrightness = p.intensity;
        drawColor.r = (unsigned char)(p.color.r * finalBrightness);
        drawColor.g = (unsigned char)(p.color.g * finalBrightness);
        drawColor.b = (unsigned char)(p.color.b * finalBrightness);
        drawColor.a = (unsigned char)(255.0f * p.life);
        DrawCylinder(p.position, p.size, p.size, p.size, 5, drawColor);
    }
}