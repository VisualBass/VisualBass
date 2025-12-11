#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

// Prevent Windows macros that conflict with raylib
#define NOGDI           // Avoid 'Rectangle' macro
#define NOUSER          // Avoid 'ShowCursor' & 'CloseWindow'
#define NOWINRES        // Avoid 'LoadImage'

// ---------- Adjust these constants to tweak “punchiness” ----------

// Frequency range (Hz) to average for bass detection (lower = sub‐kick, higher = more rumble)
#define BASS_LOW_FREQ       35.0f
#define BASS_HIGH_FREQ      75.0f

// Normalization factor: lower values = bigger visual response
#define BASS_NORM_FACTOR    275.0f

// Exponent applied after normalization: higher = more contrast/sharpness
#define BASS_BOOST_EXPONENT 2.0f

// Mix factor for glow_value smoothing:
//   0.0 = no response,
//   1.0 = fully snap to boostedBass.
// Lower values make the bass “softer,” higher values make it more “punchy.”
#define GLOW_MIX            0.275f

// ------------------------------------------------------------------

const char* VERSION = "Development Build 0.5.1";  // Update this to bump the version
extern int numOrbs;

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
#include "globals.h"     // Ensure these globals are declared there:
#include "imgui.h"               //   float glow_value;
                         //   float hue_value;
                         //   int orbCount;
                         //   bool escape_mode;
                         //   float hueShift;
                         //   float control_sensitivity;
                         //   float control_brightness_floor;
                         //   float waveform_height_scale;
                         //   float waveform_smoothing_factor;
                         //   int control_waveform_points;
#include "menu.h"
#include "cube.h"
#include "particle01.h"  // Include the renamed particle system

// Standard constants
#define SAMPLE_RATE         44100
#define FFT_SIZE            FRAMES_PER_BUFFER
#define MAX_ORBS            1250
#define GRAVITY_RAMP        16.0f
#define CENTER_SUCK_RADIUS  30.0f
#define ORB_RESPAWN         true
#define SPAWN_MARGIN        40.0f
#define MOUSE_REPEL_RADIUS  200.0f
#define MOUSE_REPEL_FORCE   1.4f

enum VisualizationMode {
    WAVEFORM_MODE,
    GRAVITY_MODE,
    CUBE_MODE,
    PARTICLE_MODE_01,
};

// Audio callback for PortAudio: fills gAudioBuffer and flags data ready
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

// ProcessFFT: averages magnitudes between BASS_LOW_FREQ and BASS_HIGH_FREQ, normalizes,
// applies an exponent, and returns a value in [0,1].
float ProcessFFT() {
    kiss_fft_cfg cfg = kiss_fft_alloc(FFT_SIZE, 0, NULL, NULL);
    if (!cfg) return 0.0f;

    // Prepare input for FFT
    kiss_fft_cpx in[FFT_SIZE];
    kiss_fft_cpx out[FFT_SIZE];
    for (int i = 0; i < FFT_SIZE; i++) {
        in[i].r = gAudioBuffer[i];
        in[i].i = 0;
    }
    kiss_fft(cfg, in, out);
    free(cfg);

    float freqRes = SAMPLE_RATE / (float)FFT_SIZE;
    int   lowBin  = (int)(BASS_LOW_FREQ / freqRes);
    int   highBin = (int)(BASS_HIGH_FREQ / freqRes);
    float sumBass = 0.0f;
    int   count   = 0;

    // Sum magnitudes between lowBin and highBin
    for (int i = lowBin; i <= highBin; i++) {
        float mag = sqrtf(out[i].r * out[i].r + out[i].i * out[i].i);
        sumBass += mag;
        count++;
    }

    float avgBass = (count > 0) ? (sumBass / (float)count) : 0.0f;

    // Normalize and clamp
    float normalized = avgBass / BASS_NORM_FACTOR;
    if (normalized > 1.0f) normalized = 1.0f;
    if (normalized < 0.0f) normalized = 0.0f;

    // Apply exponent to boost contrast
    float boostedBass = powf(normalized, BASS_BOOST_EXPONENT);
    return boostedBass;
}

// Convert HSV to RGB (h in [0..1], s in [0..1], v in [0..1])
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

Particle01 particleSystem(MAX_PARTICLES);

void UpdateAndDrawParticles(Color orbColor) {
    float deltaTime = GetFrameTime();
    float glowValue = glow_value * 200.0f;  // Example: scale small glow_value into a spawn intensity

    // Spawn a new particle at a random screen position
    particleSystem.SpawnParticle(
        (Vector3){ (float)GetRandomValue(0, GetScreenWidth()), (float)GetRandomValue(0, GetScreenHeight()), 0.0f },
        WHITE, glowValue
    );

    // Update & draw the particle system
    particleSystem.Update(deltaTime, glow_value, orbColor);
    particleSystem.Draw();
}

int main() {
    // Initialize parameters
    float glow_value             = 0.0f;
    float control_sensitivity    = 1.0f;
    float control_brightness_floor = 0.1f;
    float hue_value              = 0.0f;
    int   control_waveform_points   = 128;
    float waveform_smoothing_factor = 0.5f;
    float waveform_height_scale   = 1.0f;

    Waveform waveform(
        control_waveform_points,
        waveform_smoothing_factor,
        waveform_height_scale,
        control_brightness_floor,
        glow_value,
        control_sensitivity,
        hue_value
    );

    Menu menu;

    // Initialize WinSock + Raylib window
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return 1;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(800, 600, "Orb Visualizer");
    SetTargetFPS(240);

    // Camera setup for CUBE_MODE
    Camera3D camera = { 0 };
    camera.position   = { 10.0f, 10.0f, 10.0f };
    camera.target     = { 0.0f,  0.0f,  0.0f  };
    camera.up         = { 0.0f,  1.0f,  0.0f  };
    camera.fovy       = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Build a cube field
    std::vector<Cube> cubeField;
    for (int x = -5; x <= 5; ++x) {
        for (int z = -5; z <= 5; ++z) {
            cubeField.emplace_back(
                Vector3{ (float)x * 2.5f, 0.0f, (float)z * 2.5f },
                1.0f,                    // size
                Color{255,255,255,255},  // initial color
                1.0f                     // rotation speed
            );
        }
    }

    // Initialize orbs and PortAudio
    InitOrbs(MAX_ORBS);
    Pa_Initialize();
    PaStream* stream;
    Pa_OpenDefaultStream(
        &stream,
        1,                  // 1 input channel (mono)
        0,                  // 0 output channels
        paFloat32,          // 32-bit float samples
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,  // must match FFT_SIZE
        MyAudioCallback,
        NULL
    );
    Pa_StartStream(stream);

    VisualizationMode currentMode = WAVEFORM_MODE;

    while (!WindowShouldClose()) {
        // Cycle modes on KEY_E
        if (IsKeyPressed(KEY_E)) {
            currentMode = static_cast<VisualizationMode>((currentMode + 1) % 4);
        }

        // If new audio data is ready, compute boosted bass and update glow_value
        if (audioDataReady) {
            audioDataReady = 0;
            float boostedBass = ProcessFFT();

            // Apply smoothing/mix: glow = glow*(1-GLOW_MIX) + boostedBass*GLOW_MIX
            glow_value = glow_value * (1.0f - GLOW_MIX) + boostedBass * GLOW_MIX;
        }

        float visualGlow = fminf(powf(glow_value, 1.0f), 1.0f);

        if (autoCycleHue) {
            hueShift += 15 * GetFrameTime();
            if (hueShift >= 360.0f) hueShift -= 360.0f;
        }

        // Convert hueShift to RGB color
        Color orbColor = HSVtoRGB(hueShift / 360.0f, 1.0f, 1.0f);
        UpdateOrbs(visualGlow, escape_mode, orbColor);
        SendToPython(glow_value, hueShift / 360.0f);

        BeginDrawing();
        ClearBackground(BLACK);

        // DRAW CURRENT MODE
        if (currentMode == PARTICLE_MODE_01) {
            UpdateAndDrawParticles(orbColor);
        }

        if (currentMode == WAVEFORM_MODE) {
            // Pass a copy of the audio buffer to waveform
            std::vector<float> audioVector(std::begin(gAudioBuffer), std::end(gAudioBuffer));
            waveform.drawWaveform(audioVector, orbColor);

        } else if (currentMode == GRAVITY_MODE) {
            DrawOrbs();

        } else if (currentMode == CUBE_MODE) {
            Vector2 mouse     = GetMousePosition();
            float screenCX    = GetScreenWidth() / 2.0f;
            float screenCY    = GetScreenHeight() / 2.0f;
            float offsetX     = (mouse.x - screenCX) / screenCX;
            float offsetY     = (mouse.y - screenCY) / screenCY;
            float cameraRange = 20.0f;
            float strength    = 5.0f;

            camera.position.x = sinf(GetTime()) * cameraRange + offsetX * strength;
            camera.position.y = 10.0f + offsetY * strength;
            camera.position.z = cosf(GetTime()) * cameraRange;
            camera.target     = { 0.0f, 0.0f, 0.0f };

            BeginMode3D(camera);
            for (auto &cube : cubeField) {
                cube.Update(GetFrameTime(), visualGlow, hueShift / 360.0f);
                cube.Draw();
            }
            EndMode3D();
        }

        // HUD text
        DrawText(TextFormat("HueShift: %.2f", hueShift), 10, 10, 20, WHITE);
        DrawText(TextFormat("Build: %s", VERSION), 10, 34, 14, GREEN);
        DrawText(TextFormat("Orb Count: %d", orbCount), 10, 58, 20, GREEN);
        DrawText(TextFormat("Glow Value: %.3f", glow_value), 10, 82, 20, GREEN);
        DrawText(TextFormat("FPS: %d", GetFPS()), 10, 106, 20, GREEN);
        // Instead of that long if/else block:
        DrawText(TextFormat("Current Mode: %s", MODE_NAMES[currentMode]), 10, 130, 20, GREEN);

        menu.Update();
        menu.Draw((int)currentMode);

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
