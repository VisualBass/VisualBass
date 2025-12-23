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
#define SMOOTH_SPEED        15.0f

const char* VERSION = "Development 0.5.1";

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

#include "IdleGame/IdleGame.h"
#include "Menu/IdleGameMenu/IdleGameMenu.h"

#define SAMPLE_RATE         44100
#define FFT_SIZE            FRAMES_PER_BUFFER

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

    kiss_fft_cpx in_fft[FFT_SIZE];
    kiss_fft_cpx out_fft[FFT_SIZE];
    for (int i = 0; i < FFT_SIZE; i++) {
        in_fft[i].r = gAudioBuffer[i];
        in_fft[i].i = 0;
    }
    kiss_fft(cfg, in_fft, out_fft);
    free(cfg);

    float freqRes = SAMPLE_RATE / (float)FFT_SIZE;
    int   lowBin  = (int)(BASS_LOW_FREQ / freqRes);
    int   highBin = (int)(BASS_HIGH_FREQ / freqRes);
    float sumBass = 0.0f;
    int   count   = 0;

    for (int i = lowBin; i <= highBin; i++) {
        float mag = sqrtf(out_fft[i].r * out_fft[i].r + out_fft[i].i * out_fft[i].i);
        sumBass += mag;
        count++;
    }

    float avgBass = (count > 0) ? (sumBass / (float)count) : 0.0f;
    float normalized = avgBass / BASS_NORM_FACTOR;
    if (normalized > 1.0f) normalized = 1.0f;
    if (normalized < 0.0f) normalized = 0.0f;

    return powf(normalized, BASS_BOOST_EXPONENT);
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
    float renderGlow             = 0.0f;

    Waveform waveform(128, 0.5f, 1.0f, brightnessFloor, glow_value, 1.0f, 0.0f);
    Menu menu;
    IdleGame idleGame;
    IdleGameMenu idleGameMenu;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return 1;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(300, 720, "Visual Bass Sync");
    SetTargetFPS(240);

    particleSystem.Init();

    Camera3D camera = { 0 };
    camera.position   = { 10.0f, 10.0f, 10.0f };
    camera.target     = { 0.0f,  0.0f,  0.0f  };
    camera.up         = { 0.0f,  1.0f,  0.0f  };
    camera.fovy       = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    std::vector<Cube> cubeField = GenerateCubeField(cubeSettings);

    int prevX = cubeSettings.gridX;
    int prevY = cubeSettings.gridY;
    int prevZ = cubeSettings.gridZ;

    InitOrbs(DEFAULT_MAX_PARTICLES);
    Pa_Initialize();
    PaStream* stream;
    Pa_OpenDefaultStream(&stream, 1, 0, paFloat32, SAMPLE_RATE, FRAMES_PER_BUFFER, MyAudioCallback, NULL);
    Pa_StartStream(stream);

    VisualizationMode currentMode = WAVEFORM_MODE;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_E)) {
            currentMode = static_cast<VisualizationMode>((currentMode + 1) % 4);
        }

        if (IsKeyPressed(KEY_I)) {
            enableInterpolation = !enableInterpolation;
        }

        if (cubeSettings.gridX != prevX || cubeSettings.gridY != prevY || cubeSettings.gridZ != prevZ) {
            cubeField = GenerateCubeField(cubeSettings);
            prevX = cubeSettings.gridX;
            prevY = cubeSettings.gridY;
            prevZ = cubeSettings.gridZ;
        }

        if (audioDataReady) {
            audioDataReady = 0;
            float boostedBass = ProcessFFT();

            // --- GLOBAL PUMP APPLICATION ---
            float pumpedBass = boostedBass * globalPump;
            glow_value = glow_value * (1.0f - GLOW_MIX) + pumpedBass * GLOW_MIX;
        }

        float dt = GetFrameTime();
        if (enableInterpolation) {
            renderGlow = Lerp(renderGlow, glow_value, SMOOTH_SPEED * dt);
        } else {
            renderGlow = glow_value;
        }

        float finalBrightness = brightnessFloor + (renderGlow * (1.0f - brightnessFloor));
        float visualGlow = fminf(powf(renderGlow, 1.0f), 1.0f);

        if (autoCycleHue) {
            hueShift += hueSpeed * dt;
            if (hueShift >= 360.0f) hueShift -= 360.0f;
        }

        idleGame.Update(dt, visualGlow);
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
            particleSystem.Update(dt, visualGlow, orbColor);
            particleSystem.Draw(camera); // FIXED: Passing camera for Billboard support
            EndMode3D();
        }
        else if (currentMode == WAVEFORM_MODE) {
            std::vector<float> audioVector(std::begin(gAudioBuffer), std::end(gAudioBuffer));
            waveform.drawWaveform(audioVector, orbColor);
        }
        else if (currentMode == GRAVITY_MODE) {
            DrawOrbs();
        }
        else if (currentMode == CUBE_MODE) {
            // --- DYNAMIC CAMERA ZOOM ---
            float maxDim = (float)fmax(fmax(cubeSettings.gridX, cubeSettings.gridY), cubeSettings.gridZ);
            float currentSpacing = 1.0f + (visualGlow * cubeSettings.spacingIntensity);
            float zoomDistance = maxDim * currentSpacing * 1.5f;
            if (zoomDistance < 12.0f) zoomDistance = 12.0f;

            camera.position = { zoomDistance * 0.6f, zoomDistance * 0.6f, zoomDistance };
            camera.target   = { 0.0f, 0.0f, 0.0f };

            BeginMode3D(camera);
            for (auto &cube : cubeField) {
                cube.Update(GetTime(), visualGlow, hueShift / 360.0f, cubeSettings);
                cube.Draw();
            }
            EndMode3D();
        }

        float screenH = (float)GetScreenHeight();
        float uiScale = (screenH > 1080.0f) ? 1.25f : 1.0f;
        idleGameMenu.Draw(idleGame, uiScale);
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