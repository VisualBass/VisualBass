#include "ToggleControl.h"
#include "menu/GetColorFromHue.h"

ToggleControl::ToggleControl(bool& targetValue, const char* labelText, float& globalHue)
    : targetRef(&targetValue), label(labelText), hueRef(globalHue)
{
}

void ToggleControl::Draw(Rectangle bounds, float uiScale) {
    Vector2 mousePos = GetMousePosition();
    bool isHover = CheckCollisionPointRec(mousePos, bounds);
    bool isOn = *targetRef;

    if (isHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        *targetRef = !isOn;
    }

    Color hueColor = GetColorFromHue((int)hueRef);

    // Style
    Color bgColor = isOn ? Fade(hueColor, 0.7f) : Fade(DARKGRAY, 0.6f);
    if (isHover) bgColor = isOn ? Fade(hueColor, 0.9f) : Fade(DARKGRAY, 0.8f);
    Color borderColor = (isOn || isHover) ? hueColor : GRAY;
    Color textColor = isOn ? BLACK : WHITE;

    DrawRectangleRec(bounds, bgColor);
    DrawRectangleLinesEx(bounds, 1.0f * uiScale, borderColor);

    int fontSize = (int)(20 * uiScale);
    int textWidth = MeasureText(label, fontSize);
    int textX = (int)(bounds.x + (bounds.width - textWidth) / 2.0f);
    int textY = (int)(bounds.y + (bounds.height - fontSize) / 2.0f);

    DrawText(label, textX, textY, fontSize, textColor);
}