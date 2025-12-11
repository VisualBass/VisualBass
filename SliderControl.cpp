#include "SliderControl.h"
#include <cstdio>
#include <cmath>
#include "Menu/GetColorFromHue.h"

// --- CONSTRUCTOR 1: INTEGER (Legacy/Orbs) ---
SliderControl::SliderControl(int min, int max, int& var, const char* text, float& hue)
    : type(INT_MODE), intValPtr(&var), minInt(min), maxInt(max),
      label(text), hueShiftRef(hue)
{
}

// --- CONSTRUCTOR 2: FLOAT (New/Brightness) ---
SliderControl::SliderControl(float min, float max, float& var, const char* text, float& hue)
    : type(FLOAT_MODE), floatValPtr(&var), minFloat(min), maxFloat(max),
      label(text), hueShiftRef(hue)
{
}

void SliderControl::DrawSlider(Rectangle startArea) {
    float width = (startArea.width > 0) ? startArea.width : 200.0f;
    float height = (startArea.height > 0) ? startArea.height : 20.0f;
    sliderArea = { startArea.x, startArea.y, width, height };

    // 1. Background
    DrawRectangleRec(sliderArea, DARKGRAY);

    // 2. Calculate Progress (0.0 to 1.0)
    float t = 0.0f;
    if (type == INT_MODE) {
        float range = (float)(maxInt - minInt);
        if (range > 0) t = (*intValPtr - minInt) / range;
    } else {
        float range = maxFloat - minFloat;
        if (range > 0) t = (*floatValPtr - minFloat) / range;
    }

    // 3. Draw Progress Bar
    float progressWidth = t * sliderArea.width;
    Rectangle progressBox = { sliderArea.x, sliderArea.y, progressWidth, sliderArea.height };
    Color baseColor = GetColorFromHue((int)hueShiftRef);

    Vector2 mousePos = GetMousePosition();
    bool isHovering = CheckCollisionPointRec(mousePos, sliderArea);
    Color fillColor = isHovering ? InvertColor(baseColor) : baseColor;

    DrawRectangleRec(progressBox, fillColor);

    // 4. Handle Line
    float handleX = sliderArea.x + progressWidth;
    // Clamp handle
    if (handleX > sliderArea.x + sliderArea.width - 1) handleX = sliderArea.x + sliderArea.width - 1;
    DrawLine((int)handleX, (int)sliderArea.y, (int)handleX, (int)(sliderArea.y + sliderArea.height), WHITE);

    // 5. Label
    char text[64];
    Color labelColor = isHovering ? baseColor : fillColor;

    if (type == INT_MODE) {
        std::snprintf(text, sizeof(text), "%s: %d", label, *intValPtr);
    } else {
        // For floats, we display 2 decimal places (e.g., "Floor: 0.50")
        // Or if you prefer percentage: "Floor: %.0f%%", *floatValPtr * 100
        std::snprintf(text, sizeof(text), "%s: %.2f", label, *floatValPtr);
    }

    // Draw text above slider
    int textWidth = MeasureText(text, 20);
    int centerX = (int)(sliderArea.x + (sliderArea.width - textWidth) * 0.5f);
    DrawText(text, centerX, (int)(sliderArea.y - 25), 20, labelColor);
}

void SliderControl::UpdateSlider() {
    Vector2 mousePos = GetMousePosition();
    bool over = CheckCollisionPointRec(mousePos, sliderArea);

    // --- MOUSE WHEEL ---
    float wheel = GetMouseWheelMove();
    if (over && wheel != 0.0f) {
        if (type == INT_MODE) {
            float range = (float)(maxInt - minInt);
            int delta = (int)std::round(range * stepPercent * wheel);
            if (delta == 0) delta = (wheel > 0) ? 1 : -1;
            *intValPtr += delta;
            if (*intValPtr < minInt) *intValPtr = minInt;
            if (*intValPtr > maxInt) *intValPtr = maxInt;
        } else {
            float range = maxFloat - minFloat;
            // Float precision step
            *floatValPtr += (range * stepPercent * wheel);
            if (*floatValPtr < minFloat) *floatValPtr = minFloat;
            if (*floatValPtr > maxFloat) *floatValPtr = maxFloat;
        }
    }

    // --- CLICK DRAG ---
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && over && sliderArea.width > 0.f) {
        float t = (mousePos.x - sliderArea.x) / sliderArea.width;
        if (t < 0.f) t = 0.f;
        if (t > 1.f) t = 1.f;

        if (type == INT_MODE) {
            *intValPtr = minInt + (int)std::round(t * (float)(maxInt - minInt));
        } else {
            *floatValPtr = minFloat + (t * (maxFloat - minFloat));
        }
    }
}

Color SliderControl::InvertColor(Color color) {
    return (Color){ (unsigned char)(255 - color.r), (unsigned char)(255 - color.g), (unsigned char)(255 - color.b), color.a };
}