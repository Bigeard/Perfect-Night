#include "../../../../../lib/raylib/src/raylib.h"
#include "../../../../../lib/raylib/src/raymath.h"

#include "./laser.h"
#include "../../item.h"
#include "../../../gameplay/gameplay.h"
#include "../../../player/player.h"
#include "../../../tool/tool.h"
#include "../../../ray_collisions/ray_collisions.h"

Item InitItemLaser(int player_id, float maxTimer)
{
    float defaultMaxTimer = 8.0f;
    if (maxTimer > -1.0f)
        defaultMaxTimer = maxTimer;

    return (Item){
        player_id,
        "Laser",
        true,
        GetTime(),
        defaultMaxTimer,
        false,
        false,
        ShootItemLaser,
        UpdateItemLaser,
        DrawItemLaser};
}

void ShootItemLaser(Item *item, float calcPosRadianX, float calcPosRadianY, float delta_x, float delta_y)
{
    Player *player = &players[item->player_id - 1];
    player->bullets[player->lastBullet] = (Bullet){
        player->id,
        (Physic){
            {player->p.pos.x + 20.0f + calcPosRadianX * 22.0f - 3.0f, player->p.pos.y + 20.0f + calcPosRadianY * 22.0f - 3.0f},
            {3.0f, 3.0f},
            {cosf(player->lastRadian) * 26, sinf(player->lastRadian) * 26},
            {false, false, false, false, false}},
        {26, 26},
        player->lastRadian,
        false,
        false,
        true,
        (Vector2){
            player->p.pos.x + player->p.size.x / 2,
            player->p.pos.y + player->p.size.y / 2},
        500.0f,
        player->color};
}

void UpdateItemLaser(Item *item)
{
    if (!item->active)
        return;

    Player *player = &players[item->player_id - 1];
    for (int i = 0; i < MAX_BULLET; i++)
    {
        if (!player->bullets[i].playerId && player->bullets[i].inactive)
            continue;

        float endPosX = player->bullets[i].p.pos.x + player->bullets[i].p.size.x / 2;
        float endPosY = player->bullets[i].p.pos.y + player->bullets[i].p.size.y / 2;
        float distance = sqrtf(powf(player->bullets[i].startPos.x - endPosX, 2.0f) + powf(player->bullets[i].startPos.y - endPosY, 2.0f));
        if (distance >= player->bullets[i].distanceLaser)
        {
            distance = player->bullets[i].distanceLaser;
        }
        float startPosX = endPosX - distance * cosf(player->bullets[i].radian);
        float startPosY = endPosY - distance * sinf(player->bullets[i].radian);

        if (startPosX != endPosX && startPosY != endPosY)
            player->bullets[i].distanceLaser -= 10.0f;

        if (player->bullets[i].p.collision[0])
            player->bullets[i].inactive = true;
    }

    if (item->maxTimer < GetTime() - item->timer)
    {
        item->active = false;
    }
}

void DrawItemLaser(Item *item)
{
    if (!item->active)
        return;
    const Player player = players[item->player_id - 1];

    // Init Color
    Color color = player.color;
    Color whiteColor = WHITE;
    Color blackColor = BLACK;

    if (player.invincible)
    {
        // Lighten color
        color = LightenColor(color, 0.5);
        whiteColor = Fade(WHITE, 0.5);
    }

    // Draw BLACK 1 Cannon
    DrawRectanglePro((Rectangle){player.p.pos.x + 20, player.p.pos.y + 20, 33, 10}, (Vector2){0, 5}, player.radian * (180 / PI), blackColor);
    DrawRectanglePro((Rectangle){player.p.pos.x + 20, player.p.pos.y + 20, 16, 16}, (Vector2){-25, 8}, player.radian * (180 / PI), blackColor);

    // Draw Border of the Tank
    DrawTextureEx(playerBorderTexture, (Vector2){player.p.pos.x, player.p.pos.y}, 0, 1, whiteColor);

    // Draw WHITE 2 and COLOR 3 Cannon
    DrawRectanglePro((Rectangle){player.p.pos.x + 20, player.p.pos.y + 20, 32, 8}, (Vector2){0, 4}, player.radian * (180 / PI), WHITE);
    DrawRectanglePro((Rectangle){player.p.pos.x + 20, player.p.pos.y + 20, 14, 14}, (Vector2){-26, 7}, player.radian * (180 / PI), WHITE);

    DrawRectanglePro((Rectangle){player.p.pos.x + 20, player.p.pos.y + 20, 30, 4}, (Vector2){0, 2}, player.radian * (180 / PI), color);
    DrawRectanglePro((Rectangle){player.p.pos.x + 20, player.p.pos.y + 20, 10, 10}, (Vector2){-28, 5}, player.radian * (180 / PI), color);

    // Draw Body / Template of the tank
    DrawTextureEx(playerBodyTexture, (Vector2){player.p.pos.x + 3, player.p.pos.y + 3}, 0, 1, color);
    DrawTexturePro(playerTemplatesTextures[player.id], (Rectangle){0, 0, 32, 32}, (Rectangle){player.p.pos.x + 20, player.p.pos.y + 20, 32, 32}, (Vector2){16, 16}, player.radian * (180 / PI) + 90, WHITE);

    for (int a = 0; a < player.ammunition; a++)
    {
        float calcRadian = player.radian + PI + (a * 0.7 + 0.7 / 2 - (player.ammunition * 0.7 / 2));
        float ammunitionPosX = (player.p.pos.x + player.p.size.x / 2.0f) + 25 * cosf(calcRadian);
        float ammunitionPosY = (player.p.pos.y + player.p.size.x / 2.0f) + 25 * sinf(calcRadian);
        DrawCircle(ammunitionPosX, ammunitionPosY, 7.3, BLACK);
        DrawCircle(ammunitionPosX, ammunitionPosY, 6.5, WHITE);
        DrawCircle(ammunitionPosX, ammunitionPosY, 5, DarkenColor(player.color, 0.7));
    }

    // Animation Bullet
    for (int i = 0; i < MAX_BULLET; i++)
    {
        if (!player.bullets[i].playerId)
            continue;
        float endPosX = player.bullets[i].p.pos.x + player.bullets[i].p.size.x / 2;
        float endPosY = player.bullets[i].p.pos.y + player.bullets[i].p.size.y / 2;

        float distance = sqrtf(powf(player.bullets[i].startPos.x - endPosX, 2.0f) + powf(player.bullets[i].startPos.y - endPosY, 2.0f));
        if (distance >= player.bullets[i].distanceLaser)
        {
            distance = player.bullets[i].distanceLaser;
        }
        float startPosX = endPosX - distance * cosf(player.bullets[i].radian);
        float startPosY = endPosY - distance * sinf(player.bullets[i].radian);

        DrawLine(
            startPosX,
            startPosY,
            endPosX,
            endPosY,
            player.color);
    }

    // Animation pick-up item
    double elapsedTime = GetTime() - item->timer;
    if (elapsedTime <= 4.0)
    {
        DrawTexture(
            LaserTexture,
            (player.p.pos.x + player.p.size.x / 2.0f) - 14.0f,
            player.p.pos.y - 34.0f,
            Fade(player.color, 1.0f - ((float)elapsedTime / 4.0f)));
    }
}
