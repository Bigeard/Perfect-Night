#include "../lib/raylib/src/raylib.h"
#include "src/gameplay/gameplay.h"
#include <emscripten/emscripten.h>

static void GameMainLoop(void);

int main(void)
{
    InitWindow(1920, 1060, "Perfect Night");
    InitAudioDevice();
    InitGameplay();
    emscripten_set_main_loop(GameMainLoop, 120, 1);
    return 0;
}

static void GameMainLoop(void)
{
    UpdateGameplay();
    DrawGameplay();
}
