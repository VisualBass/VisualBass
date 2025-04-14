#ifndef ORBCONTROL_H
#define ORBCONTROL_H

#include "raylib.h"
#include "../globals.h"
#include <cstdio>

// OrbControl class to manage orb slider and logic
class OrbControl {
public:
    // Constructor initializes minOrbs, maxOrbs, and numOrbs
    OrbControl(int min, int max);

    // Method to draw the orb slider
    void DrawOrbSlider(Rectangle startArea);

    // Method to update orb count based on the slider interaction
    void UpdateOrbSlider();

private:
    int numOrbs;  // Current number of orbs
    int minOrbs;  // Minimum number of orbs
    int maxOrbs;  // Maximum number of orbs

    // Dragging-related variables
    bool dragging;  // Flag for tracking mouse dragging state
    float dragStartX;  // To store the starting mouse X position during a drag
};

#endif // ORBCONTROL_H
