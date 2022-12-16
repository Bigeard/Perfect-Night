#include "../../../raylib/src/raylib.h"
#include "../../../raylib/src/raymath.h"
#include <emscripten/emscripten.h>
#include "stdio.h"

#include "../../../tmx/src/tmx.h"
#include "../tmx_raylib/tmx_raylib.h"

#include "../physic/physic.h"
#include "../player/player.h"
#include "../bullet/bullet.h"
#include "../box/box.h"
#include "gameplay.h"


EM_JS(int, GetCanvasWidthCustom, (), { return window.innerWidth });
EM_JS(int, GetCanvasHeightCustom, (), { return window.innerHeight });
EM_JS(int, ToggleInfoPeerJs, (), { return togglePeerJs() });
EM_JS(int, GetNumberPlayer, (), { return listGamepad.size });
EM_JS(char*, GetIdGamepad, (const int index), { 
    let i = 0;
    let res = "";
    listGamepad.forEach((_,k) => { i === index ? res = k : 0; i++; });
    const byteCount = Module.lengthBytesUTF8(res)+1;
    const idPointer = Module._malloc(byteCount);
    Module.stringToUTF8(res, idPointer, byteCount);
    return idPointer;
});

tmx_map *map;

float arenaSize = 32.0f * 32;
bool activeDev = true;
double lastSecond = 0;
static Camera2D camera = { 0 };
static bool pauseGame = 0;
int lastPlayer = 0;
bool playerSpace[8] = { 0 };

// static Player players[4] = { 0 };
static Player players[8] = {
    {},{},{},{},{},{},{},
    { 
        8, // ID
        "0", // Gamepad Id
        3, // Life
        0, // Invincible
        155, // Damages Taken
        1, // Ammunition
        300, // Ammunition loading
        {
            { 200 - 20, 200 - 20 }, // Position
            { 40, 40 }, // Size
            { 0.0, 0.0 }, // Velocity
            { 0, 0, 0, 0, 0 } // Collision: IsCollision, Up, Down, Left, Right
        },
        { 3.5, 3.5 }, // Speed
        { 0, 0 },
        2, // Charge 
        true, // Can Shoot 
        0, // Time Shoot
        0, // Radian
        { 0 }, // Bullets
        0, // Last Bullet
        { GREEN, LIME, DARKGREEN },
        MOUSE, // Input Type
        { KEY_Z, KEY_S, KEY_Q, KEY_D, KEY_G, KEY_F, KEY_H }, // KEY: Up, Down, Left, Right, MOVE CANNON, SHOT, MOVE CANNON
    },
    // {   
    //     2, "0", 3, 0, 0, 1000000, 300, {{600 - 20, 200 - 20}, {40, 40}, {0, 0}, {0, 0, 0, 0, 0}}, {3.5, 3.5}, { 0, 0 }, 2, true, 0, 0, { 0 }, 0, { PINK, RED, MAROON }, GAMEPAD,
    //     {GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_RIGHT_X, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, GAMEPAD_AXIS_RIGHT_Y},
    // },
    // {   
    //     3, "1", 3, 0, 0, 1, 300, {{200 - 20, 600 - 20}, {40, 40}, {0, 0}, {0, 0, 0, 0, 0}}, {3.5, 3.5}, { 0, 0 }, 2, true, 0, 0, { 0 }, 0, { SKYBLUE, BLUE, DARKBLUE }, GAMEPAD,
    //     {GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_RIGHT_X, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, GAMEPAD_AXIS_RIGHT_Y},
    // },
    // {   
    //     4, "4", 3, 0, 155, 1, 300, {{600 - 20, 600 - 20}, {40, 40}, {0, 0}, {0, 0, 0, 0, 0}}, {3.5, 3.5}, { 0, 0 }, 2, true, 0, 0, { 0 }, 0, { PURPLE, VIOLET, DARKPURPLE }, MOBILE,
    //     {GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_RIGHT_X, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, GAMEPAD_AXIS_RIGHT_Y},
    // },
};

static Box boxes[40] = {
    // {{{400, 150}, {100, 100}, {0, 0}}, GRAY },
    // {{{100, 300}, {100, 100}, {0, 0}}, GRAY },
    // {{{250, 350}, {100, 100}, {0, 0}}, GRAY },
    // {{{200, 300}, {100, 25}, {0, 0}}, GRAY },
    // {{{450, 450}, {100, 100}, {0, 0}}, GRAY },
    // {{{550, 250}, {100, 100}, {0, 0}}, GRAY },
    // {{{550, 350}, {100, 100}, {0, 0}}, GRAY },
    // {{{450, 650}, {100, 100}, {0, 0}}, GRAY },
    // {{{450, 550}, {100, 100}, {0, 0}}, GRAY },
    // {{{550, 450}, {100, 100}, {0, 0}}, GRAY },
    // {{{650, 450}, {100, 100}, {0, 0}}, GRAY },
};

static int playersLength = sizeof(players)/sizeof(players[0]);
static int boxesLength = sizeof(boxes)/sizeof(boxes[0]);


void InitGameplay(void) {
	tmx_img_load_func = raylib_tex_loader;
	tmx_img_free_func = raylib_free_tex;

	map = tmx_load("resources/map1.tmx");
	if (!map) {
		tmx_perror("Cannot load map");
	}

    tmx_init_object(map->ly_head, players, boxes);
    InitPlayer();

    camera.target = (Vector2){ 0, 0 };
    camera.offset = (Vector2){ 0, 0 };
    camera.rotation = 0.0f;
    camera.zoom = 1.3f;
}

void UpdateGameplay(void) {
    int centerPositionX = 0;
    int centerPositionY = 0;
    // int centerDistance = 0;

    double time = GetTime();
    if ((int)time == lastSecond) {
        SetWindowSize(GetCanvasWidthCustom(), GetCanvasHeightCustom());
        camera.target = (Vector2){ 0, 0 };
        camera.offset = (Vector2){ GetScreenWidth() - arenaSize/2 - GetScreenWidth()/2, GetScreenHeight() - arenaSize/2 - GetScreenHeight()/2 };
        lastSecond += 1;
    }

    int numberPlayer = GetNumberPlayer();
    if(lastPlayer < numberPlayer) {
        for (int i = 0; i < playersLength; i++) {
            if(!playerSpace[i]) {
                playerSpace[i] = true;
                players[i] = (Player) {
                    i+1, GetIdGamepad(i), 3, 0, 155, 1, 300, {{600 - 20, 600 - 20}, {40, 40}, {0, 0}, {0, 0, 0, 0, 0}}, {3.5, 3.5}, { 0, 0 }, 2, true, 0, 0, { 0 }, 0, { PURPLE, VIOLET, DARKPURPLE }, MOBILE,
                    {GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_RIGHT_X, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, GAMEPAD_AXIS_RIGHT_Y},
                };
                i = playersLength;
                lastPlayer++;
                tmx_init_object(map->ly_head, players, boxes);
            }
        }
    }

    if(lastPlayer > numberPlayer) {

    }

    camera.zoom += ((float)GetMouseWheelMove()*0.01f);

    // Pause
    if (IsKeyPressed(KEY_O)) activeDev = !activeDev;

    if (IsKeyPressed(KEY_U)) {
        ToggleInfoPeerJs();
    }

    // Pause
    if (IsKeyPressed(KEY_P)) pauseGame = !pauseGame;
    if (pauseGame) {
        if (IsKeyPressed(KEY_C)) {
            // Controller display
        }
        if (IsKeyPressed(KEY_E)) {
            // Edit map display
        }
        return;
    }

    // Reset
    if (IsKeyPressed(KEY_R)) {
        // Press R to reset
    }

    // Update Players / Bullets
    for (int i = 0; i < playersLength; i++) {
        UpdatePlayer(&players[i]);

        // Bullets
        for (int j = 0; j < sizeof(players[i].bullets)/sizeof(players[i].bullets[0]); j++) {
            UpdateBullet(&players[i].bullets[j]);

            Rectangle newBulletRec = { players[i].bullets[j].p.pos.x, players[i].bullets[j].p.pos.y, players[i].bullets[j].p.size.x * 2 - 2, players[i].bullets[j].p.size.y * 2 - 2 };

            for (int b = 0; b < boxesLength; b++) {
                Rectangle envBox = { boxes[b].p.pos.x, boxes[b].p.pos.y, boxes[b].p.size.x, boxes[b].p.size.y };
                CollisionPhysic(&players[i].bullets[j].p, newBulletRec, envBox);
                

            }
            // Stops the ball if it hits the wall when it is created
            if (players[i].bullets[j].isNew && players[i].bullets[j].p.collision[0]) {
                players[i].bullets[j].inactive = true;
            }
            players[i].bullets[j].isNew = false;

            for (int p = 0; p < playersLength; p++) {
                if(players[p].life <= 0) continue;
                Rectangle envPlayer = { players[p].p.pos.x, players[p].p.pos.y, players[p].p.size.x, players[p].p.size.y };
                CollisionBulletPlayer(&players[i].bullets[j], &players[p], envPlayer);
                CollisionPhysic(&players[i].bullets[j].p, newBulletRec, envPlayer);
            }

            BulletBounce(&players[i].bullets[j]);
        }
        
        // Collision
        Rectangle newPlayerRec = { players[i].p.pos.x, players[i].p.pos.y, players[i].p.size.x, players[i].p.size.y };
        for (int j = 0; j < playersLength; j++) {
            if(players[j].life <= 0) continue;
            if (players[i].id != players[j].id) {
                Rectangle envPlayer = { players[j].p.pos.x, players[j].p.pos.y, players[j].p.size.x, players[j].p.size.y };
                CollisionPhysic(&players[i].p, newPlayerRec, envPlayer);
            }
        }

        for (int j = 0; j < boxesLength; j++) {
            Rectangle envBox = { boxes[j].p.pos.x, boxes[j].p.pos.y, boxes[j].p.size.x, boxes[j].p.size.y };
            CollisionPhysic(&players[i].p, newPlayerRec, envBox);
        }

        centerPositionX += players[i].p.pos.x;
        centerPositionY += players[i].p.pos.y;
        // centerDistance += sqrtf(powf(camera.target.x - player.p.pos.x, 2) + powf(camera.target.x - player.p.pos.y, 2));
    }
    if (lastPlayer>=1) {
        camera.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
        centerPositionX = centerPositionX / playersLength;
        centerPositionY = centerPositionY / playersLength;
        camera.target = (Vector2){ centerPositionX + 20, centerPositionY + 20 };
    } else {
        camera.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
        camera.target = (Vector2){ players[7].p.pos.x, players[7].p.pos.y };
    }

    // Zoom out or in by distance
    // camera.zoom = (100 - centerDistance / 35) * 0.01;

    // Target Player 1
    // camera.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
    // camera.target = (Vector2){ players[0].p.pos.x, players[0].p.pos.y };

    // TraceLog(LOG_INFO, "centerDistance: %d", centerDistance);
    // TraceLog(LOG_INFO, "camera.zoom: %d", camera.zoom);
}

void DrawGameplay(void) {
    // DRAW GAME
    BeginMode2D(camera);
        render_map(map);
        DrawGameArena();

        // Draw Boxes
        for (int i = 0; i < boxesLength; i++) {
            DrawBox(boxes[i]);
        }

        // Draw Players / Bullets
        for (int i = 0; i < playersLength; i++) {
            for (int j = 0; j < sizeof(players[i].bullets)/sizeof(players[i].bullets[0]); j++) {
                DrawBullet(players[i].bullets[j]);
            }
            DrawPlayer(players[i]);
        }

        DrawPauseGame();
    EndMode2D();

    // DRAW STAT LOG INFO
    BeginDrawing();
        for (int i = 0; i < lastPlayer; i++) {
            DrawStatsPlayer(players[i]);
        }

        if (activeDev) {
            // DISPLAY FPS
            DrawFPS(10, 10);
            // DISPLAY ZOOM
            DrawText(TextFormat("ZOOM: %f", camera.zoom), 10, 30, 10, BLACK);
            DrawText(TextFormat("TARGET: %f/%f", camera.target.x, camera.target.y), 10, 40, 10, BLACK);
            DrawText(TextFormat("DELTA: %f", GetFrameTime()), 10, 50, 10, BLACK);

            // DISPLAY GAMEPAD
            DrawText(TextFormat("GP1: %s", GetGamepadName(0)), 10, 400, 10, BLACK);
            DrawText(TextFormat("GP2: %s", GetGamepadName(1)), 10, 410, 10, BLACK);
            DrawText(TextFormat("GP2: %s", GetGamepadName(3)), 10, 420, 10, BLACK);
            DrawText(TextFormat("GP2: %s", GetGamepadName(4)), 10, 430, 10, BLACK);

            DrawText(TextFormat("DETECTED AXIS [%i]:", GetGamepadAxisCount(0)), 10, 200, 10, MAROON);
            for (int i = 0; i < GetGamepadAxisCount(0); i++)
            {
                DrawText(TextFormat("AXIS %i: %.02f", i, GetGamepadAxisMovement(0, i)), 20, 220 + 20*i, 10, DARKGRAY);
            }
            if (GetGamepadButtonPressed() != -1) DrawText(TextFormat("DETECTED BUTTON: %i", GetGamepadButtonPressed()), 10, 440, 10, RED);
            else DrawText("DETECTED BUTTON: NONE", 10, 440, 10, GRAY);
        }
    EndDrawing();

}

void DrawGameArena(void) {
    // int arenaSize = 800;
    if (activeDev) {
        // DrawRectangleRec((Rectangle) { camera.target.x - GetScreenWidth() / 2, camera.target.y - GetScreenHeight() / 2, GetScreenWidth(), GetScreenHeight() }, BLACK);
        // // DrawCircleGradient(arenaSize/2 - 20, arenaSize/2 - 20, 2000, BLACK, SKYBLUE);
        // for (int i=0; i<=1500; i++) {
        //     DrawLine(-1000, i * 5 - 4000, 4000, i * 5 + 1000, BLACK); 
        //     DrawLine(i * 5 - 1000, -1000, i * 5 - 4000 - 1000, 4000 - 1000, BLACK); 
        // } 
            // DrawRectangleRec((Rectangle) { 0, 0, arenaSize, arenaSize }, WHITE);
        for (int i=0; i<=arenaSize*0.01; i++) {
            if (i < arenaSize*0.01) {
                DrawText(TextFormat("%d", i+1), i * 100 + 6, 4, 20, LIGHTGRAY);
                DrawText(TextFormat("%d", i+1), 6,  i * 100 + 4, 20, LIGHTGRAY);
            }
            Rectangle posViewX = { i * 100, 0, 2, arenaSize };
            Rectangle posViewY = { 0, i * 100, arenaSize, 2 };
            DrawRectangleRec(posViewX, LIGHTGRAY);
            DrawRectangleRec(posViewY, LIGHTGRAY);
        }
    }


    DrawRectangleRec((Rectangle) { -2, -2, 5, 5 }, BLACK); // 0,0
    DrawRectangleRec((Rectangle) { arenaSize - 1, arenaSize - 1, 5, 5 }, BLACK); // 1000, 1000
    DrawRectangleRec((Rectangle) { arenaSize - 1, -2, 5, 5 }, BLACK); // 1000, 0
    DrawRectangleRec((Rectangle) { -2, arenaSize - 1, 5, 5 }, BLACK); // 0, 1000

    DrawRectangleRec((Rectangle) { -2, -2, arenaSize + 1, 1 }, BLACK);
    DrawRectangleRec((Rectangle) { -2, -2, 1, arenaSize + 1 }, BLACK);
    DrawRectangleRec((Rectangle) { arenaSize + 3, -2, 1, arenaSize + 1 }, BLACK);
    DrawRectangleRec((Rectangle) { -2, arenaSize + 3, arenaSize + 1, 1 }, BLACK);
}

void DrawPauseGame(void) {
    if (pauseGame) {
        DrawRectangleRec((Rectangle) { camera.target.x - 480.0 / 2, camera.target.y - 240, 480, 400 }, Fade(LIGHTGRAY, 0.6));
        DrawText("RESTART (R)", camera.target.x - 8 * 11, camera.target.y - 200.0, 30, BLACK);
        DrawText("CONTROLLER (C)", camera.target.x - 8 * 14, camera.target.y - 100.0, 30, BLACK);
        DrawText("EDIT THE MAP (E)", camera.target.x - 8 * 16, camera.target.y - 0.0, 30, BLACK);
        DrawText("CONTINUE (P)", camera.target.x - 8 * 12, camera.target.y + 100.0, 30, BLACK);
    }
}
