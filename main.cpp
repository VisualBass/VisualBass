#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI
#define NOUSER
#define NOWINRES

#define BASS_LOW_FREQ       35.0f
#define BASS_HIGH_FREQ      75.0f
#define BASS_NORM_FACTOR    275.0f
#define BASS_BOOST_EXPONENT 2.0f
#define GLOW_MIX            0.275f

const char* VERSION = "Development 0.4.1 - Idle XP";

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
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
#include "globals.h"
#include "imgui.h"
#include "menu.h"
#include "cube.h"
#include "particle01.h"

// --- IDLE GAME INCLUDES ---
#include "IdleGame/IdleGame.h"
#include "Menu/IdleGameMenu/IdleGameMenu.h"

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

float ProcessFFT() {
    kiss_fft_cfg cfg = kiss_fft_alloc(FFT_SIZE, 0, NULL, NULL);
    if (!cfg) return 0.0f;

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

    for (int i = lowBin; i <= highBin; i++) {
        float mag = sqrtf(out[i].r * out[i].r + out[i].i * out[i].i);
        sumBass += mag;
        count++;
    }

    float avgBass = (count > 0) ? (sumBass / (float)count) : 0.0f;
    float normalized = avgBass / BASS_NORM_FACTOR;
    if (normalized > 1.0f) normalized = 1.0f;
    if (normalized < 0.0f) normalized = 0.0f;

    float boostedBass = powf(normalized, BASS_BOOST_EXPONENT);
    return boostedBass;
}

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

int main() {
    float glow_value_local       = 0.0f;
    float control_sensitivity    = 1.0f;
    float hue_value              = 0.0f;
    int   control_waveform_points   = 128;
    float waveform_smoothing_factor = 0.5f;
    float waveform_height_scale   = 1.0f;

    Waveform waveform(
        control_waveform_points,
        waveform_smoothing_factor,
        waveform_height_scale,
        brightnessFloor,
        glow_value_local,
        control_sensitivity,
        hue_value
    );

    Menu menu;
    IdleGame idleGame;
    IdleGameMenu idleGameMenu;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return 1;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(800, 600, "Orb Visualizer");
    SetTargetFPS(240);

    Camera3D camera = { 0 };
    camera.position   = { 10.0f, 10.0f, 10.0f };
    camera.target     = { 0.0f,  0.0f,  0.0f  };
    camera.up         = { 0.0f,  1.0f,  0.0f  };
    camera.fovy       = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    std::vector<Cube> cubeField;
    for (int x = -5; x <= 5; ++x) {
        for (int z = -5; z <= 5; ++z) {
            cubeField.emplace_back(
                Vector3{ (float)x * 2.5f, 0.0f, (float)z * 2.5f },
                1.0f, Color{255,255,255,255}, 1.0f
            );
        }
    }

    InitOrbs(MAX_ORBS);
    Pa_Initialize();
    PaStream* stream;
    Pa_OpenDefaultStream(&stream, 1, 0, paFloat32, SAMPLE_RATE, FRAMES_PER_BUFFER, MyAudioCallback, NULL);
    Pa_StartStream(stream);

    VisualizationMode currentMode = WAVEFORM_MODE;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_E)) {
            currentMode = static_cast<VisualizationMode>((currentMode + 1) % 4);
        }

        if (audioDataReady) {
            audioDataReady = 0;
            float boostedBass = ProcessFFT();
            glow_value = glow_value * (1.0f - GLOW_MIX) + boostedBass * GLOW_MIX;
        }

        float finalBrightness = brightnessFloor + (glow_value * (1.0f - brightnessFloor));
        float visualGlow = fminf(powf(glow_value, 1.0f), 1.0f);

        if (autoCycleHue) {
            hueShift += hueSpeed * GetFrameTime();
            if (hueShift >= 360.0f) hueShift -= 360.0f;
        }

        idleGame.Update(GetFrameTime(), visualGlow);
        Color orbColor = HSVtoRGB(hueShift / 360.0f, 1.0f, 1.0f);
        UpdateOrbs(visualGlow, escape_mode, orbColor);
        SendToPython(finalBrightness, hueShift / 360.0f);

        BeginDrawing();
        ClearBackground(BLACK);

        if (currentMode == PARTICLE_MODE_01) {
            Vector2 mouse = GetMousePosition();
            float screenCX = GetScreenWidth() / 2.0f;
            float screenCY = GetScreenHeight() / 2.0f;
            float offsetX = (mouse.x - screenCX) / screenCX;
            float offsetY = (mouse.y - screenCY) / screenCY;
            float cameraRange = 20.0f;

            camera.position.x = sinf(GetTime()) * cameraRange + offsetX * 5.0f;
            camera.position.y = 10.0f + offsetY * 5.0f;
            camera.position.z = cosf(GetTime()) * cameraRange;
            BeginMode3D(camera);
            particleSystem.Update(GetFrameTime(), visualGlow, orbColor);
            particleSystem.Draw();
            EndMode3D();
        }
        else if (currentMode == WAVEFORM_MODE) {
            std::vector<float> audioVector(std::begin(gAudioBuffer), std::end(gAudioBuffer));
            waveform.drawWaveform(audioVector, orbColor);
        } else if (currentMode == GRAVITY_MODE) {
            DrawOrbs();
        } else if (currentMode == CUBE_MODE) {
            Vector2 mouse = GetMousePosition();
            float screenCX = GetScreenWidth() / 2.0f;
            float screenCY = GetScreenHeight() / 2.0f;
            float offsetX = (mouse.x - screenCX) / screenCX;
            float offsetY = (mouse.y - screenCY) / screenCY;
            float cameraRange = 20.0f;

            camera.position.x = sinf(GetTime()) * cameraRange + offsetX * 5.0f;
            camera.position.y = 10.0f + offsetY * 5.0f;
            camera.position.z = cosf(GetTime()) * cameraRange;
            BeginMode3D(camera);
            for (auto &cube : cubeField) {
                cube.Update(GetFrameTime(), visualGlow, hueShift / 360.0f);
                cube.Draw();
            }
            EndMode3D();
        }

        float screenH = (float)GetScreenHeight();
        float uiScale = (screenH > 1080.0f) ? 1.25f : 1.0f;
        idleGameMenu.Draw(idleGame, uiScale);

        // --- HUD Removed Here ---

        menu.Update();
        menu.Draw((int)currentMode);

        EndDrawing();
    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    WSACleanup();
    CloseWindow();
    return 0;
}