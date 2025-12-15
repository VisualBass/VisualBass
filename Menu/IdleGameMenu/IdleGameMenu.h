#ifndef IDLEGAMEMENU_H
#define IDLEGAMEMENU_H

#include "raylib.h"
// Go up two directories to find the header in Root
#include "../../IdleGame/IdleGame.h"

class IdleGameMenu {
public:
    IdleGameMenu();

    void Draw(IdleGame& game, float uiScale);

private:
    const char* FormatNumber(double num);
};

#endif