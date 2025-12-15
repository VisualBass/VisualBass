#ifndef SLIDERCONTROL_H
#define SLIDERCONTROL_H

#include "raylib.h"

class SliderControl {
public:
    // Constructor (Int)
    SliderControl(int min, int max, int& targetValue, int defaultValue, const char* labelText, float& globalHue);

    // Constructor (Float)
    SliderControl(float min, float max, float& targetValue, float defaultValue, const char* labelText, float& globalHue);

    void UpdateSlider();

    // UPDATED: Added scale parameter for font resizing
    void DrawSlider(Rectangle bounds, Rectangle clipRegion, float scale);

private:
    enum SliderType { SLIDER_INT, SLIDER_FLOAT };

    SliderType type;
    void* targetRef;
    float minVal;
    float maxVal;
    float defaultVal;
    const char* label;
    float& hueRef;
    bool isDragging;

    // --- NEW: Text Input State ---
    bool isEditing;       // Are we currently typing a number?
    char textBuffer[32];  // Buffer for the text being typed
    int cursorBlinkFrame; // To make the cursor blink

    float GetNormalizedValue() const;
    void SetValueFromNormalized(float t);

    // Helper to validate and apply the typed text
    void ApplyTextBuffer();
};

#endif