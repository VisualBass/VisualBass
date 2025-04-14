#include "particle01.h"
#include "raymath.h"  // For Vector3Add, Vector3Scale
#include "raylib.h"   // For drawing functions
#include <algorithm>

const float BASE_SPAWN_RATE = 0.2f;  // Base spawn rate (before applying glowValue)
const float GLOW_SPAWN_RAMP = 4.0f;  // Ramp-up factor for spawn rate based on glowValue
const float BASE_RADIUS = 25.0f;      // Base radius of particles (even without glowValue)
const float MAX_RADIUS = 1250.0f;      // Max radius of particles
const float MIN_RADIUS = 0.0001f;      // Minimum radius of particles, below which they will be destroyed
const float MIN_GLOW_THRESHOLD = 0.0025f;  // Minimum glow threshold below which particles won't spawn
const float SPAWN_BIAS = 0.5f;        // Bias towards center (0.0 to 1.0), higher value = closer to the center
const float SPAWN_RADIUS = 10.0f;    // Radius around center where particles will spawn
const bool FORCE_RESPAWN_IN_CENTER = true;  // Whether to always respawn in the center (true/false)
const float ZOOM_FACTOR = 2.0f;      // For zoom effect to adjust SPAWN_RADIUS

// Constructor to initialize the particle system
Particle01::Particle01(int maxParticles) {
    this->maxParticles = maxParticles;
    particles.reserve(maxParticles);  // Pre-allocate space for particles
}

// Destructor to clean up memory
Particle01::~Particle01() {
    particles.clear();  // Clear all particles
}

// Method to spawn a particle at a given position with color and velocity based on glowValue
void Particle01::SpawnParticle(Vector3 position, Color color, float glowValue) {
    if (particles.size() >= maxParticles) {
        particles.erase(particles.begin());  // Remove the oldest particle
    }

    // Initialize particle with a random position
    particles.push_back({ position, Vector3{0.0f, 0.0f, 0.0f}, color, 1.0f, BASE_RADIUS });
}

// Method to update all particles (spawn, move, fade)
void Particle01::Update(float deltaTime, float glowValue, Color orbColor) {
    // If glowValue is very low, clear the particles (or allow a small amount of particles to spawn)
    if (glowValue <= MIN_GLOW_THRESHOLD) {
        // Allow some particles to stay for a fading effect
        for (auto& particle : particles) {
            // Make particles fade slower when glowValue is low
            particle.life -= deltaTime * 0.1f;  // Fade them out slower as glowValue decreases
        }
        particles.erase(
            std::remove_if(particles.begin(), particles.end(),
                [](const Particle& p) { return p.life <= 0; }),  // Remove dead particles
            particles.end()
        );
        return;  // Skip further updates to particles
    }

    // Calculate dynamic spawn rate based on glowValue (bass strength)
    float spawnRate = BASE_SPAWN_RATE + glowValue * GLOW_SPAWN_RAMP;  // Ramp up the spawn rate as glowValue increases

    // Dynamic SPAWN_RADIUS based on screen size, zoomed in
    float screenWidth = GetScreenWidth();
    float screenHeight = GetScreenHeight();
    float dynamicSpawnRadius = SPAWN_RADIUS * std::min(screenWidth, screenHeight) / 800.0f; // Adjust according to screen size
    dynamicSpawnRadius /= ZOOM_FACTOR; // Apply zoom effect for more controlled spawn

    // Spawn a new particle based on audio (glowValue) every frame
    if (GetRandomValue(0, 1000) < spawnRate * 1000) {
        // Spawn near the center based on the FORCE_RESPAWN_IN_CENTER flag
        if (FORCE_RESPAWN_IN_CENTER) {
            // Calculate the center of the screen (bias)
            float centerX = screenWidth * 0.5f;
            float centerY = screenHeight * 0.5f;

            // Apply a slight random spread, based on the glowValue, near the center
            float spreadX = GetRandomValue(-dynamicSpawnRadius, dynamicSpawnRadius);  // Range will be [-dynamicSpawnRadius, dynamicSpawnRadius]
            float spreadY = GetRandomValue(-dynamicSpawnRadius, dynamicSpawnRadius);  // Same for Y axis

            // Calculate random spawn position near the center
            Vector2 randomPosition = {
                centerX + spreadX,  // Apply bias from the center with random spread
                centerY + spreadY   // Same for Y axis
            };

            // Spawn particle at the random position near the center
            SpawnParticle((Vector3){randomPosition.x, randomPosition.y, 0.0f}, orbColor, glowValue);
        } else {
            // When NOT forcing respawn in center, randomize the spawn position on screen with bias
            float biasX = GetRandomValue(0, 100) < (SPAWN_BIAS * 100) ? 1.0f : 0.0f;  // Apply bias towards center with SPAWN_BIAS
            float biasY = GetRandomValue(0, 100) < (SPAWN_BIAS * 100) ? 1.0f : 0.0f;  // Apply bias towards center with SPAWN_BIAS

            // Adjust X and Y to fit within the screen, applying bias
            float spawnX = GetRandomValue(0, screenWidth) * (1 - biasX) + screenWidth * 0.5f * biasX;
            float spawnY = GetRandomValue(0, screenHeight) * (1 - biasY) + screenHeight * 0.5f * biasY;

            // Spawn particle at the calculated position
            SpawnParticle((Vector3){spawnX, spawnY, 0.0f}, orbColor, glowValue);
        }
    }

    // Update existing particles with explosive effect
    Vector3 center = { screenWidth * 0.5f, screenHeight * 0.5f, 0.0f };  // Center of the screen for outward movement
    for (auto it = particles.begin(); it != particles.end();) {
        auto& particle = *it;

        // Particle fade-out control: dynamic fading based on glowValue and life
        float fadeSpeed = glowValue < MIN_GLOW_THRESHOLD ? 0.2f : (glowValue > 0.5f ? 0.5f : 0.3f);  // Smooth fade at higher glow values
        particle.life -= deltaTime * fadeSpeed;

        // Apply fade effect (transparency based on life)
        particle.color.a = (unsigned char)(particle.life * 255);  // Fade opacity over time

        // Explosive outward motion (calculate direction away from center)
        Vector3 direction = Vector3Subtract(particle.position, center); // Direction from the center
        float distance = Vector3Length(direction); // Calculate the distance from the center

        if (distance > 0) {
            direction = Vector3Normalize(direction); // Normalize direction
        }

        // Apply explosive movement based on glow value
        float speed = glowValue * 500.0f; // Speed based on glowValue
        particle.velocity = Vector3Scale(direction, speed * deltaTime);

        // Apply the velocity to the particle position to make it burst outwards
        particle.position = Vector3Add(particle.position, particle.velocity);

        // Handle particles going off-screen and wrap them around
        if (particle.position.x < 0) particle.position.x = screenWidth;
        if (particle.position.x > screenWidth) particle.position.x = 0;
        if (particle.position.y < 0) particle.position.y = screenHeight;
        if (particle.position.y > screenHeight) particle.position.y = 0;

        // If particle is dead (life <= 0), remove it
        if (particle.life <= 0) {
            it = particles.erase(it);  // Remove the particle
        } else {
            ++it;  // Move to the next particle if it wasn't erased
        }
    }
}


// Method to draw all particles in the system
void Particle01::Draw() {
    for (const auto& particle : particles) {
        if (particle.life > 0) {
            Color color = particle.color;
            color.a = (unsigned char)(particle.color.a * particle.life);  // Adjust alpha for fading
            DrawSphere(particle.position, particle.radius, color);  // Draw particle as a sphere
        }
    }
}

// Method to respawn a particle at a new location
void Particle01::RespawnParticle(Particle* particle) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // Ensure the particle is completely inside the screen by adjusting for its radius
    float maxX = screenWidth - particle->radius;
    float maxY = screenHeight - particle->radius;

    // Respawn based on the FORCE_RESPAWN_IN_CENTER flag
    if (FORCE_RESPAWN_IN_CENTER) {
        // Calculate the center of the screen (bias)
        float centerX = GetScreenWidth() * 0.5f;
        float centerY = GetScreenHeight() * 0.5f;

        // Apply a slight random spread, based on the glowValue, near the center
        float spreadX = GetRandomValue(-SPAWN_RADIUS, SPAWN_RADIUS);  // Range will be [-SPAWN_RADIUS, SPAWN_RADIUS]
        float spreadY = GetRandomValue(-SPAWN_RADIUS, SPAWN_RADIUS);  // Same for Y axis

        // Calculate random spawn position near the center
        particle->position.x = centerX + spreadX;  // Apply bias from the center with random spread
        particle->position.y = centerY + spreadY;  // Same for Y axis
    } else {
        // When NOT forcing respawn in center, randomize the spawn position on screen
        particle->position.x = (float)GetRandomValue(0, static_cast<int>(maxX));
        particle->position.y = (float)GetRandomValue(0, static_cast<int>(maxY));
    }

    particle->position.z = 0.0f;  // Set Z to 0.0f since it's 2D

    // Assign random velocity based on glow value
    float randomAngle = GetRandomValue(0, 360);
    float speed = 2.0f + (particle->life * 0.5f);  // Default speed + scaling based on life (or audio)

    particle->velocity = (Vector3){
        speed * cosf(randomAngle * DEG2RAD),  // X velocity
        speed * sinf(randomAngle * DEG2RAD),  // Y velocity
        0.0f                                // Z velocity is 0 in 2D
    };

    particle->life = 1.0f; // Reset life to full
    particle->radius = BASE_RADIUS; // Set radius to the desired size (base size)
}

// same particale, it's using one stored in memory when it should be using more than that at least.