#ifndef DEBUG_MENU_H
#define DEBUG_MENU_H

#include "raylib.h"
#include "../ToggleControl.h"

class DebugMenu {
public:
    DebugMenu(float& globalHue);

    // Draws the debug controls (toggle button) inside the scrollable menu area.
    float Draw(float offsetX, float offsetY, float width, float uiScale, float alpha);

    // Getter if main.cpp needs to know status
    bool IsDebugActive() const { return isDebugOverlayVisible; }

private:
    float& hueRef;
    bool isDebugOverlayVisible;

    // Controls
    ToggleControl debugToggle;
};

#endif