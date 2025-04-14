// GetColorFromHue.cpp

#include "GetColorFromHue.h"
#include "raylib.h"
#include <cmath>

// Helper function to convert hue to a Color
Color GetColorFromHue(int hue) {
    if (hue == 0) return DARKGRAY;
    float normalizedH = fmod(static_cast<float>(hue), 360.0f) / 360.0f;
    float r, g, b;
    int i = (int)(normalizedH * 6.0f);
    float f = normalizedH * 6.0f - i;
    float v = 1.0f, s = 1.0f;
    float p = v * (1.0f - s), q = v * (1.0f - f * s), t = v * (1.0f - (1.0f - f) * s);
    switch(i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
        default: r = g = b = 0; break;
    }
    return (Color){ (unsigned char)(r * 255), (unsigned char)(g * 255), (unsigned char)(b * 255), 255 };
}
