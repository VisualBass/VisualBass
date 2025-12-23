#ifndef MENU_H
#define MENU_H

#include "raylib.h"
#include "globals.h"
#include <cstdio>
#include "menu/ColorPicker.h"
#include "SliderControl.h"
#include "menu/Debug.h"

class Menu {
public:
    Menu();

    bool IsVisible() const;
    Rectangle GetMenuBounds();
    Vector2 GetLocalMousePos();
    float DrawMenuContent(float offsetX, float offsetY, Rectangle visibleArea, int currentMode, float uiScale);

    void UpdateTextInput();
    void Draw(int currentMode);
    void Update();

private:
    bool visible;
    float backgroundAlpha;
    bool isMouseHovering;

    ColorPicker colorPicker;

    // Sliders
    SliderControl orbSlider;
    SliderControl brightnessSlider;
    SliderControl gravitySlider;
    SliderControl sizeSlider;
    SliderControl repelSlider;

    // Cube Sliders
    SliderControl cubeSpeedSlider;
    SliderControl cubePumpSlider;

    // Global Sliders
    SliderControl hueSpeedSlider;
    SliderControl globalPumpSlider;

    DebugMenu debugMenu;

    bool lifxConnected;
    bool editingHue;
    char hueBuffer[16];

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