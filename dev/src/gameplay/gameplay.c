#include "../../../lib/raylib/src/raylib.h"
#include "../../../lib/raylib/src/raymath.h"
#include <emscripten/emscripten.h>
#include <stdbool.h>
#include <string.h>
#include "stdio.h"

#include "../../../tmx/src/tmx.h"
#include "../tmx_raylib/tmx_raylib.h"

#include "../physic/physic.h"
#include "../player/player.h"
#include "../bullet/bullet.h"
#include "../box/box.h"
#include "gameplay.h"
#include "../../../lib/qrcode/c/qrcodegen.h"


EM_JS(char*, GetGamepadUrl, (), { 
    const byteCount = Module.lengthBytesUTF8(gamepadUrl)+1;
    const idPointer = Module._malloc(byteCount);
    Module.stringToUTF8(gamepadUrl, idPointer, byteCount);
    return idPointer;
});
EM_JS(int, GetCanvasWidthCustom, (), { return window.innerWidth });
EM_JS(int, GetCanvasHeightCustom, (), { return window.innerHeight });
EM_JS(int, ToggleInfoPeerJs, (), { return togglePeerJs() });
EM_JS(int, GetNumberPlayer, (), { return listGamepad.size });
EM_JS(char*, GetIdGamepad, (int index), { 
    let i = 0;
    let res = "";
    listGamepad.forEach((_,k) => { i === index ? res = k : 0; i++; });
    const byteCount = Module.lengthBytesUTF8(res)+1;
    const idPointer = Module._malloc(byteCount);
    Module.stringToUTF8(res, idPointer, byteCount);
    return idPointer;
});
EM_JS(int, InitColorGamepad, (char *p_id, int color_r, int color_g, int color_b), { 
    const id = Module.UTF8ToString(p_id);
    const gamepad = listGamepad.get(id);
    gamepad.color = `rgb(${color_r}, ${color_g}, ${color_b})`;
    gamepad.edit = true;
    listGamepad.set(id, gamepad);
    return 1;
});


tmx_map *map;

// float arenaSize = 32.0f * 32;
float arenaSizeX = 58.0f * 32;
float arenaSizeY = 32.0f * 32;
bool activeDev = false;
double lastSecond = 0;
static Camera2D camera = { 0 };
static bool pauseGame = 0;

double startTime = 0.0;
double elapsedTime = 0.0;

int playerAlive = 0;
int playerAliveId = 0;
Color ColorAlive;
bool OtherColorAlive = false;

int lastPlayer = 0;
bool playerSpace[8] = { 0 };
Color themeColor[8] = {
    { 35, 235, 141, 255}, // GREEN
    { 241, 82, 91, 255}, // RED
    { 64, 230, 230, 255}, // BLUE
    { 127, 102, 242, 255}, // PURPLE
    { 241, 209, 37, 255}, // YELLOW
    { 255, 130, 47, 255}, // ORANGE
    { 255, 149, 229, 255}, // PINK
    { 101, 126, 255, 255} // DISCORD COLOR x)
};

bool qrCodeOk = false;
Texture2D qrCodeTexture;

static Player players[8] = {
    // { 
    //     1, // ID
    //     "1", // Gamepad Id
    //     1, // Life
    //     0, // Invincible
    //     155, // Damages Taken
    //     3, // Ammunition
    //     600, // Ammunition loading
    //     {
    //         { 0, 0 }, // Position
    //         { 40, 40 }, // Size
    //         { 0.0, 0.0 }, // Velocity
    //         { 0, 0, 0, 0, 0 } // Collision: IsCollision, Up, Down, Left, Right
    //     },
    //     { 0, 0 }, // Spawn
    //     { 3.5, 3.5 }, // Speed
    //     2, // Charge 
    //     true, // Can Shoot 
    //     0, // Time Shoot
    //     0, // Radian
    //     { 0 }, // Bullets
    //     0, // Last Bullet
    //     { GREEN, LIME, DARKGREEN },
    //     MOUSE, // Input Type
    //     { KEY_Z, KEY_S, KEY_Q, KEY_D, KEY_G, KEY_F, KEY_H }, // KEY: Up, Down, Left, Right, MOVE CANNON, SHOT, MOVE CANNON
    // },
    // {   
    //     2, "2", 1, 0, 0, 3, 600, {{0, 0}, {40, 40}, {0, 0}, {0, 0, 0, 0, 0}}, { 0, 0 }, {3.5, 3.5}, 2, true, 0, 0, { 0 }, 0, { PINK, RED, MAROON }, GAMEPAD,
    //     {GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_RIGHT_X, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, GAMEPAD_AXIS_RIGHT_Y},
    // },
    {},{},{},{},{},{},{},{}
    // {   
    //     3, "1", 3, 0, 0, 1, 300, {{200 - 20, 600 - 20}, {40, 40}, {0, 0}, {0, 0, 0, 0, 0}}, { 0, 0 }, {3.5, 3.5}, 2, true, 0, 0, { 0 }, 0, { SKYBLUE, BLUE, DARKBLUE }, GAMEPAD,
    //     {GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_RIGHT_X, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, GAMEPAD_AXIS_RIGHT_Y},
    // },
    // {   
    //     4, "4", 3, 0, 155, 1, 300, {{600 - 20, 600 - 20}, {40, 40}, {0, 0}, {0, 0, 0, 0, 0}}, { 0, 0 }, {3.5, 3.5}, 2, true, 0, 0, { 0 }, 0, { PURPLE, VIOLET, DARKPURPLE }, MOBILE,
    //     {GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_RIGHT_X, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, GAMEPAD_AXIS_RIGHT_Y},
    // },
};
static int playersLength = sizeof(players)/sizeof(players[0]);

static Box boxes[40] = {};
static int boxesLength = sizeof(boxes)/sizeof(boxes[0]);


void InitGameplay(void) {
	tmx_img_load_func = raylib_tex_loader;
	tmx_img_free_func = raylib_free_tex;

	map = tmx_load("resources/map4-team.tmx");
	if (!map) {
		tmx_perror("Cannot load map");
	}

    tmx_init_object(map->ly_head, players, boxes);
    InitPlayer();

    camera.target = (Vector2){ 0, 0 };
    camera.offset = (Vector2){ 0, 0 };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
}

void UpdateGameplay(void) {
    int centerPositionX = 0;
    int centerPositionY = 0;
    // int centerDistance = 0;

    playerAlive = 0;
    playerAliveId = 0;
    OtherColorAlive = false;

    double time = GetTime();
    if ((int)time == lastSecond) {
        SetWindowSize(GetCanvasWidthCustom(), GetCanvasHeightCustom());
        camera.target = (Vector2){ 0, 0 };
        camera.offset = (Vector2){ GetScreenWidth() - arenaSizeX/2 - GetScreenWidth()/2, GetScreenHeight() - arenaSizeY/2 - GetScreenHeight()/2 };
        lastSecond += 1;
    }

    int numberPlayer = GetNumberPlayer(); // +2
    if(lastPlayer < numberPlayer) {
        for (int i = 0; i < playersLength; i++) {
            if(!playerSpace[i]) {
                playerSpace[i] = true;
                players[i] = (Player) {
                    i+1, GetIdGamepad(i), 1, 0, 0, 1, 600, {{600 - 20, 600 - 20}, {40, 40}, {0, 0}, {0, 0, 0, 0, 0}}, { 0, 0 }, {3.5, 3.5}, 2, true, 0, 0, { 0 }, 0, { PURPLE, VIOLET, DARKPURPLE }, MOBILE,
                    {GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_RIGHT_X, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, GAMEPAD_AXIS_RIGHT_Y},
                };
                tmx_init_object(map->ly_head, players, boxes);
                InitColorGamepad(players[i].gamepadId, players[i].COLORS[0].r, players[i].COLORS[0].g, players[i].COLORS[0].b);
                i = playersLength;
                lastPlayer++;
            }
        }
    }

    if(lastPlayer > numberPlayer) {
        // @Todo remove player / disconnect
    }

    GenerateQrCode();

    // Zoom out / zoom in with the mouse wheel
    camera.zoom += ((float)GetMouseWheelMove()*0.01f);

    // Active developer mode
    if (IsKeyPressed(KEY_O)) activeDev = !activeDev;

    // Display info of PeerJS
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
        // return;
    }

    // Reset
    if (IsKeyPressed(KEY_R)) {
        // Press R to reset
        tmx_init_object(map->ly_head, players, boxes);
        for (int i = 0; i < playersLength; i++) {
            if(playerSpace[i]) {
                players[i].life = 1;
                players[i].id = i+1;
            }
        }
    }

    // Update Players / Bullets
    for (int i = 0; i < playersLength; i++) {
        if(!players[i].id) continue;
        if(players[i].life > 0) {
            playerAlive++;
            playerAliveId = i;
            if (ColorToInt(players[i].COLORS[0]) != ColorToInt(ColorAlive)) {
                OtherColorAlive = true;
            }
            ColorAlive = players[i].COLORS[0];
        }

        UpdatePlayer(&players[i]);

        // Bullets
        if (pauseGame) continue;
        for (int j = 0; j < sizeof(players[i].bullets)/sizeof(players[i].bullets[0]); j++) {
            UpdateBullet(&players[i].bullets[j]);
            if(!players[i].bullets[j].playerId) continue;

            Rectangle newBulletRec = { players[i].bullets[j].p.pos.x + 2, players[i].bullets[j].p.pos.y + 2, players[i].bullets[j].p.size.x * 2 - 5, players[i].bullets[j].p.size.y * 2 - 5 };
            // Collision Bullet and Box
            for (int b = 0; b < boxesLength; b++) {
                Rectangle envBox = { boxes[b].p.pos.x, boxes[b].p.pos.y, boxes[b].p.size.x, boxes[b].p.size.y };
                CollisionPhysic(&players[i].bullets[j].p, newBulletRec, envBox);
            }
            // Stops the ball if it hits the wall when it is created
            if (players[i].bullets[j].isNew && players[i].bullets[j].p.collision[0]) {
                players[i].bullets[j].inactive = true;
            }
            players[i].bullets[j].isNew = false;

            // Collision Player and Bullet
            for (int p = 0; p < numberPlayer; p++) {
                Rectangle envPlayer = { players[p].p.pos.x, players[p].p.pos.y, players[p].p.size.x, players[p].p.size.y };
                bool bulletCollision = CollisionPhysic(&players[i].bullets[j].p, newBulletRec, envPlayer);
                CollisionBulletPlayer(bulletCollision, &players[i].bullets[j], &players[p], envPlayer);
            }
            BulletBounce(&players[i].bullets[j]);
        }
        
        // Collision Player and Player
        Rectangle newPlayerRec = { players[i].p.pos.x, players[i].p.pos.y, players[i].p.size.x, players[i].p.size.y };
        for (int j = 0; j < playersLength; j++) {
            if(players[j].life <= 0 || !players[i].id || !players[j].id) continue;
            if (players[i].id != players[j].id) {
                Rectangle envPlayer = { players[j].p.pos.x, players[j].p.pos.y, players[j].p.size.x, players[j].p.size.y };
                CollisionPhysic(&players[i].p, newPlayerRec, envPlayer);
            }
        }

        // Collision Player and Box
        for (int j = 0; j < boxesLength; j++) {
            if(!boxes[j].id) continue;
            Rectangle envBox = { boxes[j].p.pos.x, boxes[j].p.pos.y, boxes[j].p.size.x, boxes[j].p.size.y };
            CollisionPhysic(&players[i].p, newPlayerRec, envBox);
        }

        centerPositionX += players[i].p.pos.x;
        centerPositionY += players[i].p.pos.y;
        // centerDistance += sqrtf(powf(camera.target.x - player.p.pos.x, 2) + powf(camera.target.x - player.p.pos.y, 2));
    }

    if (lastPlayer>=2) {
        camera.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
        centerPositionX = centerPositionX / numberPlayer;
        centerPositionY = centerPositionY / numberPlayer;
        camera.target = (Vector2){ centerPositionX, centerPositionY };
    } else {
        camera.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
        camera.target = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
        // camera.target = (Vector2){ players[7].p.pos.x, players[7].p.pos.y };
    }

    if((playerAlive <= 1 || !OtherColorAlive) && lastPlayer > 1) {
        if (startTime == 0.0) {
            startTime = GetTime();
        }
        elapsedTime = GetTime() - startTime;
        if (elapsedTime > 3) {
            tmx_init_object(map->ly_head, players, boxes);
            for (int i = 0; i < playersLength; i++) {
                if(playerSpace[i]) {
                    players[i].life = 1;
                    players[i].id = i+1;
                }
            }
        }
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
            if(!players[i].id) continue;
            DrawSpawnPlayer(players[i]);
            for (int j = 0; j < sizeof(players[i].bullets)/sizeof(players[i].bullets[0]); j++) {
                if(!players[i].bullets[j].playerId) continue;
                DrawBullet(players[i].bullets[j]);
            }
            DrawPlayer(players[i]);
        }

        DrawPauseGame();
        if((playerAlive <= 1 || !OtherColorAlive) && lastPlayer > 1) {
            DrawCircle(camera.target.x, camera.target.y-100, 50, BLACK);
            DrawCircle(camera.target.x, camera.target.y-100, 48, WHITE);
            DrawCircle(camera.target.x, camera.target.y-100, 40, players[playerAliveId].COLORS[0]);
            DrawText("WIN THIS GAME", camera.target.x-50*6.5, camera.target.y, 80, players[playerAliveId].COLORS[0]);
        }
    EndMode2D();

    // DRAW STAT LOG INFO
    BeginDrawing();
        for (int i = 0; i < 8; i++) {
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
    // if (activeDev) {
        // DrawRectangleRec((Rectangle) { camera.target.x - GetScreenWidth() / 2, camera.target.y - GetScreenHeight() / 2, GetScreenWidth(), GetScreenHeight() }, BLACK);
        // // DrawCircleGradient(arenaSize/2 - 20, arenaSize/2 - 20, 2000, BLACK, SKYBLUE);
        // for (int i=0; i<=1500; i++) {
        //     DrawLine(-1000, i * 5 - 4000, 4000, i * 5 + 1000, BLACK); 
        //     DrawLine(i * 5 - 1000, -1000, i * 5 - 4000 - 1000, 4000 - 1000, BLACK); 
        // } 
        for (int x=0; x<=arenaSizeX*0.01; x++) {
            if (x < arenaSizeX*0.01) {
                DrawText(TextFormat("%d", x+1), x * 100 + 6, 4, 20, LIGHTGRAY);
            }
            // Rectangle posViewX = { 0, x * 100, arenaSizeX, 2 };
            Rectangle posViewX = { x * 100, 0, 2, arenaSizeY };
            DrawRectangleRec(posViewX, LIGHTGRAY);

        }

        for (int y=0; y<=arenaSizeY*0.01; y++) {
            if (y < arenaSizeY*0.01) {
                DrawText(TextFormat("%d", y+1), 6,  y * 100 + 4, 20, LIGHTGRAY);
            }
            // Rectangle posViewY = { y * 100, 0, 2, arenaSizeY };
            Rectangle posViewY = { 0, y * 100, arenaSizeX, 2 };
            DrawRectangleRec(posViewY, LIGHTGRAY);
        }
    // }

    DrawRectangleRec((Rectangle) { -2, -2, 5, 5 }, BLACK); // 0,0
    DrawRectangleRec((Rectangle) { arenaSizeX - 1, arenaSizeY - 1, 5, 5 }, BLACK); // 1000, 1000
    DrawRectangleRec((Rectangle) { arenaSizeX - 1, -2, 5, 5 }, BLACK); // 1000, 0
    DrawRectangleRec((Rectangle) { -2, arenaSizeY - 1, 5, 5 }, BLACK); // 0, 1000

    DrawRectangleRec((Rectangle) { -2, -2, arenaSizeX + 1, 1 }, BLACK);
    DrawRectangleRec((Rectangle) { -2, -2, 1, arenaSizeY + 1 }, BLACK);
    DrawRectangleRec((Rectangle) { arenaSizeX + 3, -2, 1, arenaSizeY + 1 }, BLACK);
    DrawRectangleRec((Rectangle) { -2, arenaSizeY + 3, arenaSizeX + 1, 1 }, BLACK);
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

void GenerateQrCode(void) {
    if(!qrCodeOk) {
        // Generate Image QrCode
        const char *url = GetGamepadUrl();
        if(url[0] != 'n') {
            enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;  // Error correction level
            uint8_t qrCode[qrcodegen_BUFFER_LEN_MAX];
            uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
            qrCodeOk = qrcodegen_encodeText(url, tempBuffer, qrCode, errCorLvl,
                qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
            if(qrCodeOk) {
                int qrCodeSize = qrcodegen_getSize(qrCode);
                int border = 8;
                int sizeRec = 256.f;
                sizeRec -= border*2;
                int t = sizeRec / qrCodeSize;
                int c = 0;
                Image qrCodeImage = GenImageColor(t*qrCodeSize+border*2, t*qrCodeSize+border*2, WHITE);
                for (int x = 0; x < qrCodeSize; x++) {
                    for (int y = 0; y < qrCodeSize; y++) {
                        if (qrcodegen_getModule(qrCode, x, y)) {
                            if (c>7) c=0;
                            if (((y<7 && x>qrCodeSize-8)) || 
                                (y>qrCodeSize-8 && y<qrCodeSize && x<7) || 
                                (x<7 && y<7) || 
                                ((x>qrCodeSize-10 && y>qrCodeSize-10) && (x<qrCodeSize-4 && y<qrCodeSize-4))) {
                                // ImageDrawRectangle(&qrCodeImage, x*t+border, y*t+border, t, t, GRAY);
                                ImageDrawRectangle(&qrCodeImage, x*t+border, y*t+border, t, t, BLACK);
                            }
                            else {
                                // ImageDrawRectangle(&qrCodeImage, x*t+border, y*t+border, t, t, themeColor[c]);
                                ImageDrawRectangle(&qrCodeImage, x*t+border, y*t+border, t, t, BLACK);
                            }
                        }
                        c++;
                    }
                }
                ImageResize(&qrCodeImage, sizeRec, sizeRec);
                qrCodeTexture = LoadTextureFromImage(qrCodeImage);
                UnloadImage(qrCodeImage);
                TraceLog(LOG_INFO, "Generate QrCode Ok.");
            }
        }
    }
}
