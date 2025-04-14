#include "OrbControl.h"
#include "raylib.h"
#include <cstdio>
#include "SliderControl.h"
#include "../globals.h"
#include "GetColorFromHue.h"

// Constructor initializes the number of orbs
OrbControl::OrbControl(int min, int max) {
    numOrbs = 10;
    minOrbs = min;
    maxOrbs = max;
}

void OrbControl::DrawOrbSlider(Rectangle startArea) {
    const int sliderWidth = 400;
    const int sliderHeight = 20;

    // 1. Draw the background for the slider (gray box)
    DrawRectangle(startArea.x, startArea.y, sliderWidth, sliderHeight, DARKGRAY);

    // 2. Calculate the width of the progress box based on the current value (numOrbs)
    float progressWidth = (numOrbs - minOrbs) * (sliderWidth / (float)(maxOrbs - minOrbs));
    int progressWidthInt = (int)progressWidth;

    // 3. Draw the progress box (colored box that reflects the current value, overlaps the gray box)
    Rectangle progressBox = { startArea.x, startArea.y, (float)progressWidthInt, sliderHeight };
    Color progressColor = GetColorFromHue((int)hueShift);  // Use hueShift to color the progress box
    DrawRectangleRec(progressBox, progressColor);

    // 4. Draw the thin white line at the current position (replacing the handle)
    int handleX = (int)(startArea.x + progressWidth - 1);  // Position the line at the current progress
    DrawLine(handleX, (int)(startArea.y), handleX, (int)(startArea.y + sliderHeight), WHITE);  // Draw a 1px white line

    // Display the value (Orbs) on the right of the progress bar
    char text[32];
    sprintf(text, "Orbs: %d", numOrbs);
    DrawText(text, (int)(startArea.x + sliderWidth + 20), (int)(startArea.y - 10), 20, WHITE);
}

void OrbControl::UpdateOrbSlider() {
    Vector2 mousePos = GetMousePosition();

    // Check if mouse is over the slider and update numOrbs based on the mouse position
    if (CheckCollisionPointRec(mousePos, (Rectangle){100, 100, 400, 40}) && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        numOrbs = (int)((((mousePos.x - 100) / 400.0f) * (maxOrbs - minOrbs)) + minOrbs);
        // Ensure numOrbs stays within the bounds
        if (numOrbs < minOrbs) numOrbs = minOrbs;
        if (numOrbs > maxOrbs) numOrbs = maxOrbs;
    }

    // Support for scrolling to adjust the number of orbs
    if (IsMouseButtonPressed(MOUSE_MIDDLE_BUTTON)) {
        numOrbs += 1;
        if (numOrbs > maxOrbs) numOrbs = maxOrbs;
        if (numOrbs < minOrbs) numOrbs = minOrbs;
    }
}


