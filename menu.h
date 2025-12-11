#ifndef MENU_H
#define MENU_H

#include "raylib.h"
#include "globals.h"
#include <cstdio>
#include "menu/ColorPicker.h"
#include "SliderControl.h"

class Menu {
public:
    Menu();

    bool IsVisible() const;
    Rectangle GetMenuBounds();
    Vector2 GetLocalMousePos();

    // Draws the content and returns the total height
    float DrawMenuContent(float offsetX, float offsetY, int currentMode);

    void UpdateTextInput();

    // Main draw function that sets up scissor mode and scrolling
    void Draw(int currentMode);

    void Update();

private:
    bool visible;
    float backgroundAlpha;
    bool isMouseHovering;

    ColorPicker colorPicker;

    // --- SLIDERS ---
    // Generic Slider Control
    SliderControl orbSlider;
    SliderControl brightnessSlider; // <--- ADD THIS LINE

    // Smart Home State
    bool lifxConnected;

    // For manual hue input:
    bool editingHue;
    char hueBuffer[16];

    // For scrolling:
    float scrollOffset;
    float totalContentHeight;

    static constexpr float MENU_WIDTH_RATIO  = 0.4f;
    static constexpr float MENU_HEIGHT_RATIO = 0.4f;
    static constexpr float FADE_IN_SPEED     = 0.08f;
    static constexpr float FADE_OUT_SPEED    = 0.03f;
    static constexpr float MAX_ALPHA         = 0.85f;
    static constexpr float MIN_ALPHA         = 0.1f;
};

#endif