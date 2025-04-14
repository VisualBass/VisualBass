#include "ColorPicker.h"
#include "../globals.h"
#include <cstdio>  // Include for sprintf
#include "GetColorFromHue.h"  // Include the header where GetColorFromHue is defined

// Constructor: Initialize hue options
ColorPicker::ColorPicker() {
    for (int i = 0; i < NUM_COLOR_OPTIONS; i++) {
        hueOptions[i] = i * 15;  // Generate hues from 0 to 360
    }
    selectedHue = -1.0f; // Default selected hue
    isSelectingColor = false;
}

void ColorPicker::DrawColorPicker(Rectangle startArea, float& hueShift) {
    const int columns = 8;
    const int boxSize = 30;
    const int spacing = 10;

    for (int i = 0; i < NUM_COLOR_OPTIONS; i++) {
        int row = i / columns;
        int col = i % columns;
        Rectangle colorBox = {
            startArea.x + col * (boxSize + spacing),
            startArea.y + row * (boxSize + spacing),
            (float)boxSize,
            (float)boxSize
        };
        int optionHue = hueOptions[i];
        Color displayColor = GetColorFromHue(optionHue);  // Get the color for the selected hue
        DrawRectangleRec(colorBox, displayColor);

        char label[8];
        sprintf(label, "%d", optionHue);  // Format the hue as a string
        DrawText(label, (int)(colorBox.x + 2), (int)(colorBox.y + 7), 10, WHITE);

        // Only update hueShift on mouse click (not hover)
        if (CheckCollisionPointRec(GetMousePosition(), colorBox)) {
            DrawRectangleLinesEx(colorBox, 2, WHITE);  // Highlight the hovered color box

            // Check for mouse click and update hueShift
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                // If the selected color is 0, toggle autoCycleHue
                if (optionHue == 0) {
                    if (autoCycleHue) {
                        autoCycleHue = false;  // Stop cycling if it was already on
                    } else {
                        autoCycleHue = true;   // Start cycling from the current color
                    }
                } else {
                    // For any other color, stop cycling and set the hueShift
                    autoCycleHue = false;  // Stop cycling when a non-zero color is selected
                    hueShift = (float)optionHue;  // Set hueShift to the selected color's hue
                }
            }
        }
    }
}



void ColorPicker::UpdateColorSelection(float& hueShift) {
    if (selectedHue != -1.0f) {
        hueShift = selectedHue;  // Update hueShift based on the selected hue
    }
}
