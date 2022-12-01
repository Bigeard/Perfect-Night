#include "raylib.h"
#include "src/gameplay/gameplay.h"
#include <emscripten/emscripten.h>

static void GameMainLoop(void);

int main(void)
{
    // const int screenWidth = 1280;
    // const int screenHeight = 720;
    const int screenWidth = 1920 * 0.9;
    const int screenHeight = 1060 * 0.9;
    InitWindow(screenWidth, screenHeight, "Perfect Night");
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