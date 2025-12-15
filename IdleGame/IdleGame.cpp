#include "idlegame.h"
#include <fstream>
#include <iostream>
#include <cstdio>
#include <cmath>
#include <filesystem> // C++17 feature for folder creation
#include <cstdlib>    // For getting environment variables

namespace fs = std::filesystem;

// 100 ticks per minute = 100 / 60 seconds = 1.66 ticks/sec
// Interval = 60 / 100 = 0.6 seconds
#define TICK_INTERVAL 0.6f

// --- PATH HELPER ---
// This function determines the path: C:\Users\YourName\Games\VisualBassSync\idlegame_save.dat
std::string GetSaveFilePath() {
    // 1. Get User's Home Directory (e.g., C:\Users\David)
    const char* userProfile = std::getenv("USERPROFILE");

    if (userProfile) {
        fs::path saveDir(userProfile);

        // 2. Append the requested path
        saveDir /= "Games";
        saveDir /= "VisualBassSync";

        // 3. Create directories if they don't exist
        try {
            if (!fs::exists(saveDir)) {
                fs::create_directories(saveDir);
                // Optional: Comment out logging to avoid console spam every tick if you move logic here
                // std::cout << "Created directory: " << saveDir << std::endl;
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error creating directory: " << e.what() << std::endl;
            return "idlegame_save.dat"; // Fallback to local folder on error
        }

        // 4. Return full file path
        return (saveDir / "idlegame_save.dat").string();
    }

    // Fallback if we can't find the user profile
    return "idlegame_save.dat";
}

IdleGame::IdleGame()
    : totalXP(0.0),
      tickTimer(0.0f),
      accumulatorInterval(TICK_INTERVAL),
      accumulatedGlow(0.0f)
{
    LoadGame();
}

IdleGame::~IdleGame() {
    SaveGame();
}

void IdleGame::Update(float dt, float currentGlowValue) {
    // 1. Accumulate Glow Value
    accumulatedGlow += currentGlowValue * dt;

    // 2. Handle Tick Timer
    tickTimer += dt;
    if (tickTimer >= accumulatorInterval) {
        TriggerTick();
        tickTimer -= accumulatorInterval;
    }

    // 3. Update Floating Texts
    for (size_t i = 0; i < floatingTexts.size(); ) {
        floatingTexts[i].life -= dt;
        floatingTexts[i].position.y -= floatingTexts[i].velocityY * dt;

        if (floatingTexts[i].life <= 0.0f) {
            floatingTexts.erase(floatingTexts.begin() + i);
        } else {
            ++i;
        }
    }
}

void IdleGame::TriggerTick() {
    if (accumulatedGlow > 0.01f) {
        // XP Formula
        float gainedXP = 10.0f * (accumulatedGlow * 2.0f);
        int displayXP = (int)std::ceil(gainedXP);

        if (displayXP > 0) {
            totalXP += displayXP;

            // Spawn Text
            char buffer[32];
            std::sprintf(buffer, "+%d XP", displayXP);

            Color txtColor = GREEN;
            if (displayXP > 50) txtColor = YELLOW;
            if (displayXP > 100) txtColor = ORANGE;

            SpawnFloatingText(std::string(buffer), txtColor);

            // --- AUTO SAVE EVERY TICK ---
            SaveGame();
        }
    }
    accumulatedGlow = 0.0f;
}

void IdleGame::SpawnFloatingText(const std::string& text, Color color) {
    FloatingText ft;
    ft.position = { 0, 0 }; // Position set by Menu Draw
    ft.text = text;
    ft.color = color;
    ft.life = 2.0f;
    ft.velocityY = 50.0f;

    floatingTexts.push_back(ft);
}

// --- SAVING & LOADING ---

void IdleGame::SaveGame() {
    std::string path = GetSaveFilePath();
    std::ofstream file(path);

    if (file.is_open()) {
        file << "xp " << totalXP << "\n";
        file.close();
        // Commented out to prevent console spam every 0.6 seconds
        // std::cout << "Game Saved" << std::endl;
    } else {
        std::cerr << "Failed to save game to: " << path << std::endl;
    }
}

void IdleGame::LoadGame() {
    std::string path = GetSaveFilePath();
    std::ifstream file(path);

    if (file.is_open()) {
        std::string key;
        while (file >> key) {
            if (key == "xp") {
                file >> totalXP;
            }
        }
        file.close();
        std::cout << "Game Loaded from: " << path << std::endl;
    }
}