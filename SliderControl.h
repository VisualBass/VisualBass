#ifndef SLIDERCONTROL_H
#define SLIDERCONTROL_H

#include "raylib.h"
#include <string>

class SliderControl {
public:
    // --- CONSTRUCTORS ---

    // 1. Existing Integer Constructor (For Orbs)
    SliderControl(int min, int max, int& var, const char* text, float& hue);

    // 2. NEW Float Constructor (For Brightness)
    SliderControl(float min, float max, float& var, const char* text, float& hue);

    // --- METHODS ---
    void DrawSlider(Rectangle startArea);
    void UpdateSlider();
    void SetStepPercent(float pct) { stepPercent = (pct <= 0.f) ? 0.05f : pct; }
    static Color InvertColor(Color color);

private:
    // We use an Enum to know which mode we are in
    enum SliderType { INT_MODE, FLOAT_MODE };
    SliderType type;

    // Pointers to the external variables (Only one of these will be used at a time)
    int* intValPtr = nullptr;
    float* floatValPtr = nullptr;

    // Limits
    int minInt, maxInt;
    float minFloat, maxFloat;

    // Common data
    const char* label;
    float& hueShiftRef;
    Rectangle sliderArea{};
    float stepPercent = 0.05f;
};

#endif // SLIDERCONTROL_H