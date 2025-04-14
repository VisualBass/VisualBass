#ifndef SLIDERCONTROL_H
#define SLIDERCONTROL_H

#include "raylib.h"
#include <string>

class SliderControl {
public:
    // Constructor for int reference (e.g., numOrbs)
    SliderControl(int minValue, int maxValue, int& value, const char* label, float& hueShift);

    // Method to draw the slider with color reflecting hueShift
    void DrawSlider(Rectangle startArea);

    // Method to update the slider value based on mouse input
    void UpdateSlider();

    // Method to invert color (for hover effect)
    static Color InvertColor(Color color);

private:
    int minValue;  // Minimum value of the slider
    int maxValue;  // Maximum value of the slider
    int& value;    // Reference to the value that the slider controls
    const char* label;  // Label to be displayed on the slider
    float& hueShift;  // Reference to hueShift value
};

#endif // SLIDERCONTROL_H
