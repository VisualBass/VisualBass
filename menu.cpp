#include "Menu.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include "rlgl.h"
<<<<<<< HEAD
#include "menu/ColorPicker.h"
#include "menu/GetColorFromHue.h"
#include "globals.h"
#include "Menu/OrbControl.h"

// Declare min and max orbs
int minOrbs = 1;
int maxOrbs = 100;

// Now pass them to the OrbControl constructor
OrbControl orbControl(minOrbs, maxOrbs);  // Make sure this constructor is used
=======

static Color GetColorFromHue(int hue) {
    if (hue == 0) return DARKGRAY;
    float normalizedH = fmod(static_cast<float>(hue), 360.0f) / 360.0f;
    float r, g, b;
    int i = (int)(normalizedH * 6.0f);
    float f = normalizedH * 6.0f - i;
    float v = 1.0f, s = 1.0f;
    float p = v*(1.0f - s), q = v*(1.0f - f*s), t = v*(1.0f - (1.0f - f)*s);
    switch(i % 6) {
        case 0: r=v; g=t; b=p; break;
        case 1: r=q; g=v; b=p; break;
        case 2: r=p; g=v; b=t; break;
        case 3: r=p; g=q; b=v; break;
        case 4: r=t; g=p; b=v; break;
        case 5: r=v; g=p; b=q; break;
        default: r=g=b=0; break;
    }
    return (Color){ (unsigned char)(r*255), (unsigned char)(g*255), (unsigned char)(b*255), 255 };
}
>>>>>>> e3acf683c19700d64f5c0b8033b00208fd966701

Menu::Menu()
    : visible(false),
      backgroundAlpha(MIN_ALPHA),
      isMouseHovering(false),
      editingHue(false),
      scrollOffset(0.0f),
<<<<<<< HEAD
      orbControl(minOrbs, maxOrbs)  // Initialize orbControl with min and max values
=======
      totalContentHeight(0.0f)
>>>>>>> e3acf683c19700d64f5c0b8033b00208fd966701
{
    hueBuffer[0] = '\0';
}

<<<<<<< HEAD

=======
>>>>>>> e3acf683c19700d64f5c0b8033b00208fd966701
bool Menu::IsVisible() const {
    return visible;
}

<<<<<<< HEAD
=======
// The bounding rectangle for the entire menu.
>>>>>>> e3acf683c19700d64f5c0b8033b00208fd966701
Rectangle Menu::GetMenuBounds() {
    const float margin = 20.0f;
    float width  = GetScreenWidth() * MENU_WIDTH_RATIO;
    float height = GetScreenHeight() * MENU_HEIGHT_RATIO;
    return (Rectangle){ margin, (float)GetScreenHeight() - height - margin, width, height };
}

<<<<<<< HEAD
// In your menu draw method, update hueShift when necessary:
=======
// A helper that draws all menu fields (including color picker, new header, etc.)
// We pass an (x, y) offset so we can shift the content by scrollOffset.
>>>>>>> e3acf683c19700d64f5c0b8033b00208fd966701
float Menu::DrawMenuContent(float offsetX, float offsetY) {
    float startY = offsetY; // We'll track how tall the entire content is.

    // SETTINGS title
    DrawText("SETTINGS", (int)offsetX + 20, (int)offsetY + 20, 24, Fade(RAYWHITE, backgroundAlpha));
    offsetY += 60; // space below the title

<<<<<<< HEAD
    // ** Hue field with the current hue value as two boxes **
=======
    // Hue Field
>>>>>>> e3acf683c19700d64f5c0b8033b00208fd966701
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
<<<<<<< HEAD
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


=======
    DrawColorPicker(pickerArea);
    offsetY += 100 + 20; // after color picker, add spacing

    // New Field Header
    Rectangle newFieldHeader = {
        offsetX + 20,
        offsetY,
        (float)(GetMenuBounds().width - 40),
        30
    };
    Color headerColor = GetColorFromHue((int)hueShift);
    DrawRectangleRec(newFieldHeader, Fade(headerColor, backgroundAlpha));
    DrawText("Coming Soon", (int)(newFieldHeader.x + 10), (int)(newFieldHeader.y + 5), 20, Fade(WHITE, backgroundAlpha));
    offsetY += 30 + 10; // move down

    // Possibly more content below...
    // e.g. let's draw 3 more dummy fields
    for (int i = 1; i <= 3; i++) {
        Rectangle dummyField = {
            offsetX + 20,
            offsetY,
            (float)(GetMenuBounds().width - 40),
            30
        };
        DrawRectangleRec(dummyField, Fade(DARKGRAY, backgroundAlpha));
        DrawText(TextFormat("Extra Field #%d", i), (int)(dummyField.x + 10), (int)(dummyField.y + 5), 20, Fade(WHITE, backgroundAlpha));
        offsetY += 30 + 10;
    }

    // The total content height is offsetY minus the startY
    return offsetY - startY;
}

void Menu::DrawColorPicker(Rectangle startArea) {
    const int columns = 4;
    const int boxSize = 30;
    const int spacing = 10;
    const int rows = NUM_COLOR_OPTIONS / columns + (NUM_COLOR_OPTIONS % columns ? 1 : 0);

    for (int i = 0; i < NUM_COLOR_OPTIONS; i++) {
        int row = i / columns;
        int col = i % columns;
        Rectangle colorBox = {
            startArea.x + col*(boxSize + spacing),
            startArea.y + row*(boxSize + spacing),
            (float)boxSize,
            (float)boxSize
        };
        int optionHue = hueOptions[i];
        Color displayColor = GetColorFromHue(optionHue);
        DrawRectangleRec(colorBox, displayColor);
        char label[8];
        sprintf(label, "%d", optionHue);
        DrawText(label, (int)(colorBox.x + 2), (int)(colorBox.y + 7), 10, WHITE);

        // Interaction
        if (CheckCollisionPointRec(GetMousePosition(), colorBox)) {
            DrawRectangleLinesEx(colorBox, 2, WHITE);
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (optionHue == 0) {
                    autoCycleHue = true;
                } else {
                    hueShift = (float)optionHue;
                    autoCycleHue = false;
                }
            }
        }
    }
}

// Text input logic is the same as your original
>>>>>>> e3acf683c19700d64f5c0b8033b00208fd966701
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
<<<<<<< HEAD
=======
        if (value < 0.0f) value = 0.0f;
>>>>>>> e3acf683c19700d64f5c0b8033b00208fd966701
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
<<<<<<< HEAD
    rlPushMatrix();
    rlTranslatef(0.0f, scrollOffset, 0.0f);

=======
    // We'll do a quick push to the Raylib transform stack
    // Alternatively, you can do an explicit Y offset in DrawMenuContent
    rlPushMatrix();
    rlTranslatef(0.0f, scrollOffset, 0.0f);

    // Actually draw the menu contents. This returns how tall it was
>>>>>>> e3acf683c19700d64f5c0b8033b00208fd966701
    totalContentHeight = DrawMenuContent(bounds.x, bounds.y);

    rlPopMatrix();
    EndScissorMode();
<<<<<<< HEAD
=======

    // Optionally draw a visual scroll bar or up/down arrow here
>>>>>>> e3acf683c19700d64f5c0b8033b00208fd966701
}

void Menu::Update() {
    Rectangle bounds = GetMenuBounds();
    isMouseHovering = CheckCollisionPointRec(GetMousePosition(), bounds);

<<<<<<< HEAD
    // Fade effect when hovering
=======
    // Fade effect
>>>>>>> e3acf683c19700d64f5c0b8033b00208fd966701
    if (isMouseHovering) {
        backgroundAlpha += FADE_IN_SPEED;
        if (backgroundAlpha > MAX_ALPHA) backgroundAlpha = MAX_ALPHA;
    } else {
        backgroundAlpha -= FADE_OUT_SPEED;
        if (backgroundAlpha < MIN_ALPHA) backgroundAlpha = MIN_ALPHA;
    }

<<<<<<< HEAD
    // Scroll handling with the mouse wheel
    if (isMouseHovering) {
        float wheelMove = GetMouseWheelMove(); // How many steps (could be ±1 for each wheel tick)
        if (fabs(wheelMove) > 0.001f) {
            scrollOffset += wheelMove * 15.0f;
        }
    }

    // Clamp scrollOffset so we can't scroll past the content
    float menuHeight = bounds.height;
=======
    // Toggle menu with TAB
    if (IsKeyPressed(KEY_TAB)) {
        visible = !visible;
        if (!visible) editingHue = false;
    }

    // Only scroll if we're hovering the menu
    if (visible && isMouseHovering) {
        float wheelMove = GetMouseWheelMove(); // how many steps (could be ±1 for each wheel tick)
        if (fabs(wheelMove) > 0.001f) {
            // Adjust offset by e.g. 15 pixels per step
            scrollOffset += wheelMove * 15;
        }
    }

    // We clamp scrollOffset so we can't scroll past the content
    float menuHeight = bounds.height;
    // The content starts at bounds.y; if totalContentHeight < menuHeight, no scrolling is needed
>>>>>>> e3acf683c19700d64f5c0b8033b00208fd966701
    float maxScroll = 0;
    float minScroll = menuHeight - totalContentHeight;
    if (totalContentHeight <= menuHeight) {
        scrollOffset = 0;
    } else {
        if (scrollOffset > maxScroll) scrollOffset = maxScroll;
        if (scrollOffset < minScroll) scrollOffset = minScroll;
    }

<<<<<<< HEAD
    // Toggle menu visibility with TAB key (optional)
    if (IsKeyPressed(KEY_TAB)) {
        visible = !visible;
        if (!visible) editingHue = false;

=======
    // If content fits in the area, no scroll needed
    if (totalContentHeight <= menuHeight) {
        scrollOffset = 0;
    } else {
        // Otherwise clamp
        if (scrollOffset > maxScroll) scrollOffset = maxScroll;
        if (scrollOffset < minScroll) scrollOffset = minScroll;
>>>>>>> e3acf683c19700d64f5c0b8033b00208fd966701
    }
}
