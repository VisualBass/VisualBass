#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

// Prevent Windows macros that conflict with raylib
#define NOGDI           // Avoid 'Rectangle' macro
#define NOUSER          // Avoid 'ShowCursor' & 'CloseWindow'
#define NOWINRES        // Avoid 'LoadImage'

// Constant for version control
const char* VERSION = "0.01b";  // Update this value to change the version

#include <windows.h>     // Safe now with above defines
#include <winsock2.h>
#include <ws2tcpip.h>    // (optional, but future-safe)
#include "raylib.h"
#include "raymath.h"
#include <portaudio.h>
#include "kiss_fft.h"
#include <math.h>
#include <stdio.h>
#include <chrono>
#include "gravityorbs.h"
#include "Waveform.h"
#include "networking.h"
#include <vector>
#include "globals.h" // Include the header file where globals are declared
#include "menu.h"
#include "cube.h"

// Constants
#define SAMPLE_RATE         44100
#define FFT_SIZE            FRAMES_PER_BUFFER
#define BASS_FREQ_THRESHOLD 80.0f
#define BASS_NORM_FACTOR    225.0f
#define BASS_BOOST_EXPONENT 3.5f

#define MAX_ORBS            1250
#define GRAVITY_RAMP        32.0f
#define CENTER_SUCK_RADIUS  30.0f
#define ORB_RESPAWN         true
#define SPAWN_MARGIN        40.0f
#define MOUSE_REPEL_RADIUS  200.0f
#define MOUSE_REPEL_FORCE   1.4f

// Global Variables (Ensure these are defined in globals.h)
extern float glow_value;
extern float hue_value;
extern int orbCount;
extern bool escape_mode;
extern float hueShift;
extern float control_sensitivity;
extern float control_brightness_floor;
extern float waveform_height_scale;
extern float waveform_smoothing_factor;
extern int control_waveform_points;


// Define the constants for the modes
enum VisualizationMode {
    WAVEFORM_MODE,
    GRAVITY_MODE,
    CUBE_MODE,
};

// Audio Callback function for PortAudio
static int MyAudioCallback(const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData) {
    if (inputBuffer != NULL) {
        const float *in = (const float*) inputBuffer;
        for (unsigned int i = 0; i < framesPerBuffer; i++) {
            gAudioBuffer[i] = in[i];
        }
        audioDataReady = 1;
    }
    return paContinue;
}

// Function to process FFT and get the bass level
float ProcessFFT() {
    kiss_fft_cfg cfg = kiss_fft_alloc(FFT_SIZE, 0, NULL, NULL);
    if (!cfg) return 0;

    kiss_fft_cpx in[FFT_SIZE];
    kiss_fft_cpx out[FFT_SIZE];
    for (int i = 0; i < FFT_SIZE; i++) {
        in[i].r = gAudioBuffer[i];
        in[i].i = 0;
    }
    kiss_fft(cfg, in, out);
    free(cfg);

    float freqRes = SAMPLE_RATE / (float)FFT_SIZE;
    int bassBins = (int)(BASS_FREQ_THRESHOLD / freqRes);
    if (bassBins < 1) bassBins = 1;

    float maxBass = 0.0f;
    for (int i = 1; i < bassBins; i++) {
        float mag = sqrtf(out[i].r * out[i].r + out[i].i * out[i].i);
        if (mag > maxBass) maxBass = mag;
    }
    float normalized = maxBass / BASS_NORM_FACTOR;
    return (normalized > 1.0f) ? 1.0f : normalized;
}

// Convert HSV to RGB color
Color HSVtoRGB(float h, float s, float v) {
    float r, g, b;
    int i = (int)(h * 6.0f);
    float f = h * 6.0f - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - f * s);
    float t = v * (1.0f - (1.0f - f) * s);

    switch (i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }
    return (Color){ (unsigned char)(r * 255), (unsigned char)(g * 255), (unsigned char)(b * 255), 255 };
}

int main() {
    // Initialize parameters
    float glow_value = 0.000f;
    float control_sensitivity = 1.0f;
    float control_brightness_floor = 0.1f;
    float hue_value = 0.0f;
    int control_waveform_points = 128;
    float waveform_smoothing_factor = 0.5f;
    float waveform_height_scale = 1.0f;

    // Waveform instance
    Waveform waveform(control_waveform_points, waveform_smoothing_factor, waveform_height_scale,
                      control_brightness_floor, glow_value, control_sensitivity, hue_value);

    Menu menu;

    // Init WinSock + Raylib
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return 1;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(800, 600, "Orb Visualizer");
    SetTargetFPS(240);

    // Camera for CUBE_MODE
    Camera3D camera = { 0 };
    camera.position = { 10.0f, 10.0f, 10.0f };
    camera.target = { 0.0f, 0.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Cube field setup (persistent)
    std::vector<Cube> cubeField;

    for (int x = -5; x <= 5; ++x) {
        for (int z = -5; z <= 5; ++z) {
            cubeField.emplace_back(
                Vector3{ (float)x * 2.5f, 0.0f, (float)z * 2.5f },
                1.0f,                             // size
                Color{255, 255, 255, 255},        // color (white)
                1.0f                              // rotationSpeed
            );
        }
    }

    // Audio + Orb init
    InitOrbs(MAX_ORBS);
    Pa_Initialize();
    PaStream* stream;
    Pa_OpenDefaultStream(&stream, 1, 0, paFloat32, SAMPLE_RATE, FRAMES_PER_BUFFER, MyAudioCallback, NULL);
    Pa_StartStream(stream);

    // Initial visualizer mode
    VisualizationMode currentMode = WAVEFORM_MODE;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_E)) {
            currentMode = static_cast<VisualizationMode>((currentMode + 1) % 3);
        }

        if (audioDataReady) {
            audioDataReady = 0;
            float rawBass = ProcessFFT();
            float boostedBass = powf(rawBass, BASS_BOOST_EXPONENT);
            glow_value = glow_value * 0.9f + boostedBass * 0.1f;
        }

        float visualGlow = fminf(powf(glow_value, 1.0f) * 1.0f, 1.0f);

        if (autoCycleHue) {
            hueShift += 15 * GetFrameTime();
            if (hueShift >= 360.0f) hueShift -= 360.0f;
        }

        Color orbColor = HSVtoRGB(hueShift / 360.0f, 1.0f, 1.0f);
        UpdateOrbs(visualGlow, escape_mode, orbColor);
        SendToPython(glow_value, hueShift);

        BeginDrawing();
        ClearBackground(BLACK);

        if (currentMode == WAVEFORM_MODE) {
            std::vector<float> audioVector(std::begin(gAudioBuffer), std::end(gAudioBuffer));
            waveform.drawWaveform(audioVector, orbColor);
        } else if (currentMode == GRAVITY_MODE) {
            DrawOrbs();
        } else if (currentMode == CUBE_MODE) {
            Vector2 mouse = GetMousePosition();
            float screenCenterX = GetScreenWidth() / 2.0f;
            float screenCenterY = GetScreenHeight() / 2.0f;

            float offsetX = (mouse.x - screenCenterX) / screenCenterX;
            float offsetY = (mouse.y - screenCenterY) / screenCenterY;

            float strength = 5.0f;

            camera.position.x = sinf(GetTime()) * 20.0f + offsetX * strength;
            camera.position.y = 10.0f + offsetY * strength;
            camera.position.z = cosf(GetTime()) * 20.0f;

            camera.target = { 0.0f, 0.0f, 0.0f };
            BeginMode3D(camera);
            for (auto& cube : cubeField)
                cube.Update(GetFrameTime(), visualGlow, hueShift / 360.0f);

            for (const auto& cube : cubeField)
                cube.Draw();
            for (const auto& cube : cubeField) cube.Draw();
            EndMode3D();
        }

        DrawText(TextFormat("HueShift: %.2f", hueShift), 10, 10, 20, WHITE);
        DrawText(TextFormat("Build: %s", VERSION), 10, 34, 14, GREEN);
        DrawText(TextFormat("Orb Count: %d", orbCount), 10, 58, 20, GREEN);
        DrawText(TextFormat("Glow Value: %.3f", glow_value), 10, 82, 20, GREEN);
        DrawText(TextFormat("FPS: %d", GetFPS()), 10, 106, 20, GREEN);
        DrawText(currentMode == WAVEFORM_MODE ? "Current Mode: Waveform" :
                 currentMode == GRAVITY_MODE ? "Current Mode: Gravity Orbs" :
                 "Current Mode: Cube Field", 10, 130, 20, GREEN);

        menu.Update();
        menu.Draw();

        EndDrawing();
    }

    // Cleanup
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    WSACleanup();
    CloseWindow();
    return 0;
}
