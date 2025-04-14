#ifndef COLORPICKER_H
#define COLORPICKER_H

#include "raylib.h"

// Number of color options for the picker
#define NUM_COLOR_OPTIONS 24

// Define a ColorPicker class to manage color selection
class ColorPicker {
public:
    ColorPicker(); // Constructor
    void DrawColorPicker(Rectangle startArea, float& hueShift); // Draw the color picker grid
    void UpdateColorSelection(float& hueShift); // Handle color selection logic and update hueShift

private:
    int hueOptions[NUM_COLOR_OPTIONS]; // Array for the hue options
    bool isSelectingColor; // Flag for selecting a color
    float selectedHue; // Selected hue for color picker
};

#endif // COLORPICKER_H
