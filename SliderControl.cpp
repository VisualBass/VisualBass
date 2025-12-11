#include "SliderControl.h"
#include <cstdio>      // For sprintf
#include <cmath>       // For std::round
#include "Menu/GetColorFromHue.h"
#include "globals.h"   // Ensure hueShift access if needed

// Constructor: Initializes the reference to the variable (value) and the hue
SliderControl::SliderControl(int minValue, int maxValue, int& value, const char* label, float& hueShift)
    : minValue(minValue), maxValue(maxValue), value(value), label(label), hueShift(hueShift) {}

// Method to draw the slider
void SliderControl::DrawSlider(Rectangle startArea) {
    // FIX: Use the width/height passed in startArea instead of hardcoding screen percentage.
    // If startArea width is 0 (safety check), fallback to a default.
    float width = (startArea.width > 0) ? startArea.width : 200.0f;
    float height = (startArea.height > 0) ? startArea.height : 20.0f;

    // Store the true area for input hit-testing
    sliderArea = { startArea.x, startArea.y, width, height };

    // 1. Draw the background (gray box)
    DrawRectangle((int)sliderArea.x, (int)sliderArea.y, (int)sliderArea.width, (int)sliderArea.height, DARKGRAY);

    // 2. Calculate the width of the progress box based on the current value
    const float range = (float)(maxValue - minValue);
    const float t = (range > 0.f) ? ((float)(value - minValue) / range) : 0.f;
    float progressWidth = t * sliderArea.width;

    // 3. Draw the progress box (colored box)
    Rectangle progressBox = { sliderArea.x, sliderArea.y, progressWidth, sliderArea.height };
    Color baseColor = GetColorFromHue((int)hueShift);

    // Hover check
    Vector2 mousePos = GetMousePosition();
    bool isHovering = CheckCollisionPointRec(mousePos, sliderArea);

    Color fillColor = isHovering ? InvertColor(baseColor) : baseColor;
    DrawRectangleRec(progressBox, fillColor);

    // 4. Draw the handle line
    float handleXf = sliderArea.x + progressWidth;
    if (handleXf < sliderArea.x) handleXf = sliderArea.x;
    if (handleXf > sliderArea.x + sliderArea.width - 1.f) handleXf = sliderArea.x + sliderArea.width - 1.f;
    int handleX = (int)std::round(handleXf);
    DrawLine(handleX, (int)sliderArea.y, handleX, (int)(sliderArea.y + sliderArea.height), WHITE);

    // 5. Display the value label centered above
    char text[64];
    std::snprintf(text, sizeof(text), "%s: %d", label, value);

    int textWidth = MeasureText(text, 20);
    int centerX = (int)(sliderArea.x + (sliderArea.width - textWidth) * 0.5f);

    Color labelColor = isHovering ? baseColor : fillColor;
    DrawText(text, centerX, (int)(sliderArea.y - 30), 20, labelColor);
}

void SliderControl::UpdateSlider() {
    Vector2 mousePos = GetMousePosition();
    bool over = CheckCollisionPointRec(mousePos, sliderArea);

    // 1) Mouse wheel adjustment
    float wheel = GetMouseWheelMove();
    if (over && wheel != 0.0f) {
        float rawDelta = (float)(maxValue - minValue) * stepPercent * wheel;
        int delta = (int)std::round(rawDelta);
        if (delta == 0) delta = (wheel > 0.f) ? 1 : -1;

        value += delta;
        if (value < minValue) value = minValue;
        if (value > maxValue) value = maxValue;
    }

    // 2) Click-drag adjustment
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && over && sliderArea.width > 0.f) {
        float t = (mousePos.x - sliderArea.x) / sliderArea.width;
        if (t < 0.f) t = 0.f;
        if (t > 1.f) t = 1.f;
        value = minValue + (int)std::round(t * (float)(maxValue - minValue));
        if (value < minValue) value = minValue;
        if (value > maxValue) value = maxValue;
    }
}

Color SliderControl::InvertColor(Color color) {
    return (Color) {
        (unsigned char)(255 - color.r),
        (unsigned char)(255 - color.g),
        (unsigned char)(255 - color.b),
        color.a
    };
}