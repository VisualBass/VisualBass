#include "SliderControl.h"
#include "menu/GetColorFromHue.h" // Ensure this path matches your file structure
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <cstdlib>

SliderControl::SliderControl(int min, int max, int& targetValue, int defaultValue, const char* labelText, float& globalHue)
    : type(SLIDER_INT), targetRef(&targetValue), minVal((float)min), maxVal((float)max),
      defaultVal((float)defaultValue), label(labelText), hueRef(globalHue), isDragging(false),
      isEditing(false), cursorBlinkFrame(0)
{
    textBuffer[0] = '\0';
}

SliderControl::SliderControl(float min, float max, float& targetValue, float defaultValue, const char* labelText, float& globalHue)
    : type(SLIDER_FLOAT), targetRef(&targetValue), minVal(min), maxVal(max),
      defaultVal(defaultValue), label(labelText), hueRef(globalHue), isDragging(false),
      isEditing(false), cursorBlinkFrame(0)
{
    textBuffer[0] = '\0';
}

float SliderControl::GetNormalizedValue() const {
    float current = 0.0f;
    if (type == SLIDER_INT) current = (float)(*(int*)targetRef);
    else current = *(float*)targetRef;

    if (maxVal == minVal) return 0.0f;
    return (current - minVal) / (maxVal - minVal);
}

void SliderControl::SetValueFromNormalized(float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    if (type == SLIDER_INT) {
        int range = (int)(maxVal - minVal);
        *(int*)targetRef = (int)(minVal) + (int)std::round(t * range);
    } else {
        float range = maxVal - minVal;
        *(float*)targetRef = minVal + (t * range);
    }
}

void SliderControl::ApplyTextBuffer() {
    if (strlen(textBuffer) == 0) return;

    float val = (float)atof(textBuffer);

    // Clamp value to limits
    if (val < minVal) val = minVal;
    if (val > maxVal) val = maxVal;

    // Apply to target
    if (type == SLIDER_INT) {
        *(int*)targetRef = (int)std::round(val);
    } else {
        *(float*)targetRef = val;
    }
}

void SliderControl::UpdateSlider() {
    // Logic handled in DrawSlider
}

// UPDATED: Now uses scale for fonts and lines
void SliderControl::DrawSlider(Rectangle bounds, Rectangle clipRegion, float scale) {
    Vector2 mousePos = GetMousePosition();
    bool isHover = CheckCollisionPointRec(mousePos, bounds);
    bool mouseInClip = CheckCollisionPointRec(mousePos, clipRegion);

    // Scaled Font Size
    int fontSize = (int)(20 * scale);

    // ============================================================
    // STATE 1: EDITING MODE (Text Input)
    // ============================================================
    if (isEditing) {
        // 1. Handle Key Input
        int key = GetCharPressed();
        while (key > 0) {
            // Allow digits, decimal point, and minus sign
            if ((key >= '0' && key <= '9') || key == '.' || key == '-') {
                int len = (int)strlen(textBuffer);
                if (len < 15) {
                    textBuffer[len] = (char)key;
                    textBuffer[len+1] = '\0';
                }
            }
            key = GetCharPressed();
        }

        // Backspace
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = (int)strlen(textBuffer);
            if (len > 0) textBuffer[len - 1] = '\0';
        }

        // Confirm (Enter) or Cancel (Escape/Click Outside)
        bool confirm = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER);
        bool cancel = IsKeyPressed(KEY_ESCAPE) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !isHover);

        if (confirm) {
            ApplyTextBuffer();
            isEditing = false;
        } else if (cancel) {
            isEditing = false;
        }

        // 2. Render Edit Box (High Contrast)
        DrawRectangleRec(bounds, WHITE);
        DrawRectangleLinesEx(bounds, 2 * scale, GetColorFromHue((int)hueRef));

        // Blinking Cursor logic
        cursorBlinkFrame++;
        bool showCursor = (cursorBlinkFrame / 30) % 2 == 0;

        const char* displayStr = (showCursor) ? TextFormat("%s_", textBuffer) : textBuffer;

        int textWidth = MeasureText(textBuffer, fontSize); // Measure sans cursor for centering
        int textX = (int)(bounds.x + (bounds.width - textWidth) / 2.0f);
        int textY = (int)(bounds.y + (bounds.height - fontSize) / 2.0f);

        DrawText(displayStr, textX, textY, fontSize, BLACK);
        return; // Return early, don't draw the slider bar
    }

    // ============================================================
    // STATE 2: SLIDER MODE (Standard Interaction)
    // ============================================================

    // Shift + Click to enter Edit Mode
    if (mouseInClip && isHover && IsKeyDown(KEY_LEFT_SHIFT) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        isEditing = true;
        isDragging = false;
        cursorBlinkFrame = 0;

        // Load current value into buffer so user sees what they are editing
        if (type == SLIDER_INT) sprintf(textBuffer, "%d", *(int*)targetRef);
        else sprintf(textBuffer, "%.2f", *(float*)targetRef);

        return; // Skip the rest of the frame to prevent double interaction
    }

    // Right Click Reset
    if (mouseInClip && isHover && IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        if (type == SLIDER_INT) *(int*)targetRef = (int)defaultVal;
        else *(float*)targetRef = defaultVal;
        isDragging = false;
    }

    // Left Click Drag
    if (mouseInClip && isHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        isDragging = true;
    }
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        isDragging = false;
    }

    // Update Value while Dragging
    if (isDragging) {
        float mouseXRel = mousePos.x - bounds.x;
        float t = mouseXRel / bounds.width;
        SetValueFromNormalized(t);
    }
    else if (mouseInClip && isHover) {
        // Mouse Wheel
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            float t = GetNormalizedValue();
            SetValueFromNormalized(t + (wheel * 0.05f));
        }
    }

    // --- RENDER SLIDER ---
    bool active = (isHover && mouseInClip) || isDragging;
    Color currentHueColor = GetColorFromHue((int)hueRef);
    Color fillColor = active ? BLACK : currentHueColor;
    Color textBaseColor = active ? currentHueColor : WHITE;
    Color textFillColor = active ? currentHueColor : BLACK;

    DrawRectangleRec(bounds, Fade(DARKGRAY, 0.6f));

    float t = GetNormalizedValue();
    float fillWidth = bounds.width * t;
    Rectangle fillRect = { bounds.x, bounds.y, fillWidth, bounds.height };
    DrawRectangleRec(fillRect, Fade(fillColor, 0.9f));

    char valText[32];
    if (type == SLIDER_INT) std::sprintf(valText, "%s: %d", label, *(int*)targetRef);
    else std::sprintf(valText, "%s: %.2f", label, *(float*)targetRef);

    int textWidth = MeasureText(valText, fontSize);
    int textX = (int)(bounds.x + (bounds.width - textWidth) / 2.0f);
    int textY = (int)(bounds.y + (bounds.height - fontSize) / 2.0f);

    DrawText(valText, textX, textY, fontSize, textBaseColor);

    if (fillWidth > 1.0f) {
        float scissorX = fmaxf(fillRect.x, clipRegion.x);
        float scissorY = fmaxf(fillRect.y, clipRegion.y);
        float scissorRight = fminf(fillRect.x + fillRect.width, clipRegion.x + clipRegion.width);
        float scissorBottom = fminf(fillRect.y + fillRect.height, clipRegion.y + clipRegion.height);

        float scissorW = scissorRight - scissorX;
        float scissorH = scissorBottom - scissorY;

        if (scissorW > 0 && scissorH > 0) {
            BeginScissorMode((int)scissorX, (int)scissorY, (int)scissorW, (int)scissorH);
            DrawText(valText, textX, textY, fontSize, textFillColor);

            // Restore Parent Scissor
            BeginScissorMode((int)clipRegion.x, (int)clipRegion.y, (int)clipRegion.width, (int)clipRegion.height);
        }
    }

    Color borderColor = active ? currentHueColor : GRAY;
    DrawRectangleLinesEx(bounds, 1 * scale, borderColor);
}