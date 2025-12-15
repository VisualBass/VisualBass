#ifndef TOGGLECONTROL_H
#define TOGGLECONTROL_H

#include "raylib.h"

class ToggleControl {
public:
    // Constructor
    ToggleControl(bool& targetValue, const char* labelText, float& globalHue);

    // Draws the button
    void Draw(Rectangle bounds, float uiScale);

private:
    bool* targetRef;
    const char* label;
    float& hueRef;
};

#endif