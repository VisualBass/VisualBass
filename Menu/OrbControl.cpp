#include "OrbControl.h"
#include "raylib.h"
#include <cmath>
#include "GetColorFromHue.h"

// Globals expected in globals.h:
// extern Orb* orbs;
// extern int orbCount;
// extern float hueShift;

OrbControl::OrbControl(int min, int max)
: minOrbs(min), maxOrbs(max) {
    // Clamp any existing orbCount into range at startup
    if (orbCount < minOrbs) orbCount = minOrbs;
    if (orbCount > maxOrbs) orbCount = maxOrbs;
}

void OrbControl::DrawOrbSlider(Rectangle startArea) {
    // Default size if caller passes 0
    const float sliderWidth  = (startArea.width  > 0.f) ? startArea.width  : 400.f;
    const float sliderHeight = (startArea.height > 0.f) ? startArea.height : 20.f;

    sliderArea = { startArea.x, startArea.y, sliderWidth, sliderHeight };

    // Background
    DrawRectangle((int)sliderArea.x, (int)sliderArea.y,
                  (int)sliderArea.width, (int)sliderArea.height, DARKGRAY);

    // Progress width from orbCount
    const float range = (float)(maxOrbs - minOrbs);
    const float t = (range > 0.f) ? ((float)(orbCount - minOrbs) / range) : 0.f;
    float progressWidth = t * sliderArea.width;

    // Progress fill
    Rectangle progressBox = { sliderArea.x, sliderArea.y, progressWidth, sliderArea.height };
    Color progressColor = GetColorFromHue((int)hueShift);
    DrawRectangleRec(progressBox, progressColor);

    // Handle line
    float handleXf = sliderArea.x + progressWidth;
    if (handleXf < sliderArea.x) handleXf = sliderArea.x;
    if (handleXf > sliderArea.x + sliderArea.width - 1.f) handleXf = sliderArea.x + sliderArea.width - 1.f;
    int handleX = (int)std::round(handleXf);
    DrawLine(handleX, (int)sliderArea.y, handleX, (int)(sliderArea.y + sliderArea.height), WHITE);

    // Label to the right
    char text[32];
    std::snprintf(text, sizeof(text), "Orbs: %d", orbCount);
    DrawText(text, (int)(sliderArea.x + sliderArea.width + 20.f), (int)(sliderArea.y - 10.f), 20, WHITE);
}

void OrbControl::UpdateOrbSlider() {
    Vector2 mousePos = GetMousePosition();
    bool over = CheckCollisionPointRec(mousePos, sliderArea);

    // Mouse wheel: +/- 5% of range per notch when hovering slider
    float wheel = GetMouseWheelMove();
    if (over && wheel != 0.0f) {
        float raw = (float)(maxOrbs - minOrbs) * stepPercent * wheel;
        int delta = (int)std::round(raw);
        if (delta == 0) delta = (wheel > 0.f) ? 1 : -1;  // always move at least 1

        orbCount += delta;
        if (orbCount < minOrbs) orbCount = minOrbs;
        if (orbCount > maxOrbs) orbCount = maxOrbs;
    }

    // Click-drag maps mouse X to value
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && over && sliderArea.width > 0.f) {
        float t = (mousePos.x - sliderArea.x) / sliderArea.width;
        if (t < 0.f) t = 0.f;
        if (t > 1.f) t = 1.f;
        orbCount = minOrbs + (int)std::round(t * (float)(maxOrbs - minOrbs));
        if (orbCount < minOrbs) orbCount = minOrbs;
        if (orbCount > maxOrbs) orbCount = maxOrbs;
    }

    // Note: this adjusts orbCount only.
    // If your render/update code does not already recreate `orbs` when orbCount changes,
    // tell me and I will add a safe resize helper that reallocates the global `orbs` array.
}
