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

// NEW: Consolidated shared variables
float hueSpeed = 15.0f;          // Default speed
float brightnessFloor = 0.0f;    // Default floor

Color orbColor = WHITE;
const int MAX_PARTICLES = PARTICLES;

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

void LaunchLIFX() {
    if (hLifxProcess != NULL) return;

    fs::path currentPath = fs::current_path();
    fs::path relativePath = fs::path("networking") / "lifx" / "lifx_controller.py";
    fs::path fullScriptPath = currentPath / relativePath;

    if (!fs::exists(fullScriptPath)) {
        fullScriptPath = currentPath.parent_path() / relativePath;
    }

    if (!fs::exists(fullScriptPath)) {
        std::cerr << "[Error] Could not find lifx_controller.py" << std::endl;
        return;
    }

    fullScriptPath = fs::absolute(fullScriptPath);
    std::string scriptDir = fullScriptPath.parent_path().string();
    std::string scriptPathQuoted = "\"" + fullScriptPath.string() + "\"";

    SHELLEXECUTEINFOA shExInfo = {0};
    shExInfo.cbSize = sizeof(shExInfo);
    shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shExInfo.hwnd = NULL;
    shExInfo.lpVerb = "open";
    shExInfo.lpFile = "cmd.exe";

    std::string params = "/k python " + scriptPathQuoted;

    shExInfo.lpParameters = params.c_str();
    shExInfo.lpDirectory = scriptDir.c_str();
    shExInfo.nShow = SW_SHOW;
    shExInfo.hInstApp = NULL;

    if (ShellExecuteExA(&shExInfo)) {
        hLifxProcess = (void*)shExInfo.hProcess;
        std::cout << "[System] LIFX Controller Launched." << std::endl;
    }
}

void StopLIFX() {
    if (hLifxProcess != NULL) {
        TerminateProcess((HANDLE)hLifxProcess, 0);
        CloseHandle((HANDLE)hLifxProcess);
        hLifxProcess = NULL;
        std::cout << "[System] LIFX Controller Stopped." << std::endl;
    }
    if (lifxSocket != INVALID_SOCKET) {
        closesocket(lifxSocket);
        lifxSocket = INVALID_SOCKET;
    }
}

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