#ifndef IDLEGAME_H
#define IDLEGAME_H

#include "raylib.h"
#include <vector>
#include <string>

// Simple structure for the floating text effects
struct FloatingText {
    Vector2 position;
    std::string text;
    Color color;
    float life;      // 1.0 to 0.0
    float velocityY; // Speed moving up
};

class IdleGame {
public:
    IdleGame();
    ~IdleGame();

    // Core Loop
    void Update(float dt, float currentGlowValue);

    // Data Access
    double GetTotalXP() const { return totalXP; }
    const std::vector<FloatingText>& GetFloatingTexts() const { return floatingTexts; }

    // Persistence
    void SaveGame();
    void LoadGame();

private:
    // XP / Resource Data
    double totalXP;
    
    // Logic Timers
    float tickTimer;
    float accumulatorInterval; // Time in seconds for one "tick"
    float accumulatedGlow;     // Sum of glow over the interval

    // Visuals
    std::vector<FloatingText> floatingTexts;

    // Helper methods
    void TriggerTick();
    void SpawnFloatingText(const std::string& text, Color color);
};

#endif