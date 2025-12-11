#include "globals.h"
#include "Particle01.h"
#include <iostream>
#include <string>
#include <filesystem> // Requires C++17

// =========================================================
// WINDOWS / RAYLIB CONFLICT FIXES
// =========================================================

// 1. Disable GDI to prevent 'Rectangle' conflict
#define NOGDI
#define WIN32_LEAN_AND_MEAN

// 2. Rename conflicting Windows functions BEFORE including windows.h
#define CloseWindow WinCloseWindow
#define ShowCursor WinShowCursor
#define DrawText WinDrawText
#define LoadImage WinLoadImage
#define PlaySound WinPlaySound

// 3. Include Network Headers BEFORE Windows.h to prevent ordering errors
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <shellapi.h>

// Link the Winsock Library (Required for UDP)
#pragma comment(lib, "ws2_32.lib")

// Namespace alias for cleaner code
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

// Define orbColor globally, initialized to a default color
Color orbColor = WHITE;

// Define MAX_PARTICLES
const int MAX_PARTICLES = PARTICLES;

// Mode Names Definition
const char* MODE_NAMES[] = {
    "Waveform",
    "Gravity Orbs",
    "Cube Field",
    "Particle System 01"
};

// --- LIFX Global Variables ---
void* hLifxProcess = NULL;         // Process Handle
SOCKET lifxSocket = INVALID_SOCKET; // UDP Socket
sockaddr_in lifxDestAddr;          // Target Address
float SETTING_BrightnessFloor = 0.05f; // Adjustable via UI (Min Brightness)

// =========================================================
// LIFX CONTROLLER LOGIC
// =========================================================

void LaunchLIFX() {
    if (hLifxProcess != NULL) return;

    // Smart Path Finding
    fs::path currentPath = fs::current_path();
    fs::path relativePath = fs::path("networking") / "lifx" / "lifx_controller.py";
    fs::path fullScriptPath = currentPath / relativePath;

    // Attempt 1: Check current directory (Release mode)
    if (!fs::exists(fullScriptPath)) {
        // Attempt 2: Check parent directory (Debug/CLion mode)
        fullScriptPath = currentPath.parent_path() / relativePath;
    }

    if (!fs::exists(fullScriptPath)) {
        std::cerr << "[Error] Could not find lifx_controller.py" << std::endl;
        std::cerr << "Searched near: " << currentPath << std::endl;
        return;
    }

    // Prepare paths
    fullScriptPath = fs::absolute(fullScriptPath);
    std::string scriptDir = fullScriptPath.parent_path().string();
    std::string scriptPathQuoted = "\"" + fullScriptPath.string() + "\"";

    SHELLEXECUTEINFOA shExInfo = {0};
    shExInfo.cbSize = sizeof(shExInfo);
    shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shExInfo.hwnd = NULL;
    shExInfo.lpVerb = "open";
    shExInfo.lpFile = "cmd.exe";

    // /k keeps window open. change to /c to auto-close if successful
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

    // Clean up socket
    if (lifxSocket != INVALID_SOCKET) {
        closesocket(lifxSocket);
        lifxSocket = INVALID_SOCKET;
    }
}

// ---------------------------------------------------------
// NETWORK SENDER (UDP)
// ---------------------------------------------------------
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
    if (hLifxProcess == NULL) return; // Don't send if script isn't running
    if (lifxSocket == INVALID_SOCKET) InitLIFXSocket();

    // --- APPLY BRIGHTNESS FLOOR HERE ---
    // This allows the UI to control the minimum light level (0.0 to 1.0)
    float finalBrightness = glowValue;
    if (finalBrightness < SETTING_BrightnessFloor) {
        finalBrightness = SETTING_BrightnessFloor;
    }

    // Format: "Brightness,Hue" (e.g., "0.85,0.12")
    std::string msg = std::to_string(finalBrightness) + "," + std::to_string(hueValue);

    sendto(lifxSocket, msg.c_str(), msg.length(), 0, (sockaddr*)&lifxDestAddr, sizeof(lifxDestAddr));
}