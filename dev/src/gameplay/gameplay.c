#include "../../../lib/raylib/src/raylib.h"
#include "../../../lib/raylib/src/raymath.h"
#include <emscripten/emscripten.h>

#include "gameplay.h"
#include "../physic/physic.h"
#include "../player/player.h"
#include "../bullet/bullet.h"
#include "../box/box.h"
#include "../loot/loot.h"
#include "../item/item.h"
#include "../item/items/multi_shot/multi_shot.h"
#include "../particle/particle.h"
#include "../tool/tool.h"

#include "../../../tmx/src/tmx.h"
#include "../tmx_raylib/tmx_raylib.h"

#include "../../../lib/qrcode/c/qrcodegen.h"

#define PLAYERS_LENGTH 8

// Interaction with Javascript
// More info https://emscripten.org/docs/porting/connecting_cpp_and_javascript/Interacting-with-code.html

// Get URL to generate QrCode
EM_JS(char *, GetGamepadUrl, (), {
    const byteCount = Module.lengthBytesUTF8(gamepadUrl) + 1;
    const idPointer = Module._malloc(byteCount);
    Module.stringToUTF8(gamepadUrl, idPointer, byteCount);
    return idPointer;
});

// Resize the screen if it is too small or too large.
EM_JS(int, GetCanvasWidthCustom, (), {return window.innerWidth});
EM_JS(int, GetCanvasHeightCustom, (), {return window.innerHeight});

// Manage Gamepad player
EM_JS(int, GetNumberPlayer, (), {return listGamepad.size});
EM_JS(char *, GetIdGamepad, (int index), {
    let i = 0;
    let res = "";
    listGamepad.forEach(function(_, k) { i === index ? res = k : 0; i++; });
    const byteCount = Module.lengthBytesUTF8(res) + 1;
    const idPointer = Module._malloc(byteCount);
    Module.stringToUTF8(res, idPointer, byteCount);
    return idPointer;
});
EM_JS(int, GamepadPlayerColor, (char *p_id, int color_r, int color_g, int color_b), {
    const id = Module.UTF8ToString(p_id);
    const gamepad = listGamepad.get(id);
    gamepad.color = `rgb(${color_r}, ${color_g}, ${color_b})`;
    gamepad.edit = true;
    listGamepad.set(id, gamepad);
    return 1;
});

// Gameplay
tmx_map *map;

float arenaSizeX = 0.0f;
float arenaSizeY = 0.0f;
bool activeDev = false;
double lastSecond = 0.0;
static Camera2D camera = {0};
static bool pauseGame = false;

double startTime = 0.0;
double elapsedTime = 0.0;

// Home Screen
Texture titlePerfectNightTexture;
Texture useSameWifiTexture;
Texture andScanQrTexture;

// Player
int lastPlayer = 0;
bool playerSpace[8] = {0};
Color themeColor[8] = {
    {35, 235, 141, 255},  // GREEN
    {241, 82, 91, 255},   // RED
    {64, 230, 230, 255},  // BLUE
    {127, 102, 242, 255}, // PURPLE
    {241, 209, 37, 255},  // YELLOW
    {255, 130, 47, 255},  // ORANGE
    {255, 149, 229, 255}, // PINK
    {101, 126, 255, 255}  // DISCORD COLOR x)
};
int ColorScore[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
int numberActiveColor = 0;

int playerAlive = 0;
int playerAliveId = 0;
Color ColorAlive;
bool OtherColorAlive = false;

Player players[PLAYERS_LENGTH] = {{}, {}, {}, {}, {}, {}, {}, {}};
int numberPlayer = 0;

Player *outsidePlayer;
Player *lastOutsidePlayer;
double startTimeOutside = 0.0;
double elapsedTimeOutside = 0.0;

// QrCode
bool qrCodeOk = false;
Texture2D qrCodeTexture;

// Boxes
Box boxes[40] = {};
static int boxesLength = sizeof(boxes) / sizeof(boxes[0]);

// Loots
Loot loots[4] = {};
static int lootsLength = sizeof(loots) / sizeof(loots[0]);

// Items
Texture BonusAmmunitionTexture;
Texture BonusLifeTexture;
Texture BonusSpeedTexture;

void InitGameplay(void)
{
    // Home Screen
    // Resize Image for reduce the size of the game
    Image titlePerfectNightImage = LoadImage("resources/title_perfect_night.png");
    ImageResizeNN(&titlePerfectNightImage, 155 * 7, 75 * 7);
    titlePerfectNightTexture = LoadTextureFromImage(titlePerfectNightImage);
    UnloadImage(titlePerfectNightImage);

    Image useSameWifiImage = LoadImage("resources/use_the_same_wifi.png");
    ImageResizeNN(&useSameWifiImage, 159 * 2.5, 119 * 2.5);
    useSameWifiTexture = LoadTextureFromImage(useSameWifiImage);
    UnloadImage(useSameWifiImage);

    Image andScanQrImage = LoadImage("resources/and_scan.png");
    ImageResizeNN(&andScanQrImage, 162 * 2.5, 112 * 2.5);
    andScanQrTexture = LoadTextureFromImage(andScanQrImage);
    UnloadImage(andScanQrImage);

    // Items
    BonusAmmunitionTexture = LoadTexture("resources/bonus_ammunition.png");
    BonusLifeTexture = LoadTexture("resources/bonus_life.png");
    BonusSpeedTexture = LoadTexture("resources/bonus_speed.png");

    // Load TMX
    tmx_img_load_func = raylib_tex_loader;
    tmx_img_free_func = raylib_free_tex;

    // map = tmx_load("resources/map_vs.tmx");
    map = tmx_load("resources/map_2_team.tmx");
    // map = tmx_load("resources/map_4_team.tmx");
    if (!map)
    {
        tmx_perror("Cannot load map");
    }
    arenaSizeX = map->tile_width * map->width;
    arenaSizeY = map->tile_height * map->height;
    tmx_init_object(map->ly_head, players, boxes, loots);

    // Init
    InitPlayer();
    InitLoot();

    // Init Camera
    camera.target = (Vector2){0.0f, 0.0f};
    camera.offset = (Vector2){0.0f, 0.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
}

void UpdateGameplay(void)
{
    int centerPositionX = 0;
    int centerPositionY = 0;
    // int centerDistance = 0;

    playerAlive = 0;
    playerAliveId = 0;
    OtherColorAlive = false;

    double time = GetTime();
    if ((int)time == lastSecond)
    {
        SetWindowSize(GetCanvasWidthCustom(), GetCanvasHeightCustom());
        camera.target = (Vector2){0.0f, 0.0f};
        camera.offset = (Vector2){GetScreenWidth() - arenaSizeX / 2.0f - GetScreenWidth() / 2.0f, GetScreenHeight() - arenaSizeY / 2.0f - GetScreenHeight() / 2.0f};
        lastSecond += 1;
    }
    if (GetScreenWidth() < (arenaSizeX + 60.0f) * (camera.zoom / 1.0001f) || GetScreenHeight() < (arenaSizeY + 60.0f) * (camera.zoom / 1.0001f))
    {
        camera.zoom -= 0.001f;
    }
    if (GetScreenWidth() > (arenaSizeX + 120.0f) || GetScreenHeight() > (arenaSizeY + 120.0f))
    {
        camera.zoom += 0.001f;
    }

    numberPlayer = GetNumberPlayer(); // +2
    if (lastPlayer < numberPlayer)
    {
        for (int i = 0; i < PLAYERS_LENGTH; i++)
        {
            if (!playerSpace[i])
            {
                playerSpace[i] = true;
                players[i] = (Player){
                    i + 1,                                                                             // id: Identifier
                    GetIdGamepad(i),                                                                   // gamepadId: Gamepad identifier
                    1,                                                                                 // life: Number of life
                    DELAY_INVINCIBLE,                                                                  // invincible: Time of invincibility
                    MAX_AMMUNITION,                                                                    // ammunition: Ammunition
                    DELAY_AMMUNITION,                                                                  // ammunitionLoad: Ammunition loading
                    {{0.0f, 0.0f}, {40.0f, 40.0f}, {0.0f, 0.0f}, {false, false, false, false, false}}, // p: Physic
                    {0.0f, 0.0f},                                                                      // spawn: Spawn position
                    {3.5f, 3.5f},                                                                      // speed: Speed of the tank
                    // Bullet
                    0.0f, // charge: Charge delay
                    true, // canShoot: Can Shoot
                    0.0f, // timeShoot: Time Shoot
                    0.0f, // radian: Radian : Determine the position of the cannon
                    0.0f, // lastRadian: Last Radian : Determine the position of the cannon
                    {0},  // bullets: Array of bullet
                    0.0f, // lastBullet: Allow the ball to be replaced one after the other
                    // Other
                    PURPLE, // COLORS: Colors
                    {0},    // Item
                    // Control
                    MOBILE,                                                                                                                                                            // INPUT_TYPE: Type of input (mouse, keyboard, gamepad)
                    {GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_X, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_LEFT_Y, GAMEPAD_AXIS_RIGHT_X, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, GAMEPAD_AXIS_RIGHT_Y}, // KEY: Key you can press to move or do an action (@TODO play with controller)
                    {0}                                                                                                                                                                // shootParticle
                };
                ResetGame();
                for (int c = 0; c < sizeof(themeColor) / sizeof(themeColor[0]); c++)
                {
                    if (ColorToInt(themeColor[c]) == ColorToInt(players[i].color))
                    {
                        if (ColorScore[c] < 0)
                        {
                            numberActiveColor++;
                            ColorScore[c] = 0;
                        }
                        break;
                    }
                }
                i = PLAYERS_LENGTH;
                lastPlayer++;
            }
        }
    }

    if (lastPlayer > numberPlayer)
    {
        // @TODO remove player / disconnect
        // int listId = getPlayerDisconnected();
        // playerSpace[i] = false;
        // players[i] =
    }

    GenerateQrCode();

    // Zoom out / zoom in with the mouse wheel
    camera.zoom += ((float)GetMouseWheelMove() * 0.01f);

    // Active developer mode
    if (IsKeyPressed(KEY_O))
        activeDev = !activeDev;

    // Press R to reset
    if (IsKeyPressed(KEY_R))
    {
        for (int i = 0; i < PLAYERS_LENGTH; i++)
        {
            ColorScore[i] = -1;
        };
        ResetGame();
    }

    // Pause / @TODO
    if (IsKeyPressed(KEY_P))
        pauseGame = !pauseGame;
    if (pauseGame)
    {
        if (IsKeyPressed(KEY_C))
        {
            // Controller display
        }
        if (IsKeyPressed(KEY_E))
        {
            // Edit map display
        }
        // return;
    }
    if (pauseGame)
        return;

    // Update Players / Bullets
    for (int i = 0; i < PLAYERS_LENGTH; i++)
    {
        // If player not exist continue
        if (!players[i].id)
            continue;

        // Detect if one team win
        if (players[i].life > 0)
        {
            playerAlive++;
            playerAliveId = i;
            if (ColorToInt(players[i].color) != ColorToInt(ColorAlive))
            {
                OtherColorAlive = true;
            }
            ColorAlive = players[i].color;
        }

        UpdatePlayer(&players[i]);

        // Bullets
        for (int j = 0; j < sizeof(players[i].bullets) / sizeof(players[i].bullets[0]); j++)
        {
            if (!players[i].bullets[j].playerId || players[i].bullets[j].inactive)
                continue;

            UpdateBullet(&players[i].bullets[j]);

            Rectangle newBulletRec = {players[i].bullets[j].p.pos.x + 2.0f, players[i].bullets[j].p.pos.y + 2.0f, players[i].bullets[j].p.size.x * 2.0f - 5.0f, players[i].bullets[j].p.size.y * 2.0f - 5.0f};
            // Collision Bullet and Box
            for (int b = 0; b < boxesLength; b++)
            {
                Rectangle envBox = {boxes[b].p.pos.x, boxes[b].p.pos.y, boxes[b].p.size.x, boxes[b].p.size.y};
                CollisionPhysic(&players[i].bullets[j].p, newBulletRec, envBox);
            }
            // Stops the ball if it hits the wall when it is created
            if (players[i].bullets[j].isNew && players[i].bullets[j].p.collision[0])
            {
                players[i].bullets[j].inactive = true;
            }
            players[i].bullets[j].isNew = false;

            // Collision Player and Bullet
            for (int p = 0; p < numberPlayer; p++)
            {
                if (players[p].life <= 0)
                    continue;
                Rectangle envPlayer = {players[p].p.pos.x, players[p].p.pos.y, players[p].p.size.x, players[p].p.size.y};
                bool bulletCollision = CollisionPhysic(&players[i].bullets[j].p, newBulletRec, envPlayer);
                CollisionBulletPlayer(bulletCollision, &players[i].bullets[j], &players[p], envPlayer);
            }
            BulletBounce(&players[i].bullets[j]);
        }

        // Collision Player and Player
        Rectangle newPlayerRec = {players[i].p.pos.x, players[i].p.pos.y, players[i].p.size.x, players[i].p.size.y};
        for (int j = 0; j < PLAYERS_LENGTH; j++)
        {
            if (players[j].life <= 0 || !players[i].id || !players[j].id)
                continue;
            if (players[i].id != players[j].id)
            {
                Rectangle envPlayer = {players[j].p.pos.x, players[j].p.pos.y, players[j].p.size.x, players[j].p.size.y};
                CollisionPhysic(&players[i].p, newPlayerRec, envPlayer);
            }
        }

        // Collision Player and Box
        for (int j = 0; j < boxesLength; j++)
        {
            if (!boxes[j].id)
                continue;
            Rectangle envBox = {boxes[j].p.pos.x, boxes[j].p.pos.y, boxes[j].p.size.x, boxes[j].p.size.y};
            CollisionPhysic(&players[i].p, newPlayerRec, envBox);
        }

        for (int j = 0; j < lootsLength; j++)
        {
            UpdateLoot(&loots[j], &players[i]);
        }

        if (players[i].shootParticle[0].timer != 0)
        {
            UpdateParticles(players[i].shootParticle, 20);
        }

        centerPositionX += players[i].p.pos.x;
        centerPositionY += players[i].p.pos.y;
        // centerDistance += sqrtf(powf(camera.target.x - player.p.pos.x, 2) + powf(camera.target.x - player.p.pos.y, 2));
    }

    if (lastPlayer >= 2)
    {
        camera.offset = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
        centerPositionX = centerPositionX / numberPlayer;
        centerPositionY = centerPositionY / numberPlayer;
        camera.target = (Vector2){centerPositionX, centerPositionY};
    }
    else
    {
        camera.offset = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
        camera.target = (Vector2){arenaSizeX / 2.0f, arenaSizeY / 2.0f};
    }

    if ((outsidePlayer && !lastOutsidePlayer) || (outsidePlayer && lastOutsidePlayer && outsidePlayer->id != lastOutsidePlayer->id))
    {
        if (lastOutsidePlayer && ColorToInt(outsidePlayer->color) != ColorToInt(lastOutsidePlayer->color))
        {
            for (int i = 0; i < PLAYERS_LENGTH; i++)
            {
                if (ColorToInt(players[i].color) != ColorToInt(outsidePlayer->color))
                {
                    if ((players[i].p.pos.x >= arenaSizeX ||
                         players[i].p.pos.x + players[i].p.size.x <= 0.0f) ||
                        (players[i].p.pos.y >= arenaSizeY ||
                         players[i].p.pos.y + players[i].p.size.y <= 0.0f))
                    {

                        if (players[i].life > 0)
                        {
                            players[i].life--;
                            GamepadPlayerLife(players[i].gamepadId, players[i].life);
                        };
                        players[i].p.pos = (Vector2){players[i].spawn.x, players[i].spawn.y};
                    }
                }
            }
        }
        lastOutsidePlayer = outsidePlayer;
        startTimeOutside = GetTime();
    }
    if (startTimeOutside != 0.0)
    {
        elapsedTimeOutside = GetTime() - startTimeOutside;
    }
    if (elapsedTimeOutside > 2.0)
    {
        for (int i = 0; i < PLAYERS_LENGTH; i++)
        {
            if (ColorToInt(players[i].color) == ColorToInt(outsidePlayer->color))
            {
                if ((players[i].p.pos.x >= arenaSizeX ||
                     players[i].p.pos.x + players[i].p.size.x <= 0.0f) ||
                    (players[i].p.pos.y >= arenaSizeY ||
                     players[i].p.pos.y + players[i].p.size.y <= 0.0f))
                {

                    if (players[i].life > 0)
                    {
                        players[i].life--;
                        GamepadPlayerLife(players[i].gamepadId, players[i].life);
                    };
                    players[i].p.pos = (Vector2){players[i].spawn.x, players[i].spawn.y};
                }
            }
        }
        outsidePlayer = NULL;
        lastOutsidePlayer = NULL;
        startTimeOutside = 0.0;
    }
    if (!outsidePlayer)
    {
        lastOutsidePlayer = NULL;
        startTimeOutside = 0.0;
    }

    if (((playerAlive <= 1 || !OtherColorAlive) && lastPlayer > 1) || playerAlive == 0)
    {
        if (startTime == 0.0 && numberPlayer > 1)
        {
            for (int c = 0; c < sizeof(themeColor) / sizeof(themeColor[0]); c++)
            {
                if (ColorToInt(themeColor[c]) == ColorToInt(players[playerAliveId].color))
                {
                    if (ColorScore[c] > -1)
                    {
                        ColorScore[c]++;
                    }
                    break;
                }
            }
            for (int i = 0; i < PLAYERS_LENGTH; i++)
            {
                if (playerSpace[i])
                {
                    for (int j = 0; j < sizeof(players[i].bullets) / sizeof(players[i].bullets[0]); j++)
                    {
                        players[i].bullets[j].inactive = true;
                    }
                }
            }
            startTime = GetTime();
        }
        elapsedTime = GetTime() - startTime;
        if (elapsedTime > 3.0)
        {
            ResetGame();
            startTime = 0.0;
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

void ResetGame(void)
{
    tmx_init_object(map->ly_head, players, boxes, loots);
    for (int i = 0; i < PLAYERS_LENGTH; i++)
    {
        if (playerSpace[i])
        {
            players[i].life = 1;
            players[i].id = i + 1;
            players[i].ammunition = MAX_AMMUNITION;
            players[i].invincible = DELAY_INVINCIBLE;
            players[i].charge = 0.0f;
            players[i].item.active = false;
            players[i].speed.x = 3.5f;
            players[i].speed.y = 3.5f;
            for (int j = 0; j < sizeof(players[i].bullets) / sizeof(players[i].bullets[0]); j++)
            {
                players[i].bullets[j].inactive = true;
            }
            GamepadPlayerColor(players[i].gamepadId, players[i].color.r, players[i].color.g, players[i].color.b);
            GamepadPlayerLife(players[i].gamepadId, players[i].life);
            GamepadPlayerAmmunition(players[i].gamepadId, players[i].life);
        }
    }
}

void DrawGameplay(void)
{
    // DRAW GAME
    BeginMode2D(camera);
    if (outsidePlayer)
    {
        ClearBackground(DarkenColor(LightenColor(outsidePlayer->color, 0.2f), 1.0f - elapsedTimeOutside / 2.0f));
    }
    else
    {
        if (elapsedTimeOutside > 0)
            elapsedTimeOutside = elapsedTimeOutside - 0.01;
        ClearBackground(DarkenColor(int_to_color(map->backgroundcolor), 1.0f - elapsedTimeOutside / 2.0f));
    }
    render_map(map);
    DrawGameArena();

    // Draw Boxes
    for (int i = 0; i < boxesLength; i++)
    {
        DrawBox(boxes[i]);
    }

    // Draw Players / Bullets
    for (int i = 0; i < PLAYERS_LENGTH; i++)
    {
        if (!players[i].id)
            continue;
        DrawSpawnPlayer(players[i]);
        for (int j = 0; j < sizeof(players[i].bullets) / sizeof(players[i].bullets[0]); j++)
        {
            if (!players[i].bullets[j].playerId)
                continue;
            DrawBullet(players[i].bullets[j]);
        }
        DrawPlayer(players[i]);
        if (players[i].shootParticle[0].timer != 0)
        {
            DrawParticles(players[i].shootParticle, 20);
        }
    }

    for (int i = 0; i < lootsLength; i++)
    {
        DrawLoot(loots[i]);
    }

    DrawPauseGame();

    // Draw Wins
    if (startTime != 0.0)
    {
        int indexFindColor = 0;
        for (int i = 0; i < numberActiveColor; i++)
        {
            if (ColorScore[i] > -1)
            {
                // @TODO fix display if multi color team
                DrawText(TextFormat("%d", ColorScore[indexFindColor]), (int)(camera.target.x - 50.0f * 2.0f - 65.0f + 300.0f) * indexFindColor, (int)(camera.target.y - 80.0f), 80, themeColor[indexFindColor]);
            }
            else
                i--;
            indexFindColor++;
        }
        DrawCircle(camera.target.x, camera.target.y - 100.0f, 50.0f, BLACK);
        DrawCircle(camera.target.x, camera.target.y - 100.0f, 48.0f, WHITE);
        DrawCircle(camera.target.x, camera.target.y - 100.0f, 40.0f, players[playerAliveId].color);
        DrawText("WINS THIS GAME", (int)(camera.target.x - 50.0f * 7.0f), (int)(camera.target.y + 40.0f), 80, players[playerAliveId].color);
    }
    EndMode2D();

    // DRAW STAT LOG INFO
    BeginDrawing();
    // Draw title
    if (numberPlayer == 0)
    {
        float centerScreenX = GetScreenWidth() / 2.0f;
        float centerScreenY = GetScreenHeight() / 2.0f;
        Rectangle recBackground = { 0.0f, 0.0f, GetScreenWidth(), GetScreenHeight()};
        DrawRectangleRec(recBackground, Fade(BLACK, 0.4f));
        DrawTexture(titlePerfectNightTexture, centerScreenX - 155.0f * 3.47f, centerScreenY - 125.0f * 3.47f, WHITE);
        DrawTexture(useSameWifiTexture, centerScreenX - 205.0f * 3.47f, centerScreenY + 160.0f, WHITE);
        DrawTexture(andScanQrTexture, centerScreenX + 80.0f * 3.47f, centerScreenY + 180.0f, WHITE);
        DrawRectangleRec((Rectangle){centerScreenX - qrCodeTexture.width / 2.0f - 5.0f, centerScreenY - qrCodeTexture.height / 2.0f + 275.0f, qrCodeTexture.width + 10.0f, qrCodeTexture.height + 10.0f}, BLACK);
        DrawTexture(qrCodeTexture, centerScreenX - qrCodeTexture.width / 2.0f, centerScreenY - qrCodeTexture.height / 2.0f + 280.0f, WHITE);
    }

    for (int i = 0; i < PLAYERS_LENGTH; i++)
    {
        if (!players[i].id)
            continue;
        DrawStatsPlayer(players[i]);
    }

    if (activeDev)
    {
        // DISPLAY FPS
        DrawFPS(10, 10);

        // DISPLAY INFO
        DrawText(TextFormat("ZOOM: %f", camera.zoom), 10, 30, 10, BLACK);
        DrawText(TextFormat("TARGET: %f/%f", camera.target.x, camera.target.y), 10, 40, 10, BLACK);
        DrawText(TextFormat("DELTA: %f", GetFrameTime()), 10, 50, 10, BLACK);
    }
    EndDrawing();
}

void DrawGameArena(void)
{
    for (int x = 0; x <= (int)(arenaSizeX * 0.01f); x++)
    {
        if (x < (int)(arenaSizeX * 0.01f))
        {
            DrawText(TextFormat("%d", x + 1), x * 100 + 6, 4, 20, LIGHTGRAY);
        }
        // Rectangle posViewX = { 0, x * 100, arenaSizeX, 2 };
        Rectangle posViewX = {(float)(x * 100), 0.0f, 2.0f, arenaSizeY};
        DrawRectangleRec(posViewX, LIGHTGRAY);
    }

    for (int y = 0; y <= (int)(arenaSizeY * 0.01f); y++)
    {
        if (y < (int)(arenaSizeY * 0.01))
        {
            DrawText(TextFormat("%d", y + 1), 6, y * 100 + 4, 20, LIGHTGRAY);
        }
        // Rectangle posViewY = { y * 100, 0, 2, arenaSizeY };
        Rectangle posViewY = {0.0f, (int)(y * 100), arenaSizeX, 2.0f};
        DrawRectangleRec(posViewY, LIGHTGRAY);
    }

    DrawRectangleRec((Rectangle){-2.0f, -2.0f, 5.0f, 5.0f}, BLACK);
    DrawRectangleRec((Rectangle){arenaSizeX - 1.0f, arenaSizeY - 1.0f, 5.0f, 5.0f}, BLACK);
    DrawRectangleRec((Rectangle){arenaSizeX - 1.0f, -2.0f, 5.0f, 5.0f}, BLACK);
    DrawRectangleRec((Rectangle){-2.0f, arenaSizeY - 1.0f, 5.0f, 5.0f}, BLACK);

    DrawRectangleRec((Rectangle){-2.0f, -2.0f, arenaSizeX + 1.0f, 1.0f}, BLACK);
    DrawRectangleRec((Rectangle){-2.0f, -2.0f, 1.0f, arenaSizeY + 1.0f}, BLACK);
    DrawRectangleRec((Rectangle){arenaSizeX + 3.0f, -2.0f, 1.0f, arenaSizeY + 1.0f}, BLACK);
    DrawRectangleRec((Rectangle){-2.0f, arenaSizeY + 3.0f, arenaSizeX + 1.0f, 1.0f}, BLACK);
}

void DrawPauseGame(void)
{
    if (pauseGame)
    {
        DrawRectangleRec((Rectangle){camera.target.x - 480.0f / 2.0f, camera.target.y - 240.0f, 480.0f, 400.0f}, Fade(LIGHTGRAY, 0.6f));
        DrawText("RESTART (R)", (int)(camera.target.x - 8 * 11), (int)(camera.target.y - 200), 30, BLACK);
        DrawText("CONTROLLER (C)", (int)(camera.target.x - 8 * 14), (int)(camera.target.y - 100), 30, BLACK);
        DrawText("CHANGE MAP (E)", (int)(camera.target.x - 8 * 16), (int)(camera.target.y), 30, BLACK);
        DrawText("CONTINUE (P)", (int)(camera.target.x - 8 * 12), (int)(camera.target.y + 100), 30, BLACK);
    }
}

void GenerateQrCode(void)
{
    if (!qrCodeOk)
    {
        // Generate Image QrCode
        const char *url = GetGamepadUrl();
        if (url[0] != 'n')
        {
            enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW; // Error correction level
            uint8_t qrCode[qrcodegen_BUFFER_LEN_MAX];
            uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
            qrCodeOk = qrcodegen_encodeText(url, tempBuffer, qrCode, errCorLvl,
                                            qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
            if (qrCodeOk)
            {
                int qrCodeSize = qrcodegen_getSize(qrCode);
                int border = 8;
                int sizeRec = 256;
                sizeRec -= border * 2;
                int t = sizeRec / qrCodeSize;
                int c = 0;
                Image qrCodeImage = GenImageColor(t * qrCodeSize + border * 2, t * qrCodeSize + border * 2, WHITE);
                for (int x = 0; x < qrCodeSize; x++)
                {
                    for (int y = 0; y < qrCodeSize; y++)
                    {
                        if (qrcodegen_getModule(qrCode, x, y))
                        {
                            if (c > 7)
                                c = 0;
                            if (((y < 7 && x > qrCodeSize - 8)) ||
                                (y > qrCodeSize - 8 && y < qrCodeSize && x < 7) ||
                                (x < 7 && y < 7) ||
                                ((x > qrCodeSize - 10 && y > qrCodeSize - 10) && (x < qrCodeSize - 4 && y < qrCodeSize - 4)))
                            {
                                // ImageDrawRectangle(&qrCodeImage, x*t+border, y*t+border, t, t, GRAY);
                                ImageDrawRectangle(&qrCodeImage, x * t + border, y * t + border, t, t, BLACK);
                            }
                            else
                            {
                                // ImageDrawRectangle(&qrCodeImage, x*t+border, y*t+border, t, t, themeColor[c]);
                                ImageDrawRectangle(&qrCodeImage, x * t + border, y * t + border, t, t, BLACK);
                            }
                        }
                        c++;
                    }
                }
                ImageResizeNN(&qrCodeImage, sizeRec, sizeRec);
                qrCodeTexture = LoadTextureFromImage(qrCodeImage);
                UnloadImage(qrCodeImage);
                TraceLog(LOG_INFO, "Generate QrCode Ok.");
            }
        }
    }
}
