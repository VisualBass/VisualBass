#include "Menu.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include "rlgl.h"
#include "menu/ColorPicker.h"
#include "menu/GetColorFromHue.h"
#include "globals.h"
#include "Menu/OrbControl.h"

// Declare min and max orbs
int minOrbs = 1;
int maxOrbs = 100;

// Now pass them to the OrbControl constructor
OrbControl orbControl(minOrbs, maxOrbs);  // Make sure this constructor is used

Menu::Menu()
    : visible(false),
      backgroundAlpha(MIN_ALPHA),
      isMouseHovering(false),
      editingHue(false),
      scrollOffset(0.0f),
      orbControl(minOrbs, maxOrbs)  // Initialize orbControl with min and max values
{
    hueBuffer[0] = '\0';
}


bool Menu::IsVisible() const {
    return visible;
}

Rectangle Menu::GetMenuBounds() {
    const float margin = 20.0f;
    float width  = GetScreenWidth() * MENU_WIDTH_RATIO;
    float height = GetScreenHeight() * MENU_HEIGHT_RATIO;
    return (Rectangle){ margin, (float)GetScreenHeight() - height - margin, width, height };
}

// In your menu draw method, update hueShift when necessary:
float Menu::DrawMenuContent(float offsetX, float offsetY) {
    float startY = offsetY; // We'll track how tall the entire content is.

    // SETTINGS title
    DrawText("SETTINGS", (int)offsetX + 20, (int)offsetY + 20, 24, Fade(RAYWHITE, backgroundAlpha));
    offsetY += 60; // space below the title

    // ** Hue field with the current hue value as two boxes **
    Rectangle hueField = { offsetX + 20, offsetY, (float)(GetMenuBounds().width - 40), 30 };
    offsetY += 30 + 20; // move down by 30 + some spacing

    // Hue field background color
    Color hueFieldColor = editingHue ? LIGHTGRAY : GetColorFromHue((int)hueShift);
    bool hoverHue = CheckCollisionPointRec(GetMousePosition(), hueField);
    if (hoverHue && !editingHue) hueFieldColor = SKYBLUE;
    DrawRectangleRec(hueField, Fade(hueFieldColor, backgroundAlpha));

    // If user clicks the field, start editing
    if (!editingHue && hoverHue && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        editingHue = true;
        sprintf(hueBuffer, "%.0f", hueShift);
    }

    // Draw text in the hue field
    if (editingHue) {
        UpdateTextInput();
        DrawText(hueBuffer, (int)(hueField.x + 10), (int)(hueField.y + 5), 20, Fade(WHITE, backgroundAlpha));
        if (((int)(GetTime()*2)) % 2) {
            int len = (int)strlen(hueBuffer);
            float textWidth = MeasureText(hueBuffer, 20);
            DrawText("|", (int)(hueField.x + 10 + textWidth), (int)(hueField.y + 5), 20, Fade(WHITE, backgroundAlpha));
        }
    } else {
        char hueText[32];
        if (autoCycleHue)  sprintf(hueText, "Hue: Cycle");
        else              sprintf(hueText, "Hue: %.0f°", hueShift);
        DrawText(hueText, (int)(hueField.x + 10), (int)(hueField.y + 5), 20, Fade(WHITE, backgroundAlpha));
    }

    // Color Picker
    Rectangle pickerArea = {
        offsetX + 20,
        offsetY,
        200,
        100
    };
    colorPicker.DrawColorPicker(pickerArea, hueShift);  // Pass the hueShift as the second argument
    offsetY += 100 + 20; // After color picker, add spacing

    // After updating the color picker, update hueShift
    colorPicker.UpdateColorSelection(hueShift);

    // ** New Field Header (reflect the hueShift color) **
    Rectangle newFieldHeader = { offsetX + 20, offsetY, (float)(GetMenuBounds().width - 40), 30 };
    Color headerColor = GetColorFromHue((int)hueShift);  // Use hueShift to set header color
    DrawRectangleRec(newFieldHeader, Fade(headerColor, backgroundAlpha));
    DrawText("Orb Slider", (int)(newFieldHeader.x + 10), (int)(newFieldHeader.y + 5), 20, Fade(WHITE, backgroundAlpha));
    offsetY += 30 + 10; // move down

    // In Menu.cpp where you instantiate OrbControl:
    orbControl = OrbControl(minOrbs, maxOrbs);

    Rectangle orbSliderArea = { offsetX + 20, offsetY, 400, 30 };  // Position for the orb slider
    orbControl.DrawOrbSlider(orbSliderArea);  // Draw the orb slider
    orbControl.UpdateOrbSlider();  // Update the number of orbs based on slider
    offsetY += 50;  // Adjust for the next field

    return offsetY - startY;
}


void Menu::UpdateTextInput() {
    int key = GetCharPressed();
    while(key > 0) {
        if (key >= '0' && key <= '9') {
            int len = (int)strlen(hueBuffer);
            if (len < 15) {
                hueBuffer[len] = (char)key;
                hueBuffer[len+1] = '\0';
            }
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && strlen(hueBuffer) > 0) {
        int len = (int)strlen(hueBuffer);
        hueBuffer[len-1] = '\0';
    }
    if (IsKeyPressed(KEY_ENTER)) {
        float value = atof(hueBuffer);
        if (value > 360.0f) value = 360.0f;
        hueShift = value;
        editingHue = false;
        autoCycleHue = (value == 0.0f);
        hueBuffer[0] = '\0';
    }
}

void Menu::Draw() {
    if (!visible && backgroundAlpha <= MIN_ALPHA + 0.01f) return;
    Rectangle bounds = GetMenuBounds();
    // Draw the menu background
    DrawRectangleRounded(bounds, 0.15f, 12, Fade(DARKGRAY, backgroundAlpha));

    // Begin scissor mode: clip drawing to the menu's bounding rectangle
    BeginScissorMode((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height);

    // Shift everything upward by scrollOffset
    rlPushMatrix();
    rlTranslatef(0.0f, scrollOffset, 0.0f);

    totalContentHeight = DrawMenuContent(bounds.x, bounds.y);

    rlPopMatrix();
    EndScissorMode();
}

void Menu::Update() {
    Rectangle bounds = GetMenuBounds();
    isMouseHovering = CheckCollisionPointRec(GetMousePosition(), bounds);

    // Fade effect when hovering
    if (isMouseHovering) {
        backgroundAlpha += FADE_IN_SPEED;
        if (backgroundAlpha > MAX_ALPHA) backgroundAlpha = MAX_ALPHA;
    } else {
        backgroundAlpha -= FADE_OUT_SPEED;
        if (backgroundAlpha < MIN_ALPHA) backgroundAlpha = MIN_ALPHA;
    }

    // Scroll handling with the mouse wheel
    if (isMouseHovering) {
        float wheelMove = GetMouseWheelMove(); // How many steps (could be ±1 for each wheel tick)
        if (fabs(wheelMove) > 0.001f) {
            scrollOffset += wheelMove * 15.0f;
        }
    }

    // Clamp scrollOffset so we can't scroll past the content
    float menuHeight = bounds.height;
    float maxScroll = 0;
    float minScroll = menuHeight - totalContentHeight;
    if (totalContentHeight <= menuHeight) {
        scrollOffset = 0;
    } else {
        if (scrollOffset > maxScroll) scrollOffset = maxScroll;
        if (scrollOffset < minScroll) scrollOffset = minScroll;
    }

    // Toggle menu visibility with TAB key (optional)
    if (IsKeyPressed(KEY_TAB)) {
        visible = !visible;
        if (!visible) editingHue = false;

    }
}
