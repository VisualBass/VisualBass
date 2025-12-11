#ifndef ORBCONTROL_H
#define ORBCONTROL_H

#include "raylib.h"
#include "./globals.h"   // must declare: extern Orb* orbs; extern int orbCount; extern float hueShift;
#include <cstdio>

class OrbControl {
public:
    // Build with the allowed range
    OrbControl(int min, int max);

    // Draw at startArea.x, startArea.y. If width/height are zero, defaults are used.
    void DrawOrbSlider(Rectangle startArea);

    // Update orbCount from mouse (wheel = +/-5% per notch, left drag sets position)
    void UpdateOrbSlider();

    // Optional: change the wheel step percent (default 0.05 = 5%)
    void SetStepPercent(float pct) { stepPercent = (pct <= 0.f) ? 0.05f : pct; }

private:
    int  minOrbs;
    int  maxOrbs;

    Rectangle sliderArea{};   // last drawn area for hit-testing
    float     stepPercent = 0.05f; // 5% per wheel notch
};

#endif // ORBCONTROL_H
