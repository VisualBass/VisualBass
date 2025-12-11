#include "Menu.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include "rlgl.h"
#include "menu/ColorPicker.h"
#include "menu/GetColorFromHue.h"
#include "globals.h"

// Bring in global variables needed for the slider
extern int orbCount;
extern float hueShift;

Menu::Menu()
    : visible(false),
      backgroundAlpha(MIN_ALPHA),
      isMouseHovering(false),
      editingHue(false),
      scrollOffset(0.0f),
      // Initialize the Generic Slider
      // Range: 1 to 1250 (matches MAX_ORBS in main.cpp), controlling 'orbCount', label "Orbs"
      orbSlider(1, 1250, orbCount, "Orbs", hueShift)
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

Vector2 Menu::GetLocalMousePos() {
    Rectangle bounds = GetMenuBounds();
    Vector2 mouse = GetMousePosition();
    mouse.x -= bounds.x;
    mouse.y -= bounds.y;
    mouse.y -= scrollOffset;
    return mouse;
}

// Updated: Accepts currentMode to allow contextual settings
float Menu::DrawMenuContent(float offsetX, float offsetY, int currentMode) {
    float startY = offsetY;
    Vector2 mousePos = GetMousePosition();

    // ==========================================
    // SECTION 1: GLOBAL SETTINGS (Always Visible)
    // ==========================================

    DrawText("GLOBAL SETTINGS", (int)offsetX + 20, (int)offsetY + 20, 24, Fade(RAYWHITE, backgroundAlpha));
    offsetY += 60;

    // --- HUE FIELD ---
    Rectangle hueField = { offsetX + 20, offsetY, (float)(GetMenuBounds().width - 40), 30 };
    offsetY += 30 + 20;

    Color hueFieldColor = editingHue ? LIGHTGRAY : GetColorFromHue((int)hueShift);

    bool hoverHue = CheckCollisionPointRec(mousePos, hueField);

    if (hoverHue && !editingHue) hueFieldColor = SKYBLUE;
    DrawRectangleRec(hueField, Fade(hueFieldColor, backgroundAlpha));

    if (!editingHue && hoverHue && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        editingHue = true;
        sprintf(hueBuffer, "%.0f", hueShift);
    }

    if (editingHue) {
        UpdateTextInput();
        DrawText(hueBuffer, (int)(hueField.x + 10), (int)(hueField.y + 5), 20, Fade(WHITE, backgroundAlpha));
        if (((int)(GetTime()*2)) % 2) {
            float textWidth = MeasureText(hueBuffer, 20);
            DrawText("|", (int)(hueField.x + 10 + textWidth), (int)(hueField.y + 5), 20, Fade(WHITE, backgroundAlpha));
        }
    } else {
        char hueText[32];
        if (autoCycleHue) sprintf(hueText, "Hue: Cycle");
        else sprintf(hueText, "Hue: %.0f°", hueShift);
        DrawText(hueText, (int)(hueField.x + 10), (int)(hueField.y + 5), 20, Fade(WHITE, backgroundAlpha));
    }

    // --- COLOR PICKER ---
    Rectangle pickerArea = { offsetX + 20, offsetY, 200, 100 };
    colorPicker.DrawColorPicker(pickerArea, hueShift);
    offsetY += 100 + 20;
    colorPicker.UpdateColorSelection(hueShift);

    // ==========================================
    // SECTION 2: MODE-SPECIFIC SETTINGS
    // ==========================================

    offsetY += 20; // Add a divider gap
    DrawLine((int)offsetX + 10, (int)offsetY, (int)(offsetX + GetMenuBounds().width - 10), (int)offsetY, Fade(LIGHTGRAY, 0.5f));
    offsetY += 20;

// Switch based on the active mode (Visualizer IDs from main.cpp)
    // 0 = WAVEFORM, 1 = GRAVITY, 2 = CUBE, 3 = PARTICLE
    switch (currentMode) {
        case 0: // WAVEFORM_MODE
        {
            // FIX: Explicitly handle 0 so it says "Waveform" instead of "Mode Settings"
            DrawText("WAVEFORM SETTINGS", (int)offsetX + 20, (int)offsetY, 20, Fade(GREEN, backgroundAlpha));
            offsetY += 30;
            DrawText("No settings for Waveform yet.", (int)offsetX + 20, (int)offsetY, 18, Fade(GRAY, backgroundAlpha));
            offsetY += 30;
            break;
        }

        case 1: // GRAVITY_MODE
        {
            DrawText("GRAVITY SETTINGS", (int)offsetX + 20, (int)offsetY, 20, Fade(GREEN, backgroundAlpha));
            offsetY += 30;

            // Orb Slider is only relevant here
            Rectangle sliderRect = { offsetX + 20, offsetY, (float)(GetMenuBounds().width - 40), 20 };
            orbSlider.UpdateSlider();
            orbSlider.DrawSlider(sliderRect);
            offsetY += 50;
            break;
        }

        case 2: // CUBE_MODE
        {
            DrawText("CUBE SETTINGS", (int)offsetX + 20, (int)offsetY, 20, Fade(GREEN, backgroundAlpha));
            offsetY += 30;
            DrawText("No settings for Cubes yet.", (int)offsetX + 20, (int)offsetY, 18, Fade(GRAY, backgroundAlpha));
            offsetY += 30;
            break;
        }

        case 3: // PARTICLE_MODE_01
        {
            // FIX: Specific name "Particle System 01"
            DrawText("PARTICLE SYSTEM 01", (int)offsetX + 20, (int)offsetY, 20, Fade(GREEN, backgroundAlpha));
            offsetY += 30;
            DrawText("No settings for Particles yet.", (int)offsetX + 20, (int)offsetY, 18, Fade(GRAY, backgroundAlpha));
            offsetY += 30;
            break;
        }

        default: // Fallback for any unknown IDs
        {
            DrawText("UNKNOWN MODE", (int)offsetX + 20, (int)offsetY, 20, Fade(RED, backgroundAlpha));
            offsetY += 30;
            break;
        }
    }

    // Return the total height of the content
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
        hueBuffer[strlen(hueBuffer)-1] = '\0';
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

// Updated: Accepts currentMode
void Menu::Draw(int currentMode) {
    if (!visible && backgroundAlpha <= MIN_ALPHA + 0.01f) return;
    Rectangle bounds = GetMenuBounds();
    DrawRectangleRounded(bounds, 0.15f, 12, Fade(DARKGRAY, backgroundAlpha));

    BeginScissorMode((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height);

    // Pass currentMode to the content drawer and add scrollOffset manually
    totalContentHeight = DrawMenuContent(bounds.x, bounds.y + scrollOffset, currentMode);

    EndScissorMode();
}

void Menu::Update() {
    Rectangle bounds = GetMenuBounds();
    Vector2 mouse = GetMousePosition();
    isMouseHovering = CheckCollisionPointRec(mouse, bounds);

    if (isMouseHovering) {
        backgroundAlpha += FADE_IN_SPEED;
        if (backgroundAlpha > MAX_ALPHA) backgroundAlpha = MAX_ALPHA;
    } else {
        backgroundAlpha -= FADE_OUT_SPEED;
        if (backgroundAlpha < MIN_ALPHA) backgroundAlpha = MIN_ALPHA;
    }

    if (isMouseHovering) {
        float wheelMove = GetMouseWheelMove();
        if (fabs(wheelMove) > 0.001f) {
            scrollOffset += wheelMove * 15.0f;
        }
    }

    float menuHeight = bounds.height;
    float maxScroll = 0;
    float minScroll = menuHeight - totalContentHeight;
    if (totalContentHeight <= menuHeight) {
        scrollOffset = 0;
    } else {
        if (scrollOffset > maxScroll) scrollOffset = maxScroll;
        if (scrollOffset < minScroll) scrollOffset = minScroll;
    }

    if (IsKeyPressed(KEY_TAB)) {
        visible = !visible;
        if (!visible) editingHue = false;
    }
}