#include "Menu.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include "rlgl.h"

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

Menu::Menu()
    : visible(false),
      backgroundAlpha(MIN_ALPHA),
      isMouseHovering(false),
      editingHue(false),
      scrollOffset(0.0f),
      totalContentHeight(0.0f)
{
    hueBuffer[0] = '\0';
}

bool Menu::IsVisible() const {
    return visible;
}

// The bounding rectangle for the entire menu.
Rectangle Menu::GetMenuBounds() {
    const float margin = 20.0f;
    float width  = GetScreenWidth() * MENU_WIDTH_RATIO;
    float height = GetScreenHeight() * MENU_HEIGHT_RATIO;
    return (Rectangle){ margin, (float)GetScreenHeight() - height - margin, width, height };
}

// A helper that draws all menu fields (including color picker, new header, etc.)
// We pass an (x, y) offset so we can shift the content by scrollOffset.
float Menu::DrawMenuContent(float offsetX, float offsetY) {
    float startY = offsetY; // We'll track how tall the entire content is.

    // SETTINGS title
    DrawText("SETTINGS", (int)offsetX + 20, (int)offsetY + 20, 24, Fade(RAYWHITE, backgroundAlpha));
    offsetY += 60; // space below the title

    // Hue Field
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
        if (value < 0.0f) value = 0.0f;
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
    // We'll do a quick push to the Raylib transform stack
    // Alternatively, you can do an explicit Y offset in DrawMenuContent
    rlPushMatrix();
    rlTranslatef(0.0f, scrollOffset, 0.0f);

    // Actually draw the menu contents. This returns how tall it was
    totalContentHeight = DrawMenuContent(bounds.x, bounds.y);

    rlPopMatrix();
    EndScissorMode();

    // Optionally draw a visual scroll bar or up/down arrow here
}

void Menu::Update() {
    Rectangle bounds = GetMenuBounds();
    isMouseHovering = CheckCollisionPointRec(GetMousePosition(), bounds);

    // Fade effect
    if (isMouseHovering) {
        backgroundAlpha += FADE_IN_SPEED;
        if (backgroundAlpha > MAX_ALPHA) backgroundAlpha = MAX_ALPHA;
    } else {
        backgroundAlpha -= FADE_OUT_SPEED;
        if (backgroundAlpha < MIN_ALPHA) backgroundAlpha = MIN_ALPHA;
    }

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
    float maxScroll = 0;
    float minScroll = menuHeight - totalContentHeight;
    if (totalContentHeight <= menuHeight) {
        scrollOffset = 0;
    } else {
        if (scrollOffset > maxScroll) scrollOffset = maxScroll;
        if (scrollOffset < minScroll) scrollOffset = minScroll;
    }

    // If content fits in the area, no scroll needed
    if (totalContentHeight <= menuHeight) {
        scrollOffset = 0;
    } else {
        // Otherwise clamp
        if (scrollOffset > maxScroll) scrollOffset = maxScroll;
        if (scrollOffset < minScroll) scrollOffset = minScroll;
    }
}
