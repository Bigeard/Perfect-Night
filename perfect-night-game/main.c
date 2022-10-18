#include "raylib.h"
#include <emscripten/emscripten.h>

static void main_loop(void);

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Perfect Night");
    InitAudioDevice();
    emscripten_set_main_loop(main_loop, 60, 1);
    return 0;
}

static void main_loop(void)
{
    
}