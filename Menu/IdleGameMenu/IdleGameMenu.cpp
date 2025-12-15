#include "IdleGameMenu.h"
#include <cstdio>
#include <string>

IdleGameMenu::IdleGameMenu() {}

void IdleGameMenu::Draw(IdleGame& game, float uiScale) {
    // --- LAYOUT SETTINGS ---
    float screenW = (float)GetScreenWidth();
    float padding = 20.0f * uiScale;

    // Font setup
    int fontSizeLarge = (int)(28 * uiScale); // XP Amount
    int fontSizeSmall = (int)(18 * uiScale); // Floating XP

    // 1. Calculate XP Box Dimensions
    char xpBuffer[64];
    std::sprintf(xpBuffer, "XP: %.0f", game.GetTotalXP());
    int txtWidth = MeasureText(xpBuffer, fontSizeLarge);

    // Box Dimensions (Glass Style)
    float minBoxWidth = 140.0f * uiScale;
    float boxWidth = (txtWidth + 40.0f * uiScale) > minBoxWidth ? (txtWidth + 40.0f * uiScale) : minBoxWidth;
    float boxHeight = 50.0f * uiScale;

    // Anchor Position (Top Right)
    float boxX = screenW - boxWidth - padding;
    float boxY = padding;
    Rectangle xpBox = { boxX, boxY, boxWidth, boxHeight };

    // --- 2. DRAW FLOATING TEXTS (Background Layer) ---
    // These are drawn FIRST so they appear BEHIND the glass box

    // Cast away constness to update render positions if they are new
    auto& texts = const_cast<std::vector<FloatingText>&>(game.GetFloatingTexts());

    for (auto& ft : texts) {
        // FIX: Check only X. Logic update might have moved Y slightly off 0.0f already.
        // If X is 0 (default), we assume it hasn't been placed by the UI yet.
        if (ft.position.x <= 1.0f) {
            // Measure this text to center it horizontally relative to the box
            int ftWidth = MeasureText(ft.text.c_str(), fontSizeSmall);
            ft.position.x = boxX + (boxWidth - ftWidth) / 2.0f;

            // Start further down so it has room to scroll up before hitting the box
            ft.position.y = boxY + boxHeight + (20.0f * uiScale);
        }

        // Standard life fade
        float alpha = ft.life;
        if (alpha > 1.0f) alpha = 1.0f;

        // --- VISUAL FIX ---
        // 1. If text is completely above the box top, hide it (clipped)
        if (ft.position.y < boxY) {
            alpha = 0.0f;
        }

        // 2. Only draw if visible
        if (alpha > 0.0f) {
            Color drawColor = Fade(ft.color, alpha);

            // Draw Shadow
            DrawText(ft.text.c_str(), (int)ft.position.x + 2, (int)ft.position.y + 2, fontSizeSmall, Fade(BLACK, alpha * 0.8f));
            // Draw Main Text
            DrawText(ft.text.c_str(), (int)ft.position.x, (int)ft.position.y, fontSizeSmall, drawColor);
        }
    }

    // --- 3. DRAW GLASSY BOX (Foreground Layer) ---
    // Drawn SECOND so it overlays the floating text, creating the "collision/absorb" effect

    // Base Dark Background (Solid enough to hide text behind it)
    DrawRectangleRounded(xpBox, 0.5f, 16, Fade(BLACK, 0.90f));

    // Glossy Shine Effect (Top Half)
    BeginScissorMode((int)xpBox.x, (int)xpBox.y, (int)xpBox.width, (int)(xpBox.height * 0.45f));
        DrawRectangleRounded(xpBox, 0.5f, 16, Fade(WHITE, 0.15f));
    EndScissorMode();

    // Border (Glassy Blue)
    // Thickness param removed for compatibility
    DrawRectangleRoundedLines(xpBox, 0.5f, 16, Fade(SKYBLUE, 0.8f));

    // Main XP Text (Centered in Box)
    int textX = (int)(boxX + (boxWidth - txtWidth) / 2.0f);
    int textY = (int)(boxY + (boxHeight - fontSizeLarge) / 2.0f);

    // Text Shadow & Glow
    DrawText(xpBuffer, textX + 2, textY + 2, fontSizeLarge, Fade(BLACK, 0.5f));
    DrawText(xpBuffer, textX, textY, fontSizeLarge, WHITE);
}