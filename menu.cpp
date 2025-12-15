#include "Menu.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include "rlgl.h"
#include "menu/ColorPicker.h"
#include "menu/GetColorFromHue.h"
#include "globals.h"
#include "gravityorbs.h"

extern int orbCount;
extern float hueShift;
extern float gravityStrength;
extern float orbMaxSize;
extern float mouseRepelForce;

// Access globals defined in main.cpp
extern float brightnessFloor;
extern float hueSpeed;

// Constructor initializes DebugMenu with hueShift reference
Menu::Menu()
    : visible(false),
      backgroundAlpha(MIN_ALPHA),
      isMouseHovering(false),
      editingHue(false),
      scrollOffset(0.0f),
      lifxConnected(false),
      orbSlider(1, 1250, orbCount, 1250, "Orbs", hueShift),
      brightnessSlider(0.0f, 1.0f, brightnessFloor, 0.0f, "Floor", hueShift),
      // Binds to global hueSpeed
      hueSpeedSlider(0.01f, 40.0f, hueSpeed, 15.0f, "Hue Speed", hueShift),
      gravitySlider(0.1f, 25.0f, gravityStrength, 18.0f, "Gravity", hueShift),
      sizeSlider(0.1f, 100.0f, orbMaxSize, 20.0f, "Max Size", hueShift),
      repelSlider(0.0f, 20.0f, mouseRepelForce, 5.0f, "Repel", hueShift),
      debugMenu(hueShift)
{
    hueBuffer[0] = '\0';
}

bool Menu::IsVisible() const { return visible; }

Rectangle Menu::GetMenuBounds() {
    const float margin = 20.0f;
    float width  = GetScreenWidth() * MENU_WIDTH_RATIO;
    float height = GetScreenHeight() * MENU_HEIGHT_RATIO;
    return (Rectangle){ margin, (float)GetScreenHeight() - height - margin, width, height };
}

Vector2 Menu::GetLocalMousePos() {
    Rectangle bounds = GetMenuBounds();
    Vector2 mouse = GetMousePosition();
    return (Vector2){ mouse.x - bounds.x, mouse.y - bounds.y - scrollOffset };
}

float Menu::DrawMenuContent(float offsetX, float offsetY, Rectangle visibleArea, int currentMode, float uiScale) {
    float startY = offsetY;
    Vector2 mousePos = GetMousePosition();

    // Define scaled sizes
    int fontHeader = (int)(24 * uiScale);
    int fontText   = (int)(20 * uiScale);
    int fontButton = (int)(10 * uiScale);

    float paddingStandard = 20.0f * uiScale;
    float paddingLarge    = 60.0f * uiScale;
    float paddingMedium   = 40.0f * uiScale;
    float paddingSmall    = 10.0f * uiScale;

    float rowHeight       = 30.0f * uiScale;
    float sliderHeight    = 20.0f * uiScale;

    // ==========================================
    // SECTION 1: GLOBAL SETTINGS
    // ==========================================

    DrawText("GLOBAL SETTINGS", (int)(offsetX + paddingStandard), (int)(offsetY + paddingStandard), fontHeader, Fade(RAYWHITE, backgroundAlpha));
    offsetY += paddingLarge;

    // --- HUE FIELD ---
    Rectangle hueField = { offsetX + paddingStandard, offsetY, (float)(GetMenuBounds().width - (paddingStandard * 2)), rowHeight };
    offsetY += 50.0f * uiScale;

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
        DrawText(hueBuffer, (int)(hueField.x + (10 * uiScale)), (int)(hueField.y + (5 * uiScale)), fontText, Fade(WHITE, backgroundAlpha));
    } else {
        char hueText[32];
        if (autoCycleHue) sprintf(hueText, "Hue: Cycle");
        else sprintf(hueText, "Hue: %.0f°", hueShift);
        DrawText(hueText, (int)(hueField.x + (10 * uiScale)), (int)(hueField.y + (5 * uiScale)), fontText, Fade(WHITE, backgroundAlpha));
    }

    // --- COLOR PICKER ---
    Rectangle pickerArea = { offsetX + paddingStandard, offsetY, (float)(GetMenuBounds().width - (paddingStandard * 2)), 0 };
    int pickerHeight = colorPicker.DrawColorPicker(pickerArea, hueShift, uiScale);
    colorPicker.UpdateColorSelection(hueShift);
    offsetY += pickerHeight + paddingStandard;

    // ==========================================
    // BRIGHTNESS FLOOR SLIDER
    // ==========================================
    DrawText("BRIGHTNESS FLOOR", (int)(offsetX + paddingStandard), (int)offsetY, fontText, Fade(GREEN, backgroundAlpha));
    offsetY += 30.0f * uiScale;

    Rectangle floorRect = { offsetX + paddingStandard, offsetY, (float)(GetMenuBounds().width - (paddingStandard * 2)), sliderHeight };
    brightnessSlider.UpdateSlider();
    brightnessSlider.DrawSlider(floorRect, visibleArea, uiScale);
    offsetY += 50.0f * uiScale;

    // ==========================================
    // NEW: HUE SPEED SLIDER
    // ==========================================
    DrawText("HUESHIFT SPEED", (int)(offsetX + paddingStandard), (int)offsetY, fontText, Fade(GREEN, backgroundAlpha));
    offsetY += 30.0f * uiScale;

    Rectangle hueSpeedRect = { offsetX + paddingStandard, offsetY, (float)(GetMenuBounds().width - (paddingStandard * 2)), sliderHeight };
    hueSpeedSlider.UpdateSlider();
    hueSpeedSlider.DrawSlider(hueSpeedRect, visibleArea, uiScale);

    // UX: Force AutoCycle on drag so user sees the speed change immediately
    if (CheckCollisionPointRec(mousePos, hueSpeedRect) && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        autoCycleHue = true;
    }

    offsetY += 50.0f * uiScale;

    // ==========================================
    // SECTION 2: MODE-SPECIFIC SETTINGS
    // ==========================================
    offsetY += paddingSmall;
    DrawLine((int)(offsetX + 10 * uiScale), (int)offsetY, (int)(offsetX + GetMenuBounds().width - (10 * uiScale)), (int)offsetY, Fade(LIGHTGRAY, 0.5f));
    offsetY += paddingStandard;

    switch (currentMode) {
        case 0: // WAVEFORM
            DrawText("WAVEFORM SETTINGS", (int)(offsetX + paddingStandard), (int)offsetY, fontText, Fade(GREEN, backgroundAlpha));
            offsetY += paddingLarge;
            break;

        case 1: // GRAVITY
        {
            DrawText("GRAVITY SETTINGS", (int)(offsetX + paddingStandard), (int)offsetY, fontText, Fade(GREEN, backgroundAlpha));
            offsetY += 30.0f * uiScale;

            Rectangle sliderRect = { offsetX + paddingStandard, offsetY, (float)(GetMenuBounds().width - (paddingStandard * 2)), sliderHeight };

            orbSlider.UpdateSlider();
            orbSlider.DrawSlider(sliderRect, visibleArea, uiScale);
            offsetY += paddingMedium;

            sliderRect.y = offsetY;
            gravitySlider.UpdateSlider();
            gravitySlider.DrawSlider(sliderRect, visibleArea, uiScale);
            offsetY += paddingMedium;

            sliderRect.y = offsetY;
            sizeSlider.UpdateSlider();
            sizeSlider.DrawSlider(sliderRect, visibleArea, uiScale);
            offsetY += paddingMedium;

            sliderRect.y = offsetY;
            repelSlider.UpdateSlider();
            repelSlider.DrawSlider(sliderRect, visibleArea, uiScale);
            offsetY += paddingMedium;

            // RESET BUTTON
            float btnWidth = 100.0f * uiScale;
            float btnHeight = 25.0f * uiScale;
            float btnX = (offsetX + GetMenuBounds().width) / 2.0f - (btnWidth / 2.0f);
            Rectangle resetBtnRect = { btnX, offsetY, btnWidth, btnHeight };

            bool hoverReset = CheckCollisionPointRec(mousePos, resetBtnRect);
            if (hoverReset && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                orbCount = 1250;
                gravityStrength = 18.0f;
                orbMaxSize = 20.0f;
                mouseRepelForce = 5.0f;
            }
            Color btnColor = hoverReset ? RED : MAROON;
            DrawRectangleRounded(resetBtnRect, 0.3f, 4, Fade(btnColor, backgroundAlpha));
            const char* resetText = "RESET ALL";
            int tw = MeasureText(resetText, fontButton);
            DrawText(resetText, (int)(resetBtnRect.x + (btnWidth - tw)/2), (int)(resetBtnRect.y + (8 * uiScale)), fontButton, WHITE);
            offsetY += 50.0f * uiScale;
            break;
        }

        case 2: // CUBE_MODE
            DrawText("CUBE SETTINGS", (int)(offsetX + paddingStandard), (int)offsetY, fontText, Fade(GREEN, backgroundAlpha));
            offsetY += 30.0f * uiScale;
            DrawText("No settings for Cubes yet.", (int)(offsetX + paddingStandard), (int)offsetY, (int)(18 * uiScale), Fade(GRAY, backgroundAlpha));
            offsetY += 30.0f * uiScale;
            break;

        case 3: // PARTICLE_MODE
            DrawText("PARTICLE SYSTEM 01", (int)(offsetX + paddingStandard), (int)offsetY, fontText, Fade(GREEN, backgroundAlpha));
            offsetY += 30.0f * uiScale;
            DrawText("No settings for Particles yet.", (int)(offsetX + paddingStandard), (int)offsetY, (int)(18 * uiScale), Fade(GRAY, backgroundAlpha));
            offsetY += 30.0f * uiScale;
            break;

        default: break;
    }

    // ==========================================
    // SECTION 3: SMART LIGHTS / HARDWARE
    // ==========================================
    offsetY += paddingStandard;
    DrawLine((int)(offsetX + 10 * uiScale), (int)offsetY, (int)(offsetX + GetMenuBounds().width - (10 * uiScale)), (int)offsetY, Fade(LIGHTGRAY, 0.5f));
    offsetY += paddingStandard;

    DrawText("SMART LIGHTS", (int)(offsetX + paddingStandard), (int)offsetY, fontText, Fade(ORANGE, backgroundAlpha));
    offsetY += 40.0f * uiScale;

    DrawText("LIFX Integration", (int)(offsetX + paddingStandard), (int)(offsetY + 5 * uiScale), fontText, Fade(WHITE, backgroundAlpha));
    float btnWidth = 120.0f * uiScale;
    float btnHeight = 30.0f * uiScale;
    float btnX = (offsetX + GetMenuBounds().width) - btnWidth - (30.0f * uiScale);
    Rectangle btnRect = { btnX, offsetY, btnWidth, btnHeight };

    bool btnHover = CheckCollisionPointRec(mousePos, btnRect);
    if (btnHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        lifxConnected = !lifxConnected;
        if (lifxConnected) LaunchLIFX();
        else StopLIFX();
    }

    Color btnColor = lifxConnected ? (btnHover ? DARKGREEN : GREEN) : (btnHover ? GRAY : DARKGRAY);
    DrawRectangleRounded(btnRect, 0.3f, 6, Fade(btnColor, backgroundAlpha));
    const char* btnText = lifxConnected ? "CONNECTED" : "CONNECT";
    int textWidth = MeasureText(btnText, fontButton);
    DrawText(btnText, (int)(btnRect.x + (btnRect.width - textWidth) / 2), (int)(btnRect.y + (btnRect.height - fontButton) / 2), fontButton, WHITE);
    offsetY += 50.0f * uiScale;

    // ==========================================
    // SECTION 4: DEBUG SETTINGS
    // ==========================================
    offsetY += paddingStandard;
    DrawLine((int)(offsetX + 10 * uiScale), (int)offsetY, (int)(offsetX + GetMenuBounds().width - (10 * uiScale)), (int)offsetY, Fade(LIGHTGRAY, 0.5f));
    offsetY += paddingStandard;

    // Theming: Title for Debug Section
    DrawText("DEBUG SETTINGS", (int)(offsetX + paddingStandard), (int)offsetY, fontText, Fade(RED, backgroundAlpha));
    offsetY += 40.0f * uiScale;

    // Call the debug menu draw function and add its height to offsetY
    // We adjust offsetY slightly because DebugMenu draws controls relative to the start point
    offsetY += debugMenu.Draw(offsetX, offsetY, GetMenuBounds().width, uiScale, backgroundAlpha);

    return offsetY - startY;
}

void Menu::UpdateTextInput() {
    int key = GetCharPressed();
    while(key > 0) {
        if (key >= '0' && key <= '9') {
            int len = (int)strlen(hueBuffer);
            if (len < 15) { hueBuffer[len] = (char)key; hueBuffer[len+1] = '\0'; }
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && strlen(hueBuffer) > 0) hueBuffer[strlen(hueBuffer)-1] = '\0';
    if (IsKeyPressed(KEY_ENTER)) {
        float value = atof(hueBuffer);
        if (value > 360.0f) value = 360.0f;
        hueShift = value;
        editingHue = false;
        autoCycleHue = (value == 0.0f);
        hueBuffer[0] = '\0';
    }
}

void Menu::Draw(int currentMode) {
    if (!visible && backgroundAlpha <= MIN_ALPHA + 0.01f) return;

    // --- SCALING LOGIC ---
    // If screen height is greater than 1080p, use 125% scale
    float h = (float)GetScreenHeight();
    float uiScale = (h > 1080.0f) ? 1.25f : 1.0f;

    Rectangle bounds = GetMenuBounds();
    DrawRectangleRounded(bounds, 0.15f, 12, Fade(DARKGRAY, backgroundAlpha));

    BeginScissorMode((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height);

    // Pass bounds and uiScale
    totalContentHeight = DrawMenuContent(bounds.x, bounds.y + scrollOffset, bounds, currentMode, uiScale);

    EndScissorMode();
}

void Menu::Update() {
    Rectangle bounds = GetMenuBounds();
    Vector2 mouse = GetMousePosition();
    isMouseHovering = CheckCollisionPointRec(mouse, bounds);

    if (isMouseHovering) {
        backgroundAlpha += FADE_IN_SPEED;
        if (backgroundAlpha > MAX_ALPHA) backgroundAlpha = MAX_ALPHA;

        float wheelMove = GetMouseWheelMove();
        if (fabs(wheelMove) > 0.001f) scrollOffset += wheelMove * 15.0f;
    } else {
        backgroundAlpha -= FADE_OUT_SPEED;
        if (backgroundAlpha < MIN_ALPHA) backgroundAlpha = MIN_ALPHA;
    }

    float menuHeight = bounds.height;
    float maxScroll = 0;
    float minScroll = menuHeight - totalContentHeight;
    if (totalContentHeight <= menuHeight) scrollOffset = 0;
    else {
        if (scrollOffset > maxScroll) scrollOffset = maxScroll;
        if (scrollOffset < minScroll) scrollOffset = minScroll;
    }

    if (IsKeyPressed(KEY_TAB)) {
        visible = !visible;
        if (!visible) editingHue = false;
    }
}