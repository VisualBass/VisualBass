#include "Debug.h"
#include <cstdio> // For TextFormat
#include "../globals.h" // Added to access 'hueSpeed'

DebugMenu::DebugMenu(float& globalHue)
    : hueRef(globalHue),
      isDebugOverlayVisible(false),
      debugToggle(isDebugOverlayVisible, "Enable Debug Overlay", globalHue)
{
}

float DebugMenu::Draw(float offsetX, float offsetY, float width, float uiScale, float alpha) {
    float startY = offsetY;

    // Layout Constants
    float padding = 20.0f * uiScale;
    float rowHeight = 30.0f * uiScale;
    float lineHeight = 20.0f * uiScale;

    // Define Button Rect (Full width minus padding)
    Rectangle btnRect = { offsetX + padding, offsetY, width - (padding * 2), rowHeight };

    // 1. Draw the Toggle Button
    debugToggle.Draw(btnRect, uiScale);
    offsetY += rowHeight;

    // 2. Draw the Info Overlay (Directly inside the menu list)
    if (isDebugOverlayVisible) {
        float textY = offsetY + (10.0f * uiScale);
        int fontSize = (int)(16 * uiScale);
        Color textColor = GREEN; // Or use GetColorFromHue((int)hueRef) for theming

        // FPS
        DrawText(TextFormat("FPS: %i", GetFPS()), (int)(offsetX + padding), (int)textY, fontSize, textColor);
        textY += lineHeight;

        // Hue Value
        DrawText(TextFormat("Hue Shift: %.2f", hueRef), (int)(offsetX + padding), (int)textY, fontSize, textColor);
        textY += lineHeight;

        // Hue Speed
        // Updated to show the live global variable 'hueSpeed'
        DrawText(TextFormat("Hue Speed: %.1f/sec", hueSpeed), (int)(offsetX + padding), (int)textY, fontSize, textColor);
        textY += lineHeight;

        offsetY = textY + (10.0f * uiScale); // Update consumed height
    }

    return offsetY - startY;
}