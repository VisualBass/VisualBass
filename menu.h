#ifndef MENU_H
#define MENU_H

#include "raylib.h"
#include "globals.h"
#include <cstdio>
#include "menu/ColorPicker.h"
#include "menu/OrbControl.h"  // Include the header file for OrbControl

class Menu {
public:
    Menu();

    void Update();
    void Draw();
    bool IsVisible() const;
    void UpdateColorSelection();  // Declare the method here

private:
    bool visible;             // Whether the menu is open/active.
    float backgroundAlpha;    // For fade in/out.
    bool isMouseHovering;     // Whether the mouse is over the menu area.

    ColorPicker colorPicker;  // Declare colorPicker as an instance of ColorPicker class
    OrbControl orbControl;  // OrbControl instance

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
};

#endif
