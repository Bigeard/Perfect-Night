#include "../../../lib/raylib/src/raylib.h"
#include "../../../lib/raylib/src/raymath.h"
#include <emscripten/emscripten.h>
#include <stdio.h>
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
#include "../score/score.h"
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
    const values = [
        gameSettings.edit,
        gameSettings.maxScore,
        gameSettings.maxAmmunition,
        gameSettings.defaultTypeItem,
        gameSettings.defaultMaxTimerItem,
        gameSettings.activeLoot,
    ];
    const arrayPointer = Module._malloc(values.length * 4);
    values.forEach(function(v, i) {
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
        const screenShare = listScreenShare[i];
        if (!screenShare || !screenShare.conn || !screenShare.conn.open)
        {
            i++;
            continue;
        }
        if (!screenShare.init)
        {
            screenShare.init = true;
            listGamepad.forEach(function (g) {
                screenShare.conn.send(JSON.stringify({
                    t : Date.now(), // Time
                    g : {
                        conn : null,
                        peer : g.id,
                        id : g.index,
                    }, // Create Virtual Gamepad
                }));
            });
        }
        const channel = screenShare.conn.dataChannel;
        if (!channel || channel.bufferedAmount < 65536)
        {
            screenShare.conn.send(dataToSend);
        }
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
static const float CAMERA_WIDER_VIEW_FACTOR = 1.1f;

// Keep simulation movement smooth while aligning the rendered world to the
// physical pixel grid. This prevents fractional camera movement from making
// primitive edges shimmer between adjacent pixels.
static Camera2D GetRenderCamera(void)
{
    Camera2D renderCamera = camera;

    renderCamera.offset.x = roundf(renderCamera.offset.x);
    renderCamera.offset.y = roundf(renderCamera.offset.y);
    if (renderCamera.zoom > 0.0f)
    {
        renderCamera.target.x = roundf(renderCamera.target.x * renderCamera.zoom) / renderCamera.zoom;
        renderCamera.target.y = roundf(renderCamera.target.y * renderCamera.zoom) / renderCamera.zoom;
    }

    return renderCamera;
}

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
static RenderTexture2D mapTexture = {0};
static bool mapTextureLoaded = false;
static tmx_layer *grassLayer = NULL;
static Shader backgroundShader = {0};
static int backgroundShaderTimeLoc = -1;
static int backgroundShaderResolutionLoc = -1;
static bool backgroundShaderReady = false;

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
static bool roundResultActive = false;
static bool roundIsDraw = false;
static int roundWinningTeam = SCORE_NO_TEAM;

Player players[NUMBER_EIGHT] = {{}, {}, {}, {}, {}, {}, {}, {}};
int numberPlayer = 0;

int GetTeamColorIndex(Color color)
{
    for (int i = 0; i < NUMBER_EIGHT; i++)
    {
        if (ColorToInt(themeColor[i]) == ColorToInt(color)) return i;
    }
    return SCORE_NO_TEAM;
}

static void UpdateScoreFont(int teamIndex)
{
    if (teamIndex < 0 || teamIndex >= NUMBER_EIGHT) return;
    BoxesScoreFontSize[teamIndex] = CalculateFontSizeWithMaxSize(
        TextFormat("%d", colorScore[teamIndex]), BoxesScoreSize[teamIndex], 40);
}

static void ActivateTeamColor(Color color)
{
    const int teamIndex = GetTeamColorIndex(color);
    if (ScoreActivateTeam(colorScore, teamIndex)) UpdateScoreFont(teamIndex);
    numberActiveColor = ScoreCountActive(colorScore);
}

static void RebuildActiveTeamScores(bool resetScores)
{
    if (resetScores) ScoreResetAll(colorScore);
    for (int i = 0; i < NUMBER_EIGHT; i++)
    {
        if (playerSpace[i] && players[i].id) ActivateTeamColor(players[i].color);
    }
    numberActiveColor = ScoreCountActive(colorScore);
    bestScore = ScoreBest(colorScore);
}

static void StopAllBullets(void)
{
    for (int i = 0; i < NUMBER_EIGHT; i++)
    {
        if (!playerSpace[i]) continue;
        for (int j = 0; j < MAX_BULLET; j++) players[i].bullets[j].inactive = true;
    }
}

static void BeginRoundResult(int winningTeam, bool awardScore)
{
    if (roundResultActive) return;
    StopAllBullets();
    roundWinningTeam = winningTeam;
    roundIsDraw = winningTeam == SCORE_NO_TEAM;
    if (!roundIsDraw)
    {
        if (awardScore)
        {
            ScoreAwardTeam(colorScore, winningTeam, maxScore, &bestScore);
            winnerMap = bestScore >= maxScore;
            UpdateScoreFont(winningTeam);
        }
    }
    startTime = GetTime();
    elapsedTime = 0.0;
    roundResultActive = true;
}

static void UpdateCameraTarget(int aliveCount, int aliveId, float centerX, float centerY)
{
    camera.offset = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};

    if (lastPlayer < 2)
    {
        camera.target = (Vector2){arenaSizeX / 2.0f, arenaSizeY / 2.0f};
    }
    else if (aliveCount >= 2)
    {
        const Vector2 aliveCenter = {centerX / aliveCount, centerY / aliveCount};
        camera.target = Vector2Lerp(
            camera.target,
            aliveCenter,
            GetFrameTime() * Vector2Distance(camera.target, aliveCenter) / 100.0f);
    }
    else if (aliveCount == 1)
    {
        camera.target = (Vector2){
            players[aliveId].p.pos.x + players[aliveId].p.size.x / 2.0f,
            players[aliveId].p.pos.y + players[aliveId].p.size.y / 2.0f - 220.0f};
    }
    // When everyone is dead, keep the last meaningful target. Dead players can
    // be moved to their spawn points during round cleanup and must not pull the
    // camera away from the place where the round ended.
}

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

#define GAME_STATE_BUFFER_SIZE 8192

int lengthDataToSend = 0;
char dataToSend[GAME_STATE_BUFFER_SIZE];

static void LoadStaticMapTexture(void)
{
    if (mapTextureLoaded)
    {
        UnloadRenderTexture(mapTexture);
        mapTextureLoaded = false;
    }

    if (!map || arenaSizeX <= 0.0f || arenaSizeY <= 0.0f)
    {
        return;
    }

    mapTexture = LoadRenderTexture((int)arenaSizeX, (int)arenaSizeY);
    BeginTextureMode(mapTexture);
    ClearBackground(BLANK);
    render_map(map);
    EndTextureMode();
    mapTextureLoaded = true;
}

static bool DynamicGrassBlocked(Vector2 point)
{
    for (int i = 0; i < boxesLength; i++)
    {
        if (!boxes[i].id || !boxes[i].collision)
            continue;
        const Rectangle box = {boxes[i].p.pos.x, boxes[i].p.pos.y, boxes[i].p.size.x, boxes[i].p.size.y};
        if (CheckCollisionPointRec(point, box))
            return true;
    }
    return false;
}

static void DrawDynamicGrassTuft(Vector2 base, float bladeLength, unsigned int hash, float windTime)
{
    const float influenceRadius = 78.0f;
    Vector2 bend = {0.0f, 0.0f};
    float strongestInfluence = 0.0f;

    for (int i = 0; i < NUMBER_EIGHT; i++)
    {
        const Player *player = &players[i];
        if (!player->id || player->life <= 0)
            continue;

        const Vector2 playerCenter = {
            player->p.pos.x + player->p.size.x/2.0f,
            player->p.pos.y + player->p.size.y/2.0f};
        const float distance = Vector2Distance(base, playerCenter);
        if (distance >= influenceRadius)
            continue;

        const float influence = 1.0f - distance/influenceRadius;
        if (influence <= strongestInfluence)
            continue;
        strongestInfluence = influence;

        Vector2 away = Vector2Subtract(base, playerCenter);
        if (Vector2LengthSqr(away) > 0.0f)
            away = Vector2Normalize(away);
        bend = Vector2Add(
            Vector2Scale(away, 7.0f*influence),
            Vector2Scale(player->p.vel, 2.2f*influence));
    }
    if (Vector2Length(bend) > 12.0f)
        bend = Vector2Scale(Vector2Normalize(bend), 12.0f);

    const float windPhase = base.x*0.018f + base.y*0.009f + (float)(hash%19u)*0.13f;
    const float breeze = sinf(windPhase + windTime*1.7f)*2.6f;
    const float gust = sinf(windPhase*0.37f + windTime*0.55f)*1.8f;
    const float heightFactor = bladeLength/22.0f;
    bend.x += (breeze + gust)*heightFactor;
    bend.y += fabsf(breeze + gust)*0.08f;

    const float naturalLean = ((float)((hash/31u)%9u) - 4.0f)*0.7f;
    const Vector2 tip = {base.x + naturalLean + bend.x, base.y - bladeLength + bend.y};
    const unsigned char shade = (unsigned char)((hash/127u)%18u);
    const Color grassDark = {(unsigned char)(38 + shade), (unsigned char)(63 + shade), (unsigned char)(81 + shade), 230};
    const Color grassLight = {(unsigned char)(64 + shade), (unsigned char)(96 + shade), (unsigned char)(116 + shade), 240};
    DrawLineEx(base, tip, 2.2f, grassDark);
    DrawLineEx((Vector2){base.x - 3.0f, base.y},
               (Vector2){tip.x - 5.0f + bend.x*0.25f, tip.y + bladeLength*0.35f}, 1.8f, grassLight);
    DrawLineEx((Vector2){base.x + 3.0f, base.y},
               (Vector2){tip.x + 6.0f + bend.x*0.25f, tip.y + bladeLength*0.42f}, 1.8f, grassLight);
}

static bool IsDynamicGrassAtlasTile(unsigned int gid)
{
    if (gid == 0 || !map->tiles[gid])
        return false;

    const tmx_tile *tile = map->tiles[gid];
    const unsigned int column = tile->ul_x/tile->tileset->tile_width;
    const unsigned int row = tile->ul_y/tile->tileset->tile_height;
    return column >= 4u && column <= 7u && row <= 3u;
}

static void DrawDynamicGrass(Camera2D renderCamera)
{
    if (activePerf || !map || !grassLayer || grassLayer->type != L_LAYER)
        return;

    const int tileWidth = (int)map->tile_width;
    const int tileHeight = (int)map->tile_height;
    const float windTime = (float)GetTime();
    const Vector2 viewTopLeft = GetScreenToWorld2D((Vector2){0.0f, 0.0f}, renderCamera);
    const Vector2 viewBottomRight = GetScreenToWorld2D(
        (Vector2){(float)GetScreenWidth(), (float)GetScreenHeight()}, renderCamera);
    int firstTileX = (int)floorf(viewTopLeft.x/(float)tileWidth) - 1;
    int firstTileY = (int)floorf(viewTopLeft.y/(float)tileHeight) - 1;
    int lastTileX = (int)ceilf(viewBottomRight.x/(float)tileWidth) + 1;
    int lastTileY = (int)ceilf(viewBottomRight.y/(float)tileHeight) + 1;

    if (firstTileX < 0) firstTileX = 0;
    if (firstTileY < 0) firstTileY = 0;
    if (lastTileX >= (int)map->width) lastTileX = (int)map->width - 1;
    if (lastTileY >= (int)map->height) lastTileY = (int)map->height - 1;

    for (int tileY = firstTileY; tileY <= lastTileY; tileY++)
    {
        for (int tileX = firstTileX; tileX <= lastTileX; tileX++)
        {
            const unsigned int gid = grassLayer->content.gids[tileY*(int)map->width + tileX] & TMX_FLIP_BITS_REMOVAL;
            if (!IsDynamicGrassAtlasTile(gid))
                continue;

            const unsigned int tileHash = (unsigned int)tileX*73856093u ^ (unsigned int)tileY*19349663u;
            const int tuftCount = 1 + (int)(tileHash%3u);
            for (int tuft = 0; tuft < tuftCount; tuft++)
            {
                const unsigned int hash = tileHash ^ ((unsigned int)tuft + 1u)*2654435761u;
                const Vector2 base = {
                    tileX*(float)tileWidth + 4.0f + (float)(hash%25u),
                    tileY*(float)tileHeight + 21.0f + (float)((hash/29u)%9u)};
                if (DynamicGrassBlocked(base))
                    continue;
                const float bladeLength = 18.0f + (float)((hash/113u)%10u);
                DrawDynamicGrassTuft(base, bladeLength, hash, windTime);
            }
        }
    }
}

void InitGameplay()
{
    backgroundShader = LoadShader(NULL, "resources/shaders/cineshader_background.fs");
    backgroundShaderTimeLoc = GetShaderLocation(backgroundShader, "iTime");
    backgroundShaderResolutionLoc = GetShaderLocation(backgroundShader, "iResolution");
    backgroundShaderReady = IsShaderValid(backgroundShader) &&
                            backgroundShaderTimeLoc >= 0 &&
                            backgroundShaderResolutionLoc >= 0;
    if (!backgroundShaderReady)
    {
        TraceLog(LOG_ERROR, "Required CineShader background failed to load");
    }

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

void InitMap()
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
    grassLayer = tmx_find_layer_by_name(map, "Background");
    tmx_init_object(map->ly_head, players, boxes, loots);
    LoadStaticMapTexture();
}

void SwitchMap()
{
    bestScore = 0;
    winnerMap = false;
    memset(boxes, 0, sizeof boxes);
    memset(loots, 0, sizeof loots);
    map = NULL;
    InitMap();
    RebuildActiveTeamScores(true);
    camera.zoom = 1.0f;
    ResetGame();
}

void UpdateGameplay()
{
    bool aliveTeams[NUMBER_EIGHT] = {false};
    centerPositionX = 0;
    centerPositionY = 0;
    // float centerDistance = 0.0f;

    playerAlive = 0;
    playerAliveId = 0;

    const float cameraArenaWidth = (arenaSizeX + 60.0f) * CAMERA_WIDER_VIEW_FACTOR;
    const float cameraArenaHeight = (arenaSizeY + 60.0f) * CAMERA_WIDER_VIEW_FACTOR;
    if (GetScreenWidth() < cameraArenaWidth * camera.zoom || GetScreenHeight() < cameraArenaHeight * camera.zoom)
    {
        camera.zoom -= 0.001f;
    }
    else if (GetScreenWidth() > (cameraArenaWidth + 60.0f) * camera.zoom && GetScreenHeight() > (cameraArenaHeight + 60.0f) * camera.zoom)
    {
        camera.zoom += 0.001f;
    }

    if (GetTime() > lastSecond)
    {
        // Resize Canvas
        const int canvasWidth = GetCanvasWidthCustom();
        const int canvasHeight = GetCanvasHeightCustom();
        if (canvasWidth != GetScreenWidth() || canvasHeight != GetScreenHeight())
        {
            SetWindowSize(canvasWidth, canvasHeight);
        }
        lastSecond += 0.5;

        // Menu Action
        int menuAction = GetMenuAction();
        if (menuAction != 0)
        {
            switch (menuAction)
            {
            case 1: // Restart Game
                ScoreClearActive(colorScore);
                bestScore = 0;
                winnerMap = false;
                ResetGame();
                break;
            case 2: // Change Map
                idMap++;
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
            bestScore = ScoreBest(colorScore);
            winnerMap = bestScore >= maxScore;
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
                for (int b = 0; b < MAX_BULLET; b++)
                {
                    players[i].bullets[b] = (Bullet){
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
                        0,
                        true,
                        (Vector2){
                            0.0f,
                            0.0f,
                        },
                        0.0f,
                        players[i].color};
                }

                ResetGame();
                ActivateTeamColor(players[i].color);
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
        RebuildActiveTeamScores(true);
        bestScore = 0;
        winnerMap = false;
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
        int idMapData = -1;
        int loot_index = 0;

        int player_index = 0;
        int player_part = 1;

        int type_index = 0;
        int type_value = 0;
        int type_loop = 1;

        int end_of_type = true;

        float bullet_pos_x = 0.0f;

        char *data = GetData();
        char *dataStart = data;
        char *token;
        // TraceLog(LOG_INFO, "%s", data);
        if (!data)
            return;

        // split the string with the delimiter ","
        while ((token = strchr(data, ',')) != NULL)
        {
            *token = '\0'; // replace the delimiter with null terminator
            if (datetime == 0.0)
            {
                datetime = atoll(data);
            }
            else if (idMapData == -1)
            { // Change map if id change
                idMapData = atoi(data);
                if (idMapData != idMap)
                {
                    idMap = idMapData;
                    SwitchMap();
                }
            }
            else if (loot_index <= 3)
            { // Display or not the the loot
                loots[loot_index].active = atoi(data);
                loot_index++;
            }
            else if (player_part <= 9) // Loop 9 times for the player
            {
                // Start Init Player !
                if (player_part == 1) // Postion player X
                {
                    players[player_index].p.pos.x = atof(data);
                }
                else if (player_part == 2) // Postion player Y
                {
                    players[player_index].p.pos.y = atof(data);
                }
                else if (player_part == 3) // Radian player cannon
                {
                    players[player_index].radian = atof(data);
                }
                else if (player_part == 4) // Charge of the bullet
                {
                    players[player_index].charge = atof(data);
                }
                else if (player_part == 5) // Type of the item (if type == 0 item = NULL)
                {
                    const int type_item = atoi(data);
                    if (type_item != 0)
                    {
                        InitItemWithTypeItem(player_index + 1, atoi(data), defaultMaxTimerItem);
                    }
                    else
                    {
                        players[player_index].item.active = false;
                    }
                }
                else if (player_part == 6) // Timer item (detect change and for the animation)
                {
                    players[player_index].item.timer = atof(data);
                }
                else if (player_part == 7) // Life player
                {
                    const int new_life = atoi(data);
                    if (new_life != players[player_index].life)
                    {
                        players[player_index].life = new_life;
                        if (players[player_index].life <= 0)
                        {
                            // Add case of velocity of the bullet (more data to send probably) @TODO
                            InitParticles(players[player_index].p.pos, players[player_index].p.vel, 0.1f, players[player_index].color, 120.0f, players[player_index].shootParticle, 20);
                        }
                    }
                }
                else if (player_part == 8) // Ammunition player
                {
                    players[player_index].ammunition = atoi(data);
                }
                else if (player_part == 9) // Score player
                {                          // End Init Player !
                    const int player_score = atoi(data);
                    const int teamIndex = GetTeamColorIndex(players[player_index].color);
                    if (ScoreActivateTeam(colorScore, teamIndex)) numberActiveColor = ScoreCountActive(colorScore);
                    if (teamIndex >= 0 && colorScore[teamIndex] != player_score)
                    {
                        colorScore[teamIndex] = player_score;
                        UpdateScoreFont(teamIndex);
                        bestScore = ScoreBest(colorScore);
                        winnerMap = bestScore >= maxScore;
                    }

                    // ...
                    // Other player stuff
                    if (players[player_index].invincible > 1)
                        players[player_index].invincible = players[player_index].invincible - GetFrameTime();
                    else
                        players[player_index].invincible = 0;

                    // memset(players[player_index].bullets, 0, MAX_BULLET);
                    for (size_t i = 0; i < MAX_BULLET; i++)
                    {
                        if (players[player_index].bullets[i].playerId)
                            players[player_index].bullets[i].inactive = true;
                    }

                    if (players[player_index].life >= 1)
                    {
                        playerAlive++;
                        playerAliveId = player_index;
                        if (teamIndex >= 0) aliveTeams[teamIndex] = true;
                    }
                    if (players[player_index].life >= 1)
                    {
                        centerPositionX += players[player_index].p.pos.x + players[player_index].p.size.x / 2.0f;
                        centerPositionY += players[player_index].p.pos.y + players[player_index].p.size.y / 2.0f;
                    }

                    // Out of area
                    if ((players[player_index].p.pos.x >= arenaSizeX ||
                         players[player_index].p.pos.x + players[player_index].p.size.x <= 0.0f) ||
                        (players[player_index].p.pos.y >= arenaSizeY ||
                         players[player_index].p.pos.y + players[player_index].p.size.y <= 0.0f))
                    {

                        if (!lastOutsidePlayer || lastOutsidePlayer->id != players[player_index].id)
                        {
                            outsidePlayer = &players[player_index];
                        }
                    }
                    else
                    {
                        if (outsidePlayer && outsidePlayer->id == players[player_index].id)
                        {
                            outsidePlayer = NULL;
                        }
                    }

                    if (players[player_index].life <= 0 && outsidePlayer && outsidePlayer->id == players[player_index].id)
                    {
                        outsidePlayer = NULL;
                    }

                    if (players[player_index].shootParticle[0].timer != 0)
                    {
                        UpdateParticles(players[player_index].shootParticle, 20);
                    }
                    UpdatePlayerMovementParticles(&players[player_index], !activePerf);

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
                                0,
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
        UpdateCameraTarget(playerAlive, playerAliveId, centerPositionX, centerPositionY);
        const int remoteAliveTeam = ScoreResolveAliveTeam(aliveTeams);
        if (!roundResultActive && ScoreRoundHasResult(numberActiveColor, remoteAliveTeam))
        {
            BeginRoundResult(remoteAliveTeam, false);
        }

        if ((outsidePlayer && !lastOutsidePlayer) || (outsidePlayer && lastOutsidePlayer && outsidePlayer->id != lastOutsidePlayer->id))
        {
            lastOutsidePlayer = outsidePlayer;
            startTimeOutside = GetTime();
        }
        if (startTimeOutside != 0.0)
        {
            elapsedTimeOutside = GetTime() - startTimeOutside;
        }
        if (elapsedTimeOutside > 2.0)
        {
            outsidePlayer = NULL;
            lastOutsidePlayer = NULL;
            startTimeOutside = 0.0;
        }
        if (!outsidePlayer)
        {
            lastOutsidePlayer = NULL;
            startTimeOutside = 0.0;
        }

        if (roundResultActive)
        {
            elapsedTime = GetTime() - startTime;
            if (elapsedTime > 3.0)
            {
                ResetGame();
            }
        }
        // TraceLog(LOG_INFO, TextFormat("Item player %d => %d", player_index, players[0].item.type));
        free(dataStart);
        return;
    }

    if (activeOnline && activeMain)
    {
        double newTime = GetTime();
        lengthDataToSend = sizeof(newTime);
        memset(dataToSend, 0, sizeof dataToSend);
        snprintf(dataToSend, sizeof(dataToSend), "%f,%d,%d,%d,%d,%d,", newTime, idMap, loots[0].active, loots[1].active, loots[2].active, loots[3].active);
    }

    // Update Players / Bullets
    for (int i = 0; i < NUMBER_EIGHT; i++)
    {
        // If player not exist continue
        if (!players[i].id)
            continue;

        UpdatePlayer(&players[i]);
        UpdatePlayerMovementParticles(&players[i], !activePerf);
        if (activeOnline && activeMain)
        {
            PlayerValueToData(players[i], dataToSend, sizeof(dataToSend));
        }

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
            if (players[i].bullets[j].p.collision[0] && players[i].bullets[j].explosionRadius > 0.0f)
            {
                ExplodeBullet(&players[i].bullets[j]);
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
                BulletValueToData(players[i].bullets[j], dataToSend, sizeof(dataToSend));
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
            // Detect if one team win
            playerAlive++;
            playerAliveId = i;
            const int teamIndex = GetTeamColorIndex(players[i].color);
            if (teamIndex >= 0) aliveTeams[teamIndex] = true;

            // Position the camera to the center
            centerPositionX += players[i].p.pos.x + players[i].p.size.x / 2.0f;
            centerPositionY += players[i].p.pos.y + players[i].p.size.y / 2.0f;
            // centerDistance += sqrtf(powf(camera.target.x - players[i].p.pos.x, 2.0f) + powf(camera.target.x - players[i].p.pos.y, 2.0f));
        }
        if (activeOnline && activeMain)
        {
            const size_t len = strlen(dataToSend);
            if (len < sizeof(dataToSend))
            {
                snprintf(dataToSend + len, sizeof(dataToSend) - len, "0,");
            }
        }
    }

    // Save data to js
    // TraceLog(LOG_INFO, "%s", dataToSend, lengthDataToSend);
    if (activeOnline && activeMain)
    {
        SendData(dataToSend);
    }

    UpdateCameraTarget(playerAlive, playerAliveId, centerPositionX, centerPositionY);

    if (lastPlayer >= 2)
    {
        // float newZoom = 1.0f - ((centerDistance - (arenaSizeX - 300)) / centerDistance);
        // if (fabs(newZoom) >= 1.1f || fabs(newZoom) <= 0.8f)
        // {
        //     newZoom = camera.zoom;
        // }
        // camera.zoom = Lerp(camera.zoom, newZoom - camera.zoom, GetFrameTime() * 10);
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
        for (int i = 0; i < NUMBER_EIGHT; i++) // Number player
        {
            if (outsidePlayer && ColorToInt(players[i].color) == ColorToInt(outsidePlayer->color))
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

    // End the round only when zero or one unique team remains alive.
    const int aliveTeam = ScoreResolveAliveTeam(aliveTeams);
    if (!roundResultActive && ScoreRoundHasResult(numberActiveColor, aliveTeam))
    {
        BeginRoundResult(aliveTeam, true);
    }
    if (roundResultActive)
    {
        elapsedTime = GetTime() - startTime;
        if (elapsedTime > 3.0)
        {
            if (winnerMap)
            {
                idMap++;
                SwitchMap();
            }
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

void ResetGame()
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
            memset(players[i].movementParticles, 0, sizeof players[i].movementParticles);
            players[i].movementParticleCursor = 0;
            players[i].movementParticleCooldown = 0.0f;
            players[i].movementParticlePositionReady = false;
            players[i].speed.x = 3.05f;
            players[i].speed.y = 3.05f;
            for (int j = 0; j < MAX_BULLET; j++)
            {
                players[i].bullets[j].inactive = true;
            }
            GamepadPlayerColor(players[i].gamepadId, players[i].color.r, players[i].color.g, players[i].color.b);
            GamepadPlayerLife(players[i].gamepadId, players[i].life);
            GamepadPlayerAmmunition(players[i].gamepadId, players[i].ammunition);
            if (defaultTypeItem > -1)
            {
                InitItemWithTypeItem(i + 1, defaultTypeItem, defaultMaxTimerItem);
            }
        }
    }
    startTime = 0.0;
    elapsedTime = 0.0;
    roundResultActive = false;
    roundIsDraw = false;
    roundWinningTeam = SCORE_NO_TEAM;
    winnerMap = false;
}

void DrawGameplay()
{
    const Camera2D renderCamera = GetRenderCamera();

    // DRAW GAME
    BeginMode2D(renderCamera); // DRAW 1

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
    if (backgroundShaderReady && !activePerf)
    {
        const float shaderTime = (float)GetTime();
        const float shaderResolution[2] = {(float)GetScreenWidth(), (float)GetScreenHeight()};
        const Vector2 viewportTopLeft = GetScreenToWorld2D((Vector2){0.0f, 0.0f}, renderCamera);
        const Vector2 viewportBottomRight = GetScreenToWorld2D(
            (Vector2){(float)GetScreenWidth(), (float)GetScreenHeight()},
            renderCamera);
        const Rectangle viewport = {
            viewportTopLeft.x,
            viewportTopLeft.y,
            viewportBottomRight.x - viewportTopLeft.x,
            viewportBottomRight.y - viewportTopLeft.y};

        SetShaderValue(backgroundShader, backgroundShaderTimeLoc, &shaderTime, SHADER_UNIFORM_FLOAT);
        SetShaderValue(backgroundShader, backgroundShaderResolutionLoc, shaderResolution, SHADER_UNIFORM_VEC2);
        BeginShaderMode(backgroundShader);
        DrawRectangleRec(viewport, WHITE);
        EndShaderMode();
    }
    DrawCircleGradient((Vector2){arenaSizeX / 2.0f, arenaSizeY / 2.0f}, arenaSizeX + 300.0f, Fade(BLACK, 0.6f), Fade(BLACK, 0.0f));
    DrawRectangle(-2, -2, arenaSizeX + 4, arenaSizeY + 4, Fade(GRAY, 0.5f));
    if (activePerf)
    {
        DrawRectangle(0, 0, arenaSizeX, arenaSizeY, BLACKGROUND);
    }
    else
    {
        if (mapTextureLoaded)
        {
            DrawTextureRec(mapTexture.texture, (Rectangle){0.0f, 0.0f, (float)mapTexture.texture.width, (float)-mapTexture.texture.height}, (Vector2){0.0f, 0.0f}, WHITE);
        }
        else
        {
            render_map(map);
        }
    }
    DrawDynamicGrass(renderCamera);
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
        DrawPlayerMovementParticles(&players[i]);
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
    if (numberPlayer == 0 || roundResultActive || pauseGame || activeDev)
    {
        BeginDrawing(); // DRAW 2

        if (numberPlayer == 0 || roundResultActive || pauseGame) // Background for title / win
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

    if (numberPlayer == 0 || roundResultActive || pauseGame)
    {
        BeginMode2D(renderCamera); // DRAW 3

        // Draw Wins
        if (roundResultActive)
        {
            ScoreDrawResult(camera.target, arenaSizeX, BLACKGROUND, colorScore, themeColor,
                            maxScore, bestScore, roundIsDraw, roundWinningTeam);
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

void DrawGameArena()
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

void GenerateQrCode()
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
        free((void *)url);
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
