#include "../../../lib/raylib/src/raylib.h"
#include "../../../lib/raylib/src/raymath.h"
#include <emscripten/emscripten.h>
#include <stdlib.h>
#include <string.h>

#include "gameplay.h"
#include "../physic/physic.h"
#include "../player/player.h"
#include "../bullet/bullet.h"
#include "../box/box.h"
#include "../loot/loot.h"
#include "../item/item.h"
#include "../particle/particle.h"
#include "../tool/tool.h"

#include "../../../tmx/src/tmx.h"
#include "../tmx_raylib/tmx_raylib.h"

#include "../../../lib/qrcode/c/qrcodegen.h"

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
EM_JS(void, GamepadPlayerColor, (char *p_id, int color_r, int color_g, int color_b), {
    const id = Module.UTF8ToString(p_id);
    const gamepad = listGamepad.get(id);
    gamepad.color = `rgb(${color_r}, ${color_g}, ${color_b})`;
    gamepad.edit = true;
    listGamepad.set(id, gamepad);
});

// Menu Action
EM_JS(int, GetMenuAction, (), {
    return menuAction;
});
EM_JS(void, SetMenuAction, (int action), {
    menuAction = action;
});

// Settings
EM_JS(bool, GetEditSettings, (), {
    return gameSettings.edit;
});
EM_JS(int *, GetAllSettings, (), {
    const arrayPointer = Module._malloc(gameSettings.length * 4);
    Object.values(gameSettings).forEach(function(v, i) {
        Module.setValue(arrayPointer + i * 4, parseInt(v), "i32");
    });
    gameSettings.edit = false;
    return arrayPointer;
});

// Online
EM_JS(void, SendData, (char *data), {
    const dataToSend = Module.UTF8ToString(data);
    let i = 0;
    while (i < listScreenShareIndex)
    {
        listScreenShare[i].send(dataToSend);
        i++;
    }
});
EM_JS(char *, GetData, (), {
    if (!dataReceive)
        return;
    const byteCount = Module.lengthBytesUTF8(dataReceive) + 1;
    const dataPointer = Module._malloc(byteCount);
    Module.stringToUTF8(dataReceive, dataPointer, byteCount);
    return dataPointer;
});

// Gameplay
tmx_map *map;
int idMap = 0;
char *listMap[4] = {
    "resources/map_2_team_v2.tmx",
    "resources/map_vs.tmx",
    "resources/map_4_team.tmx",
    "resources/map_vs_big.tmx"};
bool winnerMap = false;

float arenaSizeX = 0.0f;
float arenaSizeY = 0.0f;
double lastSecond = 0.0;
static Camera2D camera = {0};
float centerPositionX = 0;
float centerPositionY = 0;
static bool pauseGame = false;

double startTime = 0.0;
double elapsedTime = 0.0;

// Setings
bool activeDev = false;
bool activePerf = false;
bool activeOnline = false;
bool activeMain = false;
int maxScore = 3;
int maxAmmunition = 4;
bool activeLoot = true;
float defaultTypeItem = -1;     // 0 = random / -1 = default item (none)
float defaultMaxTimerItem = -1; // 0 = no limit / -1 = default max timer

//// Example
// int defaultTypeItem = LASER; // 0 = random / -1 = default item (none)
// int defaultMaxTimerItem = 0; // 0 = no limit / -1 = default max timer

// Homepage
Texture2D titlePerfectNightTexture;
Texture2D useSameWifiTexture;
Texture2D andScanQrTexture;
bool unloadHomepage = false;

// Player
int lastPlayer = 0;
bool playerSpace[NUMBER_EIGHT] = {0};
Color themeColor[NUMBER_EIGHT] = {
    {35, 235, 141, 255},  // GREEN
    {241, 82, 91, 255},   // RED
    {64, 230, 230, 255},  // BLUE
    {127, 102, 242, 255}, // PURPLE
    {241, 209, 37, 255},  // YELLOW
    {255, 130, 47, 255},  // ORANGE
    {255, 149, 229, 255}, // PINK
    {101, 126, 255, 255}  // DISCORD COLOR x)
};

// Score
int colorScore[NUMBER_EIGHT] = {-1, -1, -1, -1, -1, -1, -1, -1};
int BoxesScoreFontSize[NUMBER_EIGHT] = {0};
Vector2 BoxesScoreSize[NUMBER_EIGHT] = {0};
int bestScore = 0;

// PLayer Alive
int numberActiveColor = 0;
int playerAlive = 0;
int playerAliveId = 0;
Color colorAlive;
bool otherColorAlive = false;

Player players[NUMBER_EIGHT] = {{}, {}, {}, {}, {}, {}, {}, {}};
int numberPlayer = 0;

Player *outsidePlayer;
Player *lastOutsidePlayer;
double startTimeOutside = 0.0;
double elapsedTimeOutside = 0.0;

// QrCode
bool qrCodeOk = false;
Texture2D qrCodeTexture;

// QrCode Particles
float qrCodeParticlesAnimationTimer1 = 0.5f;
float qrCodeParticlesAnimationTimer2 = 0.3f;
float qrCodeParticlesAnimationTimer3 = 0.1f;
Particle qrCodeParticles1[20];
Particle qrCodeParticles2[20];
Particle qrCodeParticles3[20];
int qrCodeParticlesIdColor1 = 1;
int qrCodeParticlesIdColor2 = 2;
int qrCodeParticlesIdColor3 = 3;

// Boxes
Box boxes[40] = {};
int boxesLength = sizeof(boxes) / sizeof(boxes[0]);

// Loots
Loot loots[4] = {};
static int lootsLength = sizeof(loots) / sizeof(loots[0]);

// Items
Texture2D BonusAmmunitionTexture;
Texture2D BonusLifeTexture;
Texture2D BonusLifeWhiteTexture;
Texture2D BonusSpeedTexture;
Texture2D LaserTexture;
Texture2D NothingTexture;

int lengthDataToSend = 0;
char dataToSend[2048];

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
    BonusLifeWhiteTexture = LoadTexture("resources/bonus_life_white.png");
    BonusSpeedTexture = LoadTexture("resources/bonus_speed.png");
    LaserTexture = LoadTexture("resources/laser.png");
    NothingTexture = LoadTexture("resources/nothing.png");

    // Init Camera
    camera.target = (Vector2){0.0f, 0.0f};
    camera.offset = (Vector2){0.0f, 0.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // Init
    InitBox();
    InitLoot();
    InitPlayer();

    // Init / Load TMX
    tmx_img_load_func = raylib_tex_loader;
    tmx_img_free_func = raylib_free_tex;
    InitMap();
}

void InitMap(void)
{
    if (idMap >= sizeof(listMap) / sizeof(listMap[0]))
    {
        idMap = 0;
    }
    map = tmx_load(listMap[idMap]);
    if (!map)
    {
        tmx_perror("Cannot load map");
    }
    arenaSizeX = map->tile_width * map->width;
    arenaSizeY = map->tile_height * map->height;
    tmx_init_object(map->ly_head, players, boxes, loots);
}

void SwitchMap(void)
{
    bestScore = 0;
    winnerMap = false;
    idMap++;
    memset(boxes, 0, sizeof boxes);
    memset(loots, 0, sizeof loots);
    map = NULL;
    InitMap();
    numberActiveColor = 0;
    for (int i = 0; i < lastPlayer; i++)
    {
        for (int c = 0; c < sizeof(themeColor) / sizeof(themeColor[0]); c++)
        {
            if (ColorToInt(themeColor[c]) == ColorToInt(players[i].color))
            {
                numberActiveColor++;
                colorScore[c] = 0;
                BoxesScoreFontSize[c] = CalculateFontSizeWithMaxSize(TextFormat("%d", colorScore[c]), BoxesScoreSize[c], 40);
            }
        }
    }
    camera.zoom = 1.0f;
    ResetGame();
}

void UpdateGameplay(void)
{
    centerPositionX = 0;
    centerPositionY = 0;
    // float centerDistance = 0.0f;

    playerAlive = 0;
    playerAliveId = 0;
    otherColorAlive = false;

    if (GetScreenWidth() < (arenaSizeX + 60.0f) * (camera.zoom / 1.0001f) || GetScreenHeight() < (arenaSizeY + 60.0f) * (camera.zoom / 1.0001f))
    {
        camera.zoom -= 0.001f;
    }
    if (GetScreenWidth() > (arenaSizeX + 120.0f) || GetScreenHeight() > (arenaSizeY + 120.0f))
    {
        camera.zoom += 0.001f;
    }

    if (GetTime() > lastSecond)
    {
        // Resize Canvas
        SetWindowSize(GetCanvasWidthCustom(), GetCanvasHeightCustom());
        lastSecond += 0.5;

        // Menu Action
        int menuAction = GetMenuAction();
        if (menuAction != 0)
        {
            switch (menuAction)
            {
            case 1: // Restart Game
                for (size_t i = 0; i < NUMBER_EIGHT; i++)
                {
                    if (colorScore[i] != -1)
                        colorScore[i] = 0;
                }
                ResetGame();
                break;
            case 2: // Change Map
                SwitchMap();
                break;
            case 3: // Dev
                activeDev = !activeDev;
                break;
            case 4: // Perf
                activePerf = !activePerf;
                break;
            case 5:
                activeMain = true;
                activeOnline = true;
                break;
            case 6:
                activeMain = false;
                activeOnline = true;
                qrCodeOk = false;
                break;
            }
            SetMenuAction(0);
        }

        if (GetEditSettings())
        {
            int *settings = GetAllSettings();
            // settings[0] = edit
            maxScore = settings[1];
            maxAmmunition = settings[2];
            defaultTypeItem = (float)settings[3];
            defaultMaxTimerItem = (float)settings[4];
            activeLoot = (bool)settings[5];
            free(settings);
            ResetGame();
            // @TODO displayed settings have been changed
        }
    }

    numberPlayer = GetNumberPlayer(); // +2
    if (lastPlayer < numberPlayer)
    {
        for (int i = 0; i < NUMBER_EIGHT; i++)
        {
            if (!playerSpace[i])
            {
                playerSpace[i] = true;
                players[i] = (Player){
                    i + 1,            // id: Identifier
                    GetIdGamepad(i),  // gamepadId: Gamepad identifier
                    1,                // life: Number of life
                    DELAY_INVINCIBLE, // invincible: Time of invincibility
                    maxAmmunition,    // ammunition: Ammunition
                    DELAY_AMMUNITION, // ammunitionLoad: Ammunition loading
                    {
                        {0.0f, 0.0f},
                        {40.0f, 40.0f},
                        {0.0f, 0.0f},
                        {false, false, false, false, false}}, // p: Physic
                    {0.0f, 0.0f},                             // spawn: Spawn position
                    {3.05f, 3.05f},                           // speed: Speed of the tank
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
                for (int i = 0; i < MAX_BULLET; i++)
                {
                    players[i].bullets[i] = (Bullet){
                        players[i].id,
                        (Physic){
                            {-9999.9f, -9999.9f},
                            {5.0f, 5.0f},
                            {0.0f, 0.0f},
                            {false, false, false, false, false}},
                        {0.0f, 0.0f},
                        0.0f,
                        false,
                        true,
                        true,
                        (Vector2){
                            0.0f,
                            0.0f,
                        },
                        0.0f,
                        players[i].color};
                }

                ResetGame();
                for (int c = 0; c < sizeof(themeColor) / sizeof(themeColor[0]); c++)
                {
                    if (ColorToInt(themeColor[c]) == ColorToInt(players[i].color))
                    {
                        if (colorScore[c] < 0)
                        {
                            numberActiveColor++;
                            colorScore[c] = 0;
                            BoxesScoreFontSize[c] = CalculateFontSizeWithMaxSize(TextFormat("%d", colorScore[c]), BoxesScoreSize[c], 40);
                        }
                        break;
                    }
                }
                i = NUMBER_EIGHT;
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
    if (activeDev)
    {
        camera.zoom += ((float)GetMouseWheelMove() * 0.01f);
    }

    // Active developer mode
    if (IsKeyPressed(KEY_O))
        activeDev = !activeDev;

    // Press R to reset
    if (IsKeyPressed(KEY_R))
    {
        for (int i = 0; i < NUMBER_EIGHT; i++)
        {
            colorScore[i] = -1;
        };
        bestScore = 0;
        ResetGame();
    }

    // Pause / @TODO
    if (IsKeyPressed(KEY_P))
        pauseGame = !pauseGame;
    if (pauseGame)
        return;

    if (numberPlayer == 0 && camera.target.x != 0.0f && camera.target.y != 0.0f)
    {
        // Particle effect on the homepage
        QrCodeParticleInit(&qrCodeParticlesAnimationTimer1, &qrCodeParticlesIdColor1, qrCodeParticles1);
        QrCodeParticleInit(&qrCodeParticlesAnimationTimer2, &qrCodeParticlesIdColor2, qrCodeParticles2);
        QrCodeParticleInit(&qrCodeParticlesAnimationTimer3, &qrCodeParticlesIdColor3, qrCodeParticles3);
        UpdateParticles(qrCodeParticles1, 20);
        UpdateParticles(qrCodeParticles2, 20);
        UpdateParticles(qrCodeParticles3, 20);
    }
    else if (numberPlayer != 0 && !unloadHomepage)
    {
        unloadHomepage = true;
        // Unload the homepage
        memset(qrCodeParticles1, 0, sizeof(qrCodeParticles1));
        memset(qrCodeParticles2, 0, sizeof(qrCodeParticles2));
        memset(qrCodeParticles3, 0, sizeof(qrCodeParticles3));
        UnloadTexture(titlePerfectNightTexture);
        UnloadTexture(useSameWifiTexture);
        UnloadTexture(andScanQrTexture);
    }

    if (activeOnline && !activeMain)
    {
        double datetime = 0.0;

        int player_index = 0;
        int player_part = 1;

        int type_index = 0;
        int type_value = 0;
        int type_loop = 1;

        int end_of_type = true;

        float bullet_pos_x = 0.0f;

        char *data = GetData();
        char *token;
        // TraceLog(LOG_INFO, "%s", data);

        // split the string with the delimiter ","
        while ((token = strchr(data, ',')) != NULL)
        {
            *token = '\0'; // replace the delimiter with null terminator
            if (datetime == 0.0)
            {
                datetime = atoll(data);
            }
            else if (player_part <= 3)
            {
                if (player_part == 1)
                {
                    memset(players[player_index].bullets, 0, MAX_BULLET);
                    // for (int i = 0; i < MAX_BULLET; i++)
                    // {
                    //     players[player_index].bullets[i].inactive = true;
                    // }
                    players[player_index].p.pos.x = atof(data);
                }
                else if (player_part == 2)
                {
                    players[player_index].p.pos.y = atof(data);
                }
                else
                {
                    players[player_index].invincible = 0;
                    players[player_index].charge = 2.0f;
                    players[player_index].radian = atof(data);
                    end_of_type = true;
                }
                player_part++;
            }
            else
            {
                if (end_of_type)
                {
                    end_of_type = false;
                    const int new_type_value = atoi(data);

                    if (new_type_value != type_value)
                    {
                        type_value = new_type_value;
                        type_index = 0;
                    }

                    if (type_value == 0)
                    {
                        player_part = 1;
                        player_index++;
                    }
                }
                else
                {
                    switch (type_value)
                    {
                    case 1:
                        InitItemWithTypeItem(player_index, atoi(data), defaultMaxTimerItem);
                        end_of_type = true;
                        break;

                    case 2:
                        if (type_loop == 1)
                        {
                            bullet_pos_x = atof(data);
                            type_loop++;
                        }
                        else
                        {
                            players[player_index].bullets[type_index] = (Bullet){
                                players[player_index].id,
                                (Physic){
                                    {bullet_pos_x, atof(data)},
                                    {5.0f, 5.0f},
                                    {0.0f, 0.0f},
                                    {false, false, false, false, false}},
                                {0.0f, 0.0f},
                                0.0f,
                                false,
                                false,
                                false,
                                (Vector2){
                                    players[player_index].p.pos.x + players[player_index].p.size.x / 2,
                                    players[player_index].p.pos.y + players[player_index].p.size.y / 2,
                                },
                                0.0f,
                                players[player_index].color};
                            type_index++;
                            type_loop = 1;
                            end_of_type = true;
                        }
                        break;

                    default:
                        TraceLog(LOG_INFO, "Error loop, type_value: %d, data: %s", type_value, data);
                        break;
                    }
                }
            }
            data = token + 1; // move the pointer to the next token
        }
        return;
    }

    if (activeOnline && activeMain)
    {
        double newTime = GetTime();
        lengthDataToSend = sizeof(newTime);
        memset(dataToSend, 0, 2048);
        strcat(dataToSend, TextFormat("%f,", newTime));
    }

    // Update Players / Bullets
    for (int i = 0; i < NUMBER_EIGHT; i++)
    {
        if (activeOnline && activeMain)
            PlayerValueToData(players[i], dataToSend);

        // If player not exist continue
        if (!players[i].id)
            continue;

        // Detect if one team win
        if (players[i].life >= 1)
        {
            playerAlive++;
            playerAliveId = i;
            if (ColorToInt(players[i].color) != ColorToInt(colorAlive))
            {
                otherColorAlive = true;
            }
            colorAlive = players[i].color;
        }

        UpdatePlayer(&players[i]);

        // Bullets
        for (int j = 0; j < MAX_BULLET; j++)
        {
            if (!players[i].bullets[j].playerId || players[i].bullets[j].inactive)
                continue;

            UpdateBullet(&players[i].bullets[j]);

            const Rectangle newBulletRec = {players[i].bullets[j].p.pos.x + 2.0f, players[i].bullets[j].p.pos.y + 2.0f, players[i].bullets[j].p.size.x * 2.0f - 5.0f, players[i].bullets[j].p.size.y * 2.0f - 5.0f};
            // Collision Bullet and Box
            for (int b = 0; b < boxesLength; b++)
            {
                if (!boxes[b].id || !boxes[b].collision)
                    continue;
                const Rectangle envBox = {boxes[b].p.pos.x, boxes[b].p.pos.y, boxes[b].p.size.x, boxes[b].p.size.y};
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
                const Rectangle envPlayer = {players[p].p.pos.x, players[p].p.pos.y, players[p].p.size.x, players[p].p.size.y};
                CollisionBulletPlayer(
                    CollisionPhysic(&players[i].bullets[j].p, newBulletRec, envPlayer),
                    &players[i].bullets[j],
                    &players[p],
                    envPlayer);
            }
            BulletBounce(&players[i].bullets[j]);
            if (activeOnline && activeMain)
                BulletValueToData(players[i].bullets[j], dataToSend);
        }

        // Collision Player and Player
        const Rectangle newPlayerRec = {players[i].p.pos.x, players[i].p.pos.y, players[i].p.size.x, players[i].p.size.y};
        for (int j = 0; j < NUMBER_EIGHT; j++)
        {
            if (players[j].life <= 0 || !players[i].id || !players[j].id)
                continue;
            if (players[i].id != players[j].id)
            {
                const Rectangle envPlayer = {players[j].p.pos.x, players[j].p.pos.y, players[j].p.size.x, players[j].p.size.y};
                CollisionPhysic(&players[i].p, newPlayerRec, envPlayer);
            }
        }

        // Collision Player and Box
        for (int j = 0; j < boxesLength; j++)
        {
            if (!boxes[j].id || !boxes[j].collision)
                continue;
            const Rectangle envBox = {boxes[j].p.pos.x, boxes[j].p.pos.y, boxes[j].p.size.x, boxes[j].p.size.y};
            CollisionPhysic(&players[i].p, newPlayerRec, envBox);
        }

        // Loots
        if (activeLoot)
        {
            for (int j = 0; j < lootsLength; j++)
            {
                UpdateLoot(&loots[j], &players[i]);
            }
        }

        if (players[i].shootParticle[0].timer != 0)
        {
            UpdateParticles(players[i].shootParticle, 20);
        }

        if (players[i].life >= 1)
        {
            centerPositionX += players[i].p.pos.x + players[i].p.size.x / 2.0f;
            centerPositionY += players[i].p.pos.y + players[i].p.size.y / 2.0f;
            // centerDistance += sqrtf(powf(camera.target.x - players[i].p.pos.x, 2.0f) + powf(camera.target.x - players[i].p.pos.y, 2.0f));
        }
        if (activeOnline && activeMain)
        {
            ItemValueToData(players[i], dataToSend);
            strcat(dataToSend, "0,");
        }
    }

    // Save data to js
    // TraceLog(LOG_INFO, "%s", dataToSend, lengthDataToSend);
    if (activeOnline && activeMain)
    {
        SendData(dataToSend);
    }

    // Camera Management
    camera.offset = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
    if (lastPlayer >= 2)
    {
        if (playerAlive >= 2)
        {
            centerPositionX = centerPositionX / playerAlive;
            centerPositionY = centerPositionY / playerAlive;
            camera.target = Vector2Lerp(
                camera.target,
                (Vector2){centerPositionX, centerPositionY},
                GetFrameTime() * (sqrtf(powf(camera.target.x - centerPositionX, 2.0f) + powf(camera.target.y - centerPositionY, 2.0f))) / 100.0f);
        }
        else if (playerAlive == 1)
        {
            centerPositionX = (players[playerAliveId].p.pos.x + players[playerAliveId].p.size.x) / 2.0f;
            centerPositionY = (players[playerAliveId].p.pos.y + players[playerAliveId].p.size.y) / 2.0f - 220.0f;
            camera.target = (Vector2){centerPositionX, centerPositionY};
        }
        else
        {
            centerPositionX = arenaSizeX / 2;
            centerPositionY = arenaSizeY / 2;
            camera.target = (Vector2){centerPositionX, centerPositionY};
        }
        // float newZoom = 1.0f - ((centerDistance - (arenaSizeX - 300)) / centerDistance);
        // if (fabs(newZoom) >= 1.1f || fabs(newZoom) <= 0.8f)
        // {
        //     newZoom = camera.zoom;
        // }
        // camera.zoom = Lerp(camera.zoom, newZoom - camera.zoom, GetFrameTime() * 10);
    }
    else
    {
        camera.target = (Vector2){arenaSizeX / 2.0f, arenaSizeY / 2.0f};
    }

    // Outside logic
    if ((outsidePlayer && !lastOutsidePlayer) || (outsidePlayer && lastOutsidePlayer && outsidePlayer->id != lastOutsidePlayer->id))
    {
        if (lastOutsidePlayer && ColorToInt(outsidePlayer->color) != ColorToInt(lastOutsidePlayer->color))
        {
            for (int i = 0; i < NUMBER_EIGHT; i++)
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
        for (int i = 0; i < NUMBER_EIGHT; i++)
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
                        InitParticles(players[i].p.pos, players[i].p.vel, 0.1f, players[i].color, 120.0f, players[i].shootParticle, 20);
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

    // WIN Logic
    if ((!otherColorAlive && lastPlayer > 1) || playerAlive == 0)
    {
        if (startTime == 0.0 && numberPlayer > 1)
        {
            for (int i = 0; i < NUMBER_EIGHT; i++)
            {
                if (playerSpace[i])
                {
                    for (int j = 0; j < MAX_BULLET; j++)
                    {
                        players[i].bullets[j].inactive = true;
                    }
                }
            }
            for (int c = 0; c < NUMBER_EIGHT; c++)
            {
                if (ColorToInt(themeColor[c]) == ColorToInt(players[playerAliveId].color))
                {
                    if (colorScore[c] > -1)
                    {
                        colorScore[c]++;
                        BoxesScoreFontSize[c] = CalculateFontSizeWithMaxSize(TextFormat("%d", colorScore[c]), BoxesScoreSize[c], 40);
                        if (colorScore[c] >= bestScore)
                            bestScore = colorScore[c];
                        if (colorScore[c] >= maxScore)
                            winnerMap = true;
                    }
                    break;
                }
            }
            startTime = GetTime();
        }
        elapsedTime = GetTime() - startTime;
        if (elapsedTime > 3.0)
        {
            if (winnerMap)
                SwitchMap();
            else
                ResetGame();
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
    for (int i = 0; i < NUMBER_EIGHT; i++)
    {
        if (playerSpace[i])
        {
            players[i].life = 1;
            players[i].id = i + 1;
            players[i].ammunition = maxAmmunition;
            players[i].invincible = DELAY_INVINCIBLE;
            players[i].charge = 0.0f;
            players[i].item.active = false;
            players[i].speed.x = 3.05f;
            players[i].speed.y = 3.05f;
            for (int j = 0; j < MAX_BULLET; j++)
            {
                players[i].bullets[j].inactive = true;
            }
            GamepadPlayerColor(players[i].gamepadId, players[i].color.r, players[i].color.g, players[i].color.b);
            GamepadPlayerLife(players[i].gamepadId, players[i].life);
            GamepadPlayerAmmunition(players[i].gamepadId, players[i].life);
            if (defaultTypeItem > -1)
            {
                InitItemWithTypeItem(i + 1, defaultTypeItem, defaultMaxTimerItem);
            }
        }
    }
    startTime = 0.0;
}

void DrawGameplay(void)
{
    // DRAW GAME
    BeginMode2D(camera); // DRAW 1

    if (outsidePlayer)
    {
        ClearBackground(DarkenColor(DarkenColor(outsidePlayer->color, 0.2f), 1.0f - elapsedTimeOutside / 2.0f));
    }
    else
    {
        if (elapsedTimeOutside > 0)
            elapsedTimeOutside = elapsedTimeOutside - 0.01;
        ClearBackground(DarkenColor(int_to_color(map->backgroundcolor), 1.0f - elapsedTimeOutside / 2.0f));
    }
    DrawCircleGradient(arenaSizeX / 2.0f, arenaSizeY / 2.0f, arenaSizeX + 300.0f, Fade(BLACK, 0.6f), Fade(BLACK, 0.0f));
    DrawRectangle(-2, -2, arenaSizeX + 4, arenaSizeY + 4, Fade(GRAY, 0.5f));
    if (activePerf)
    {
        DrawRectangle(0, 0, arenaSizeX, arenaSizeY, BLACKGROUND);
    }
    else
    {
        render_map(map);
    }
    DrawGameArena();

    // Draw Boxes
    for (int i = 0; i < boxesLength; i++)
    {
        DrawBox(boxes[i]);
    }

    // Draw Players / Bullets
    for (int i = 0; i < NUMBER_EIGHT; i++)
    {
        if (!players[i].id)
            continue;
        DrawSpawnPlayer(players[i]);
        for (int j = 0; j < MAX_BULLET; j++)
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

    if (activeLoot)
    {
        for (int i = 0; i < lootsLength; i++)
        {
            DrawLoot(loots[i]);
        }
    }
    EndMode2D();

    // DRAW STAT LOG INFO MENU SCREEN
    if (numberPlayer == 0 || startTime != 0.0 || pauseGame || activeDev)
    {
        BeginDrawing(); // DRAW 2

        if (numberPlayer == 0 || startTime != 0.0 || pauseGame) // Background for title / win
        {
            DrawRectangle(0.0f, 0.0f, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.4f));
        }

        // @DEV
        if (activeDev)
        {
            for (int i = 0; i < NUMBER_EIGHT; i++)
            {
                if (!players[i].id)
                    continue;
                DrawStatsPlayer(players[i]);
            }

            // DISPLAY FPS
            DrawFPS(10, 10);

            // DISPLAY INFO
            DrawText(TextFormat("ZOOM: %f", camera.zoom), 10, 30, 10, WHITE);
            DrawText(TextFormat("TARGET: %f/%f", camera.target.x, camera.target.y), 10, 40, 10, WHITE);
            DrawText(TextFormat("DELTA: %f", GetFrameTime()), 10, 50, 10, WHITE);
        }

        EndDrawing();
    }

    if (numberPlayer == 0 || startTime != 0.0 || pauseGame)
    {
        BeginMode2D(camera); // DRAW 3

        // Draw Wins
        if (startTime != 0.0)
        {
            Color background = Fade(BLACKGROUND, 0.9);
            if (maxScore <= bestScore)
                background = Fade(GOLD, 0.1);
            DrawRectangle(camera.target.x - arenaSizeX, camera.target.y - 10, arenaSizeX * 2, 110, background);
            if (maxScore <= bestScore + 1)
            {
                DrawRectangleLinesEx((Rectangle){camera.target.x - arenaSizeX, camera.target.y - 10, arenaSizeX * 2, 110}, 2.5f, Fade(GOLD, 0.3f));
            }
            for (int i = 0; i < numberActiveColor; i++)
            {
                if (colorScore[i] > -1)
                {
                    const char *textScore = TextFormat("%d", colorScore[i]);
                    int textScoreSize = MeasureText(textScore, 100);
                    int centerIfOdd = 160;
                    if (numberActiveColor % 2)
                    {
                        centerIfOdd = 320;
                    }
                    DrawText(
                        textScore,
                        (int)camera.target.x + (320 - 10 * (numberActiveColor - 2)) - (320 - 20 * (numberActiveColor - 2)) * (i - numberActiveColor / 2) * -1 - centerIfOdd - textScoreSize / 2,
                        (int)camera.target.y, 100, themeColor[i]);
                }
            }
            DrawCircle(camera.target.x, camera.target.y - 300.0f, 50.0f, BLACK);
            DrawCircle(camera.target.x, camera.target.y - 300.0f, 48.0f, WHITE);
            DrawCircle(camera.target.x, camera.target.y - 300.0f, 40.0f, colorAlive);
            DrawText("WINS THIS GAME", (int)(camera.target.x - 50.0f * 7.0f), (int)(camera.target.y - 200.0f), 80, colorAlive);
        }
        // Draw Pause
        else if (pauseGame)
        {
            DrawText("PAUSE", (int)(camera.target.x - MeasureText("PAUSE", 100) / 2), (int)(camera.target.y - 200), 100, WHITE);
        }
        // Draw Title
        else if (numberPlayer == 0)
        {
            DrawParticles(qrCodeParticles1, 20);
            DrawParticles(qrCodeParticles2, 20);
            DrawParticles(qrCodeParticles3, 20);
            DrawRectangleRounded((Rectangle){camera.target.x - qrCodeTexture.width / 2 - 10, camera.target.y - qrCodeTexture.height / 2 + 310, qrCodeTexture.width + 20, qrCodeTexture.height + 20}, 0.14f, 1.0f, BLACKGROUND);
            DrawRectangleRounded((Rectangle){camera.target.x - qrCodeTexture.width / 2 - 5, camera.target.y - qrCodeTexture.height / 2 + 315, qrCodeTexture.width + 10, qrCodeTexture.height + 10}, 0.1f, 1.0f, WHITE);
            DrawTexture(qrCodeTexture, camera.target.x - qrCodeTexture.width / 2, camera.target.y - qrCodeTexture.height / 2 + 320, WHITE);
            DrawTexture(titlePerfectNightTexture, camera.target.x - 155 * 3.47, camera.target.y - 105 * 3.47, WHITE);
            DrawTexture(useSameWifiTexture, camera.target.x - 205 * 3.47, camera.target.y + 210, WHITE);
            DrawTexture(andScanQrTexture, camera.target.x + 80 * 3.47, camera.target.y + 230, WHITE);
            DrawTextPro(GetFontDefault(), "BETA !", (Vector2){camera.target.x + 490, camera.target.y - 234}, (Vector2){0.0f, 0.0f}, 26.0f, 30.0f, 2.6f, WHITE);

            DrawRectangle(0, 18, 300, 144, Fade(BLACKGROUND, 0.6f));
            DrawText("- Don't use a VPN", 30, 40, 24, WHITE);
            DrawText("- Chromium browser", 30, 80, 24, WHITE);
            DrawText("  is recommended :S", 30, 120, 24, WHITE);

            DrawRectangle(camera.target.x * 2 - 200, 20, 200, 220, Fade(BLACKGROUND, 0.6f));
            DrawRectangle(camera.target.x * 2 - 180, 60, 160, 160, BLACK);
            DrawRectangle(camera.target.x * 2 - 170, 70, 140, 140, RAYWHITE);
            DrawText("raylib", camera.target.x * 2 - 128, 166, 31, BLACK);
            DrawText("Powered with", camera.target.x * 2 - 176, 30, 24, WHITE);

            DrawText("Free Open-Source Game", camera.target.x * 2 - 203, 250, 18, WHITE);
        }
        EndMode2D();
    }
}

void DrawGameArena(void)
{
    if (activeDev || activePerf)
    {
        for (int x = 0; x <= (int)(arenaSizeX * 0.01f); x++)
        {
            if (x < (int)(arenaSizeX * 0.01f))
            {
                DrawText(TextFormat("%d", x + 1), x * 100 + 6, 4, 20, GRAYDARK);
            }
            Rectangle posViewX = {(float)(x * 100), 0.0f, 2.0f, arenaSizeY};
            DrawRectangleRec(posViewX, GRAYDARK);
        }

        for (int y = 0; y <= (int)(arenaSizeY * 0.01f); y++)
        {
            if (y < (int)(arenaSizeY * 0.01))
            {
                DrawText(TextFormat("%d", y + 1), 6, y * 100 + 4, 20, GRAYDARK);
            }
            Rectangle posViewY = {0.0f, (int)(y * 100), arenaSizeX, 2.0f};
            DrawRectangleRec(posViewY, GRAYDARK);
        }
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
                int sizeRec = 254;
                sizeRec -= border * 2;
                int t = sizeRec / qrCodeSize;
                int c = 0;
                Image qrCodeImage = GenImageColor(t * qrCodeSize + border * 2, t * qrCodeSize + border * 2, (Color){0, 0, 0, 0});
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
                                ImageDrawRectangle(&qrCodeImage, x * t + border, y * t + border, t, t, BLACKGROUND);
                            }
                            else
                            {
                                // ImageDrawRectangle(&qrCodeImage, x*t+border, y*t+border, t, t, themeColor[c]);
                                ImageDrawRectangle(&qrCodeImage, x * t + border, y * t + border, t, t, BLACKGROUND);
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

void QrCodeParticleInit(float *qrCodeParticlesAnimationTimer, int *qrCodeParticlesIdColor, Particle *qrCodeParticles)
{
    *qrCodeParticlesAnimationTimer += 0.004f;
    if (*qrCodeParticlesAnimationTimer >= 0.4f)
    {
        *qrCodeParticlesIdColor += 1;
        if (*qrCodeParticlesIdColor >= 8)
            *qrCodeParticlesIdColor = 0;

        *qrCodeParticlesAnimationTimer = 0.0f;
        InitParticles(
            (Vector2){camera.target.x,
                      camera.target.y + 316},
            (Vector2){0, 0},
            0.3f,
            themeColor[*qrCodeParticlesIdColor],
            90.0f,
            qrCodeParticles,
            20);
    }
}
