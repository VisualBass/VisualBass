#ifndef MENU_H
#define MENU_H

#include "raylib.h"
#include "globals.h"
#include <cstdio>
<<<<<<< HEAD
#include "menu/ColorPicker.h"
#include "menu/OrbControl.h"  // Include the header file for OrbControl
=======
>>>>>>> e3acf683c19700d64f5c0b8033b00208fd966701

class Menu {
public:
    Menu();

    void Update();
    void Draw();
    bool IsVisible() const;
<<<<<<< HEAD
    void UpdateColorSelection();  // Declare the method here
=======
>>>>>>> e3acf683c19700d64f5c0b8033b00208fd966701

private:
    bool visible;             // Whether the menu is open/active.
    float backgroundAlpha;    // For fade in/out.
    bool isMouseHovering;     // Whether the mouse is over the menu area.

<<<<<<< HEAD
    ColorPicker colorPicker;  // Declare colorPicker as an instance of ColorPicker class
    OrbControl orbControl;  // OrbControl instance

=======
>>>>>>> e3acf683c19700d64f5c0b8033b00208fd966701
    // For manual hue input:
    bool editingHue;
    char hueBuffer[16];

    // For scrolling:
    float scrollOffset;       // How far we have scrolled
    float totalContentHeight; // How tall the menu's content is

    // Add this line:
    float DrawMenuContent(float offsetX, float offsetY);

    Rectangle GetMenuBounds();

    // Color picker
    void DrawColorPicker(Rectangle startArea);
    void UpdateTextInput();

    static constexpr float MENU_WIDTH_RATIO  = 0.4f;
    static constexpr float MENU_HEIGHT_RATIO = 0.4f;
    static constexpr float FADE_IN_SPEED     = 0.08f;
    static constexpr float FADE_OUT_SPEED    = 0.03f;
    static constexpr float MAX_ALPHA         = 0.85f;
    static constexpr float MIN_ALPHA         = 0.1f;
<<<<<<< HEAD
=======

    static constexpr int NUM_COLOR_OPTIONS   = 8;
    int hueOptions[NUM_COLOR_OPTIONS] = {0, 1, 30, 60, 120, 240, 270, 330 };
>>>>>>> e3acf683c19700d64f5c0b8033b00208fd966701
};

#endif
