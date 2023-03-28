#include "../../../lib/raylib/src/raylib.h"
#include "../../../lib/raylib/src/raymath.h"
#include <emscripten/emscripten.h>
#include "stdio.h"
#include "string.h"

#include "player.h"
#include "../gameplay/gameplay.h"
#include "../bullet/bullet.h"
#include "../particle/particle.h"
#include "../tool/tool.h"

// @TODO optimize the code
EM_JS(float, GetJoystickMobileLeftX, (const char *id), {return listGamepad.get(Module.UTF8ToString(id)).axes[0]});
EM_JS(float, GetJoystickMobileLeftY, (const char *id), {return listGamepad.get(Module.UTF8ToString(id)).axes[1]});
EM_JS(float, GetJoystickMobileRightX, (const char *id), {return listGamepad.get(Module.UTF8ToString(id)).axes[2]});
EM_JS(float, GetJoystickMobileRightY, (const char *id), {return listGamepad.get(Module.UTF8ToString(id)).axes[3]});

EM_JS(void, GamepadPlayerLife, (char *p_id, int life), {
    const id = Module.UTF8ToString(p_id);
    const gamepad = listGamepad.get(id);
    if (gamepad)
    {
        gamepad.life = life;
        gamepad.edit = true;
        listGamepad.set(id, gamepad);
    }
});

EM_JS(void, GamepadPlayerAmmunition, (char *p_id, int ammunition), {
    const id = Module.UTF8ToString(p_id);
    const gamepad = listGamepad.get(id);
    gamepad.ammunition = ammunition;
    gamepad.edit = true;
    listGamepad.set(id, gamepad);
});

Texture2D playerBorderTexture;
Texture2D playerBodyTexture;
Texture2D playerTemplatesTextures[9];

void InitPlayer(void)
{
    playerBorderTexture = LoadTexture("resources/player_border.png");
    playerBodyTexture = LoadTexture("resources/player_body.png");
    playerTemplatesTextures[0] = LoadTexture("resources/player_template.png");
    playerTemplatesTextures[1] = LoadTexture("resources/player_1_template.png");
    playerTemplatesTextures[2] = LoadTexture("resources/player_2_template.png");
    playerTemplatesTextures[3] = LoadTexture("resources/player_3_template.png");
    playerTemplatesTextures[4] = LoadTexture("resources/player_4_template.png");
    playerTemplatesTextures[5] = LoadTexture("resources/player_5_template.png");
    playerTemplatesTextures[6] = LoadTexture("resources/player_6_template.png");
    playerTemplatesTextures[7] = LoadTexture("resources/player_7_template.png");
    playerTemplatesTextures[8] = LoadTexture("resources/player_8_template.png");
}

void UpdatePlayer(Player *player)
{
    float delta = GetFrameTime();

    if (player->life <= 0)
    {
        // @TODO Take position of winner
        return;
    }

    // Load Invincible
    if (player->invincible > 1)
    {
        player->invincible = player->invincible - delta;
    }
    else
    {
        player->invincible = 0;
    }

    // Load Ammunition
    if (player->ammunition < maxAmmunition)
    {
        player->ammunitionLoad = player->ammunitionLoad - delta;
    }
    if (player->ammunitionLoad <= 0.0f)
    {
        player->ammunition++;
        player->ammunitionLoad = DELAY_AMMUNITION;
        GamepadPlayerAmmunition(player->gamepadId, player->ammunition);
    }

    player->p.collision[0] = false;
    player->p.collision[1] = false;
    player->p.collision[2] = false;
    player->p.collision[3] = false;
    player->p.collision[4] = false;

    // MOBILE CONTROLLER
    if (player->INPUT_TYPE == MOBILE)
    {
        const float joystickLeftX = GetJoystickMobileLeftX(player->gamepadId) * 0.02f;
        const float joystickLeftY = GetJoystickMobileLeftY(player->gamepadId) * 0.02f;
        const float joystickRightX = GetJoystickMobileRightX(player->gamepadId) * 0.02f;
        const float joystickRightY = GetJoystickMobileRightY(player->gamepadId) * 0.02f;
        const float distance = sqrtf(powf(joystickRightY, 2.0f) + powf(joystickRightX, 2.0f));

        // Move Player
        player->p.vel.x = player->speed.x * joystickLeftX;
        player->p.vel.y = player->speed.y * joystickLeftY;

        // Move Cannon
        if (joystickLeftX != 0.0f || joystickLeftY != 0.0f)
        {
            player->radian = atan2f(joystickLeftY, joystickLeftX);
        }

        if ((joystickRightX != 0.0f || joystickRightY != 0.0f) && distance >= 0.1f)
        {
            player->radian = atan2f(joystickRightY, joystickRightX);
            player->lastRadian = atan2f(joystickRightY, joystickRightX);
        }
        if (distance <= 0.1f && distance > 0.01f)
        {
            player->lastRadian = player->radian;
        }

        // Charge / Shoot Bullet
        if (distance > 0.001 && player->ammunition > 0)
        {
            player->timeShoot += 2;
            if (player->charge < 15 && ((int)(player->timeShoot * delta * 10) % 2) == 1 && player->life > 0)
            {
                player->charge = player->charge + 0.5f;
                player->timeShoot = 0;
                player->speed = (Vector2){1.2f, 1.2f};
            }
        }
        else
        {
            if (player->charge > 2.0f)
            {
                // Loop bullet (Allow the ball to be replaced one after the other)
                player->lastBullet += 1;
                if (player->lastBullet >= MAX_BULLET)
                {
                    player->lastBullet = 0;
                }

                float calcPosRadianX = cosf(player->lastRadian);
                float calcPosRadianY = sinf(player->lastRadian);

                if ((calcPosRadianX > 0.6f && calcPosRadianX < 0.8f) ||
                    (calcPosRadianX < -0.6f && calcPosRadianX > -0.8f))
                {
                    calcPosRadianX = lroundf(cosf(player->lastRadian));
                }
                if ((calcPosRadianY > 0.6f && calcPosRadianY < 0.8f) ||
                    (calcPosRadianY < -0.6f && calcPosRadianY > -0.8f))
                {
                    calcPosRadianY = lroundf(sinf(player->lastRadian));
                }

                bool defaultShoot = true;
                if (player->item.active)
                {
                    player->item.ShootItem(&player->item, calcPosRadianX, calcPosRadianY, player->p.vel.x, player->p.vel.y);
                    defaultShoot = player->item.defaultShoot;
                }
                if (defaultShoot)
                {
                    player->bullets[player->lastBullet] = (Bullet){
                        player->id,
                        (Physic){
                            {player->p.pos.x + 20.0f + calcPosRadianX * 22.0f - 5.0f, player->p.pos.y + 20.0f + calcPosRadianY * 22.0f - 5.0f},
                            {5.0f, 5.0f},
                            {cosf(player->lastRadian) * player->charge + player->p.vel.x * 2.0f, sinf(player->lastRadian) * player->charge + player->p.vel.y * 2.0f},
                            {false, false, false, false, false}},
                        {player->charge + player->p.vel.x * 2.0f, player->charge + player->p.vel.y * 2.0f},
                        player->lastRadian,
                        false,
                        false,
                        0,
                        true,
                        (Vector2){
                            player->p.pos.x + player->p.size.x / 2,
                            player->p.pos.y + player->p.size.y / 2,
                        },
                        0.0f,
                        player->color};
                }
                // Remove ammunition
                player->ammunition--;
                player->canShoot = false;
                player->speed = (Vector2){3.05f, 3.05f};
                GamepadPlayerAmmunition(player->gamepadId, player->ammunition);
            }
            player->charge = 2.0f;
        }
    }

    player->p.pos.x += player->p.vel.x;
    player->p.pos.y += player->p.vel.y;

    if (player->item.active)
    {
        player->item.UpdateItem(&player->item);
    }

    // Out of area
    if ((player->p.pos.x >= arenaSizeX ||
         player->p.pos.x + player->p.size.x <= 0.0f) ||
        (player->p.pos.y >= arenaSizeY ||
         player->p.pos.y + player->p.size.y <= 0.0f))
    {

        if (lastOutsidePlayer->id != player->id)
        {
            outsidePlayer = player;
        }
    }
    else
    {
        if (outsidePlayer->id == player->id)
        {
            outsidePlayer = NULL;
        }
    }

    if (player->life <= 0 && outsidePlayer->id == player->id)
    {
        outsidePlayer = NULL;
    }
}

void CollisionBulletPlayer(bool bulletCollision, Bullet *bullet, Player *player, Rectangle recPlayer)
{
    if (bullet->inactive)
        return;
    if (player->life <= 0)
        return;
    if (player->invincible == 0 && ColorToInt(bullet->COLOR) != ColorToInt(player->color))
    {
        if (bulletCollision)
        {
            player->life--;
            player->invincible = DELAY_INVINCIBLE;
            GamepadPlayerLife(player->gamepadId, player->life);
            InitParticles(bullet->p.pos, bullet->p.vel, 0.1f, player->color, 120.0f, player->shootParticle, 20);
        }
    }
}

void DrawPlayer(Player player)
{
    if (!player.id)
        return;
    if (player.life <= 0)
        return;

    // Init Color
    Color color = player.color;
    Color whiteColor = WHITE;
    Color blackColor = BLACK;

    if (player.invincible)
    {
        // Lighten color
        color = LightenColor(color, 0.5f);
        whiteColor = Fade(WHITE, 0.5f);
    }

    bool defaultDisplay = true;
    if (player.item.active)
    {
        defaultDisplay = player.item.defaultDisplay;
    }

    if (defaultDisplay)
    {
        // Draw BLACK 1 Cannon
        DrawRectanglePro((Rectangle){player.p.pos.x + 20.0f, player.p.pos.y + 20.0f, 33.0f, 14.0f}, (Vector2){0.0f, 7.0f}, player.radian * (180.0f / PI), blackColor);

        // Draw Body of the Tank
        DrawTextureEx(playerBorderTexture, (Vector2){player.p.pos.x, player.p.pos.y}, 0.0f, 1.0f, whiteColor);

        // Draw WHITE 2 and COLOR 3 Cannon
        DrawRectanglePro((Rectangle){player.p.pos.x + 20.0f, player.p.pos.y + 20.0f, 32.0f, 12.0f}, (Vector2){0.0f, 6.0f}, player.radian * (180.0f / PI), WHITE);
        DrawRectanglePro((Rectangle){player.p.pos.x + 20.0f, player.p.pos.y + 20.0f, 30.0f, 8.0f}, (Vector2){0.0f, 4.0f}, player.radian * (180.0f / PI), color);

        // Draw Face / Template of the tank
        DrawTextureEx(playerBodyTexture, (Vector2){player.p.pos.x + 3.0f, player.p.pos.y + 3.0f}, 0.0f, 1.0f, color);
        DrawTexturePro(playerTemplatesTextures[player.id], (Rectangle){0.0f, 0.0f, 32.0f, 32.0f}, (Rectangle){player.p.pos.x + 20.0f, player.p.pos.y + 20.0f, 32.0f, 32.0f}, (Vector2){16.0f, 16.0f}, player.radian * (180.0f / PI) + 90.0f, WHITE);

        for (int a = 0; a < player.ammunition; a++)
        {
            float calcRadian = player.radian + PI + ((float)a * 0.7f + 0.7f / 2.0f - ((float)player.ammunition * 0.7f / 2.0f));
            float ammunitionPosX = (player.p.pos.x + player.p.size.x / 2.0f) + 25.0f * cosf(calcRadian);
            float ammunitionPosY = (player.p.pos.y + player.p.size.x / 2.0f) + 25.0f * sinf(calcRadian);
            DrawCircle(ammunitionPosX, ammunitionPosY, 7.3f, BLACK);
            DrawCircle(ammunitionPosX, ammunitionPosY, 6.5f, WHITE);
            DrawCircle(ammunitionPosX, ammunitionPosY, 3.0f, DarkenColor(player.color, 0.7f));
        }

        // Draw the progress bar of the charge
        if (player.charge != 2.0f)
        {
            DrawRing(
                (Vector2){player.p.pos.x + player.p.size.x / 2.0f, player.p.pos.y + player.p.size.y / 2.0f},
                38.0f,
                47.0f,
                player.radian * (180.0f / PI) * -1.0f + 270.0f - (player.charge - 2.0f) * 4.0f - 2.0f,
                player.radian * (180.0f / PI) * -1.0f + 270.0f + (player.charge - 2.0f) * 4.0f + 2.0f,
                0,
                Fade(WHITE, 0.6f));

            DrawRing(
                (Vector2){player.p.pos.x + player.p.size.x / 2.0f, player.p.pos.y + player.p.size.y / 2.0f},
                40.0f,
                45.0f,
                player.radian * (180.0f / PI) * -1.0f + 270.0f - (player.charge - 2.0f) * 4.0f,
                player.radian * (180.0f / PI) * -1.0f + 270.0f + (player.charge - 2.0f) * 4.0f,
                0,
                Fade(player.color, 0.8f));
        }
    }

    if (player.item.active)
    {
        player.item.DrawItem(&player.item);
    }

    if (activeDev)
    {
        DrawRectangleRec((Rectangle){player.p.pos.x, player.p.pos.y, player.p.size.x, player.p.size.y}, Fade(PURPLE, 0.5f));
    }
}

void DrawSpawnPlayer(Player player)
{
    const char *text = TextFormat("P%d", player.id);
    int sizeText = MeasureText(text, 20);
    DrawRectangleLinesEx((Rectangle){player.spawn.x, player.spawn.y, player.p.size.x, player.p.size.y}, 2.5f, Fade(player.color, 0.4f));
    DrawText(text, player.spawn.x + player.p.size.x / 2 - sizeText / 2, player.spawn.y + player.p.size.y / 2 - 8, 20, Fade(player.color, 0.4f));
}

void DrawStatsPlayer(Player player)
{
    Color colorDisplay = player.color;
    if (player.life < 1)
    {
        colorDisplay = Fade(player.color, 0.2f);
    }
    if (player.id % 2)
    {
        DrawRectanglePro((Rectangle){300.0f, 80.0f + 120.0f * (float)player.id, 64.0f, 72.0f}, (Vector2){66.0f, 6.0f}, -270.0f, WHITE);
        DrawRectanglePro((Rectangle){300.0f, 80.0f + 120.0f * (float)player.id, 210.0f, 110.0f}, (Vector2){205.0f, 5.0f}, 20.0f, WHITE);
        DrawRectanglePro((Rectangle){300.0f, 80.0f + 120.0f * (float)player.id, 210.0f, 110.0f}, (Vector2){205.0f, 5.0f}, 25.0f, WHITE);

        DrawRectanglePro((Rectangle){300.0f, 80.0f + 120.0f * (float)player.id, 60.0f, 60.0f}, (Vector2){60.0f, 0.0f}, -270.0f, colorDisplay);
        DrawRectanglePro((Rectangle){300.0f, 80.0f + 120.0f * (float)player.id, 200.0f, 100.0f}, (Vector2){200.0f, 0.0f}, 20.0f, colorDisplay);
        DrawRectanglePro((Rectangle){300.0f, 80.0f + 120.0f * (float)player.id, 200.0f, 100.0f}, (Vector2){200.0f, 0.0f}, 25.0f, colorDisplay);

        if (player.id == 1)
            DrawText(TextFormat("P%d", player.id), 256, 30 + 120 * player.id, 30, WHITE);
        else
            DrawText(TextFormat("P%d", player.id), 253, 30 + 120 * player.id, 30, WHITE);

        DrawRectanglePro((Rectangle){300.0f, 80.0f + 120.0f * player.id, 190.0f, 90.0f}, (Vector2){195.0f, -5.0f}, 20.0f, WHITE);

        DrawTextPro(GetFontDefault(), TextFormat("%d", player.ammunition), (Vector2){180.0f, 185.0f + 120.0f * (float)(player.id - 1)}, (Vector2){40.0f, 0.0f}, 20.0f, 60.0f, 10.0f, colorDisplay);
        DrawTextPro(GetFontDefault(), TextFormat("%.1f", player.ammunitionLoad), (Vector2){250.0f, 190.0f + 120.0f * (float)(player.id - 1)}, (Vector2){40.0f, 0.0f}, 20.0f, 40.0f, 10.0f, colorDisplay);
        DrawTextPro(GetFontDefault(), TextFormat("%d", player.life), (Vector2){180.0f, 100.0f + 120.0f * (float)(player.id - 1)}, (Vector2){40.0f, 0.0f}, 20.0f, 40.0f, 10.0f, colorDisplay);
    }
    else
    {
        DrawRectanglePro((Rectangle){GetScreenWidth() - 300.0f, 80.0f + 120.0f * (float)(player.id - 1), 64.0f, 72.0f}, (Vector2){-2.0f, 6.0f}, 270.0f, WHITE);
        DrawRectanglePro((Rectangle){GetScreenWidth() - 300.0f, 80.0f + 120.0f * (float)(player.id - 1), 210.0f, 110.0f}, (Vector2){5.0f, 5.0f}, -20.0f, WHITE);
        DrawRectanglePro((Rectangle){GetScreenWidth() - 300.0f, 80.0f + 120.0f * (float)(player.id - 1), 210.0f, 110.0f}, (Vector2){5.0f, 5.0f}, -25.0f, WHITE);

        DrawRectanglePro((Rectangle){GetScreenWidth() - 300.0f, 80.0f + 120.0f * (float)(player.id - 1), 60.0f, 60.0f}, (Vector2){0.0f, 0.0f}, 270.0f, colorDisplay);
        DrawRectanglePro((Rectangle){GetScreenWidth() - 300.0f, 80.0f + 120.0f * (float)(player.id - 1), 200.0f, 100.0f}, (Vector2){0.0f, 0.0f}, -20.0f, colorDisplay);
        DrawRectanglePro((Rectangle){GetScreenWidth() - 300.0f, 80.0f + 120.0f * (float)(player.id - 1), 200.0f, 100.0f}, (Vector2){0.0f, 0.0f}, -25.0f, colorDisplay);

        DrawText(TextFormat("P%d", player.id), GetScreenWidth() - 288, 30 + 120 * (player.id - 1), 30, WHITE);

        DrawRectanglePro((Rectangle){GetScreenWidth() - 300.0f, 80.0f + 120.0f * (float)(player.id - 1), 190.0f, 90.0f}, (Vector2){-5.0f, -5.0f}, -20.0f, WHITE);

        DrawTextPro(GetFontDefault(), TextFormat("%d", player.ammunition), (Vector2){GetScreenWidth() - 130.0f, 45.0f + 120.0f * (float)(player.id - 1)}, (Vector2){40.0f, 0.0f}, -20.0f, 60.0f, 10.0f, colorDisplay);
        DrawTextPro(GetFontDefault(), TextFormat("%.1f", player.ammunitionLoad), (Vector2){GetScreenWidth() - 250.0f, 70.0f + 120.0f * (float)(player.id - 1)}, (Vector2){40.0f, 0.0f}, -20.0f, 40.0f, 10.0f, colorDisplay);
        DrawTextPro(GetFontDefault(), TextFormat("%d", player.life), (Vector2){GetScreenWidth() - 130.0f, -40.0f + 120.0f * (float)(player.id - 1)}, (Vector2){40.0f, 0.0f}, -20.0f, 40.0f, 10.0f, colorDisplay);
    }
}

void PlayerValueToData(Player player, char *dataToSend)
{
    if (player.id)
        strcat(dataToSend, TextFormat("%f,%f,%f,%f,%d,%d,%d,%d,",
                                      player.p.pos.x,
                                      player.p.pos.y,
                                      player.radian,
                                      player.charge,
                                      player.item.active ? (int)player.item.type : 0,
                                      player.life,
                                      player.ammunition,
                                      colorScore[player.id-1]));
    // TraceLog(LOG_INFO, TextFormat("%s", dataToSend));
}
