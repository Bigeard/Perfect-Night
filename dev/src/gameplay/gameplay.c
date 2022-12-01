#include "raylib.h"
#include "raymath.h"
#include "stdio.h"

#include "../physic/physic.h"
#include "../player/player.h"
#include "../bullet/bullet.h"
#include "../box/box.h"
#include "gameplay.h"


static bool activeDev = true;

static Camera2D camera = { 0 };
static bool pauseGame = 0;
static Player players[] = {
    { 
        1, // ID
        0, // Gamepad Id
        3, // Life
        155, // Damages Taken
        {
            { 200 - 20, 200 - 20 }, // Position
            { 40, 40 }, // Size
            { 0, 0 }, // Velocity
            { 0, 0, 0, 0, 0 } // Collision: IsCollision, Up, Down, Left, Right
        },
        { 3.5, 3.5 }, // Speed
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
    {   
        2, 0, 3, 0, {{600 - 20, 200 - 20}, {40, 40}, {0, 0}, {0, 0, 0, 0, 0}}, {3.5, 3.5}, 2, true, 0, 0, { 0 }, 0, { PINK, RED, MAROON }, GAMEPAD,
        {GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_RIGHT_X, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, GAMEPAD_AXIS_RIGHT_Y},
    },
    {   
        3, 1, 3, 0, {{200 - 20, 600 - 20}, {40, 40}, {0, 0}, {0, 0, 0, 0, 0}}, {3.5, 3.5}, 2, true, 0, 0, { 0 }, 0, { SKYBLUE, BLUE, DARKBLUE }, GAMEPAD,
        {GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_RIGHT_X, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, GAMEPAD_AXIS_RIGHT_Y},
    },
    {   
        4, 2, 3, 155, {{600 - 20, 600 - 20}, {40, 40}, {0, 0}, {0, 0, 0, 0, 0}}, {3.5, 3.5}, 2, true, 0, 0, { 0 }, 0, { PURPLE, VIOLET, DARKPURPLE }, GAMEPAD,
        {GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_RIGHT_X, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, GAMEPAD_AXIS_RIGHT_Y},
    },
};
static Box boxes[] = {
    {{{400, 150}, {100, 100}, {0, 0}}, GRAY },
    {{{100, 300}, {100, 100}, {0, 0}}, GRAY },
    {{{250, 350}, {100, 100}, {0, 0}}, GRAY },
    {{{200, 300}, {100, 25}, {0, 0}}, GRAY },
    {{{450, 450}, {100, 100}, {0, 0}}, GRAY },
    {{{550, 250}, {100, 100}, {0, 0}}, GRAY },
    {{{550, 350}, {100, 100}, {0, 0}}, GRAY },
    {{{450, 650}, {100, 100}, {0, 0}}, GRAY },
    {{{450, 550}, {100, 100}, {0, 0}}, GRAY },
    {{{550, 450}, {100, 100}, {0, 0}}, GRAY },
    {{{650, 450}, {100, 100}, {0, 0}}, GRAY },
};

static int playersLength = sizeof(players)/sizeof(players[0]);
static int boxesLength = sizeof(boxes)/sizeof(boxes[0]);


void InitGameplay(void) {
    camera.target = (Vector2){ GetScreenWidth()/2.0f - 100, GetScreenHeight()/2.0f - 100 };
    camera.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
}

void UpdateGameplay(void) {
    int centerPositionX = 0;
    int centerPositionY = 0;
    // int centerDistance = 0;

    camera.zoom += ((float)GetMouseWheelMove()*0.01f);

    // Pause
    if (IsKeyPressed(KEY_O)) activeDev = !activeDev;

    // Pause
    if (IsKeyPressed(KEY_P)) pauseGame = !pauseGame;
    if (pauseGame) return;

    // Reset
    if (IsKeyPressed(KEY_R)) {
        // Press R to reset
    }

    // Update Players / Bullets
    for (int i = 0; i < playersLength; i++) {
        // Bullets
        for (int j = 0; j < sizeof(players[i].bullets)/sizeof(players[i].bullets[0]); j++) {
            UpdateBullet(&players[i].bullets[j]);

            Rectangle newBulletRec = { players[i].bullets[j].p.pos.x, players[i].bullets[j].p.pos.y, players[i].bullets[j].p.size.x * 2, players[i].bullets[j].p.size.y * 2 };

            for (int b = 0; b < boxesLength; b++) {
                Rectangle envBox = { boxes[b].p.pos.x, boxes[b].p.pos.y, boxes[b].p.size.x, boxes[b].p.size.y };
                CollisionPhysic(&players[i].bullets[j].p, newBulletRec, envBox);
            }

            for (int p = 0; p < playersLength; p++) {
                Rectangle envPlayer = { players[p].p.pos.x, players[p].p.pos.y, players[p].p.size.x, players[p].p.size.y };
                CollisionPhysic(&players[i].bullets[j].p, newBulletRec, envPlayer);
                CollisionBulletPlayer(&players[i].bullets[j], &players[p], envPlayer);
            }

            BulletBounce(&players[i].bullets[j]);
        }
        
        UpdatePlayer(&players[i]);

        // Collision
        Rectangle newPlayerRec = { players[i].p.pos.x, players[i].p.pos.y, players[i].p.size.x, players[i].p.size.y };
        for (int j = 0; j < playersLength; j++) {
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
    centerPositionX = centerPositionX / playersLength;
    centerPositionY = centerPositionY / playersLength;
    camera.target = (Vector2){ centerPositionX + 20, centerPositionY + 20 };

    // Zoom out or in by distance
    // camera.zoom = (100 - centerDistance / 35) * 0.01;

    // Target Player 1
    // camera.target = (Vector2){ players[0].p.pos.x, players[0].p.pos.y };

    // TraceLog(LOG_INFO, "centerDistance: %d", centerDistance);
    // TraceLog(LOG_INFO, "camera.zoom: %d", camera.zoom);
}

void DrawGameplay(void) {
    // DRAW GAME
    BeginMode2D(camera);
        ClearBackground(WHITE);
        DrawGamearena();

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

    // DROW STAT LOG INFO
    BeginDrawing();
        for (int i = 0; i < playersLength; i++) {
            DrawStatsPlayer(players[i]);
        }

        if (activeDev) {
            // DISPLAY FPS
            DrawFPS(10, 10);
            // DISPLAY ZOOM
            DrawText(TextFormat("ZOOM: %f", camera.zoom), 10, 30, 10, BLACK);
            DrawText(TextFormat("DELTA: %f", GetFrameTime()), 10, 40, 10, BLACK);

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

void DrawGamearena(void) {
    int arenaSize = 800;
    if (!activeDev) {
        DrawCircleGradient(arenaSize/2 - 20, arenaSize/2 - 20, 2000, BLACK, SKYBLUE);
        for (int i=0; i<=1500; i++) {
            DrawLine(-1000, i * 5 - 4000, 4000, i * 5 + 1000, BLACK); 
            DrawLine(i * 5 - 1000, -1000, i * 5 - 4000 - 1000, 4000 - 1000, BLACK); 
        } 
    }

    DrawRectangleRec((Rectangle) { 0, 0, arenaSize, arenaSize }, WHITE);
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
        DrawRectangleRec((Rectangle) { camera.target.x - 480 / 2, camera.target.y - 120 / 2, 480, 120 }, Fade(LIGHTGRAY, 0.6));
        DrawText("PRESS 'P' TO CONTINUE", camera.target.x - 190, camera.target.y - 15, 30, BLACK);
    }
}
