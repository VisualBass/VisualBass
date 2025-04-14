#ifndef PARTICLE01_H
#define PARTICLE01_H

#include "raylib.h"
#include <vector>

// Particle structure to hold particle properties
struct Particle {
    Vector3 position;
    Vector3 velocity;  // The velocity of the particle (random direction and speed)
    Color color;       // The color of the particle
    float life;        // Life of the particle (used for fading)
    float radius;      // Size of the particle
};

class Particle01 {
public:
    Particle01(int maxParticles);  // Constructor to initialize the particle system
    ~Particle01();                // Destructor to clean up particles

    void SpawnParticle(Vector3 position, Color color, float glowValue);  // Spawn a new particle
    void Update(float deltaTime, float glowValue, Color orbColor);  // Update particles based on intensity and color
    void Draw();  // Draw all particles on the screen

private:
    int maxParticles;
    std::vector<Particle> particles;  // Container to hold particles

    // Respawn a particle at a new random position and with random velocity
    void RespawnParticle(Particle* particle);
};

#endif
