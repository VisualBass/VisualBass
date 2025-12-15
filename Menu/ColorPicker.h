#ifndef COLORPICKER_H
#define COLORPICKER_H

#include "raylib.h"

#define NUM_COLOR_OPTIONS 24

class ColorPicker {
public:
    ColorPicker();
    // UPDATED: Now accepts a scale factor to resize grid elements
    int DrawColorPicker(Rectangle startArea, float& hueShift, float scale);
    void UpdateColorSelection(float& hueShift);

private:
    int hueOptions[NUM_COLOR_OPTIONS];
    bool isSelectingColor;
    float selectedHue;
};

#endif