#ifndef SLIDERCONTROL_H
#define SLIDERCONTROL_H

#include "raylib.h"
#include <string>

class SliderControl {
public:
    // Constructor for int reference (e.g., numOrbs)
    SliderControl(int minValue, int maxValue, int& value, const char* label, float& hueShift);

    // Draws the slider; stores the drawn area so input matches visuals
    void DrawSlider(Rectangle startArea);

    // Updates the slider value (mouse wheel over the slider = +/-5% steps; click on track sets value)
    void UpdateSlider();

    // Optional: change the wheel step percent (default 0.05f = 5%)
    void SetStepPercent(float pct) { stepPercent = (pct <= 0.f) ? 0.05f : pct; }

    // Method to invert color (for hover effect)
    static Color InvertColor(Color color);

private:
    int minValue;          // Minimum value of the slider
    int maxValue;          // Maximum value of the slider
    int& value;            // Reference to the value that the slider controls
    const char* label;     // Label to be displayed on the slider
    float& hueShift;       // Reference to hueShift value

    Rectangle sliderArea{}; // The actual area used by the last DrawSlider call
    float stepPercent = 0.05f; // Mouse wheel step as a fraction of range (5% by default)
};

#endif // SLIDERCONTROL_H
