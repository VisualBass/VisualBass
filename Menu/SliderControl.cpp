#include "SliderControl.h"
#include <cstdio>  // For sprintf
#include "GetColorFromHue.h"
#include "../globals.h"  // Make sure hueShift is declared as extern in this header

// Constructor for int reference (e.g., numOrbs)
SliderControl::SliderControl(int minValue, int maxValue, int& value, const char* label, float& hueShift)
    : minValue(minValue), maxValue(maxValue), value(value), label(label), hueShift(hueShift) {}

// Method to draw the slider with color reflecting hueShift
void SliderControl::DrawSlider(Rectangle startArea) {
    // Calculate slider width dynamically based on the menu or screen size
    const int sliderWidth = (int)(GetScreenWidth() * 0.5f);  // 50% of screen width
    const int sliderHeight = 20; // Fixed height for the slider

    // 1. Draw the background for the slider (gray box)
    DrawRectangle(startArea.x, startArea.y, sliderWidth, sliderHeight, DARKGRAY);

    // 2. Calculate the width of the progress box based on the current value (numOrbs)
    float progressWidth = (value - minValue) * (sliderWidth / (float)(maxValue - minValue));
    int progressWidthInt = (int)progressWidth;  // Casting to int for Rectangle width

    // 3. Draw the progress box (colored box that reflects the current value, overlaps the gray box)
    Rectangle progressBox = { startArea.x, startArea.y, progressWidthInt, sliderHeight };
    Color progressColor = GetColorFromHue((int)hueShift);  // Use hueShift to color the progress box

    // Check if mouse is over the progress box and invert the color only during hover
    Vector2 mousePos = GetMousePosition();
    bool isHovering = CheckCollisionPointRec(mousePos, startArea);
    if (isHovering) {
        progressColor = InvertColor(progressColor);  // Invert the color on hover
    }

    // Draw the progress box (colored portion)
    DrawRectangleRec(progressBox, progressColor);

    // 4. Draw the thin white line at the current position (replacing the handle)
    int handleX = (int)(startArea.x + progressWidth - 1); // Position the line at the current slider position
    DrawLine(handleX, (int)(startArea.y), handleX, (int)(startArea.y + sliderHeight), WHITE);  // Draw a 1px white line as the "handle"

    // Display the value of the slider (Orbs count) on the right side
    char text[32];
    sprintf(text, "%s: %d", label, value);

    // Calculate the width of the text for centering
    int textWidth = MeasureText(text, 20);  // Measure the text width
    int centerX = startArea.x + (sliderWidth - textWidth) / 2;  // Calculate X position to center the text

    // Draw the text with color based on hueShift, invert on hover
    Color labelColor = isHovering ? InvertColor(progressColor) : progressColor;
    DrawText(text, centerX, (int)(startArea.y - 30), 20, labelColor);  // Center the text
}

// Method to update the slider value based on mouse input
void SliderControl::UpdateSlider() {
    // Get mouse position and check if it's over the slider track
    Vector2 mousePos = GetMousePosition();

    // Only update the value when the mouse is pressed
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, (Rectangle){100, 100, 400, 40})) {
        // Update the slider value based on mouse position
        value = (int)((((mousePos.x - 100) / 400.0f) * (maxValue - minValue)) + minValue);

        // Ensure the value stays within bounds
        if (value < minValue) value = minValue;
        if (value > maxValue) value = maxValue;
    }
}

// Invert color function for use with hover
Color SliderControl::InvertColor(Color color) {
    return (Color) {
        (unsigned char)(255 - color.r),
        (unsigned char)(255 - color.g),
        (unsigned char)(255 - color.b),
        color.a
    };  // Invert RGB components, keeping alpha unchanged
}
