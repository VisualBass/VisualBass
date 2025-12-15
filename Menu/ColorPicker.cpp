#include "ColorPicker.h"
#include "../globals.h"
#include <cstdio>
#include "GetColorFromHue.h"
#include <cmath> // for ceil

ColorPicker::ColorPicker() {
    for (int i = 0; i < NUM_COLOR_OPTIONS; i++) {
        hueOptions[i] = i * 15;
    }
    selectedHue = -1.0f;
    isSelectingColor = false;
}

// UPDATED: Now uses scale to resize the boxes and grid
int ColorPicker::DrawColorPicker(Rectangle startArea, float& hueShift, float scale) {

    const int boxSize = (int)(30 * scale);
    const int spacing = (int)(10 * scale);
    const int fontSize = (int)(10 * scale);
    const int maxColumns = 8;

    // 1. Calculate Columns based on available width
    int itemWidth = boxSize + spacing;
    int availableWidth = (int)startArea.width;

    // Ensure at least 1 column
    int columns = (availableWidth + spacing) / itemWidth;
    if (columns > maxColumns) columns = maxColumns;
    if (columns < 1) columns = 1;

    // 2. Draw the Grid
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
        Color displayColor = GetColorFromHue(optionHue);
        DrawRectangleRec(colorBox, displayColor);

        char label[8];
        sprintf(label, "%d", optionHue);
        // Scaled offsets
        DrawText(label, (int)(colorBox.x + 2 * scale), (int)(colorBox.y + 7 * scale), fontSize, WHITE);

        if (CheckCollisionPointRec(GetMousePosition(), colorBox)) {
            DrawRectangleLinesEx(colorBox, 2 * scale, WHITE);

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (optionHue == 0) {
                    autoCycleHue = !autoCycleHue;
                } else {
                    autoCycleHue = false;
                    hueShift = (float)optionHue;
                }
            }
        }
    }

    // 3. Calculate and Return Total Height
    // Total rows needed = Total Options / Columns (rounded up)
    int rows = (NUM_COLOR_OPTIONS + columns - 1) / columns;
    int totalHeight = rows * (boxSize + spacing);

    return totalHeight;
}

void ColorPicker::UpdateColorSelection(float& hueShift) {
    if (selectedHue != -1.0f) {
        hueShift = selectedHue;
    }
}