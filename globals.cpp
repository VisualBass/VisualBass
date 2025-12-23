#include "globals.h"
#include "Particle01.h"
#include <iostream>
#include <string>
#include <filesystem>

// =========================================================
// WINDOWS / RAYLIB CONFLICT FIXES
// =========================================================
#define NOGDI
#define WIN32_LEAN_AND_MEAN
#define CloseWindow WinCloseWindow
#define ShowCursor WinShowCursor
#define DrawText WinDrawText
#define LoadImage WinLoadImage
#define PlaySound WinPlaySound

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <shellapi.h>

#pragma comment(lib, "ws2_32.lib")

namespace fs = std::filesystem;

// =========================================================
// GLOBAL VARIABLES
// =========================================================

// Audio & Visual Globals
volatile int audioDataReady = 0;
float gAudioBuffer[FRAMES_PER_BUFFER];
float glow_value = 0.0f;
bool escape_mode = false;
float hueShift = 0.0f;
bool autoCycleHue = true;
int numOrbs = 400;

// --- NEW SETTINGS ---
bool enableInterpolation = true;
CubeSettings cubeSettings; // Uses default constructor

float globalPump = 1.0f;

// NEW: Consolidated shared variables
float hueSpeed = 15.0f;          // Default speed
float brightnessFloor = 0.0f;    // Default floor

Color orbColor = WHITE;
const int MAX_PARTICLES = DEFAULT_MAX_PARTICLES;

const char* MODE_NAMES[] = {
    "Waveform",
    "Gravity Orbs",
    "Cube Field",
    "Particle System 01"
};

// --- LIFX Global Variables ---
void* hLifxProcess = NULL;
SOCKET lifxSocket = INVALID_SOCKET;
sockaddr_in lifxDestAddr;

// =========================================================
// LIFX CONTROLLER LOGIC
// =========================================================

void InitLIFXSocket() {
    if (lifxSocket != INVALID_SOCKET) return;
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    lifxSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    lifxDestAddr.sin_family = AF_INET;
    lifxDestAddr.sin_port = htons(7777);
    inet_pton(AF_INET, "127.0.0.1", &lifxDestAddr.sin_addr);
}

void UpdateLIFX(float glowValue, float hueValue) {
    if (hLifxProcess == NULL) return;
    if (lifxSocket == INVALID_SOCKET) InitLIFXSocket();

    // Use the SHARED brightnessFloor variable now
    float finalBrightness = glowValue;
    if (finalBrightness < brightnessFloor) {
        finalBrightness = brightnessFloor;
    }

    std::string msg = std::to_string(finalBrightness) + "," + std::to_string(hueValue);
    sendto(lifxSocket, msg.c_str(), msg.length(), 0, (sockaddr*)&lifxDestAddr, sizeof(lifxDestAddr));
}