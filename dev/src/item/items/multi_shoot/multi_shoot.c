#include "../../../../../lib/raylib/src/raylib.h"
#include "../../../../../lib/raylib/src/raymath.h"

#include "./multi_shoot.h"
#include "../../item.h"
#include "../../../gameplay/gameplay.h"
#include "../../../player/player.h"
#include "../../../tool/tool.h"

Item InitItemMultiShoot(int player_id)
{
    return (Item){
        player_id,
        "Multi Shoot",
        true,
        GetTime(),
        8,
        false,
        false,
        ShootItemMultiShoot,
        UpdateItemMultiShoot,
        DrawItemMultiShoot,
        {3}};
}

void ShootItemMultiShoot(Item *item, float calcPosRadianX, float calcPosRadianY, float delta_x, float delta_y)
{
    if (!item->active)
        return;
    Player *player = &players[item->player_id - 1];
    for (int i = 0; i < item->multiShoot.numCannon; i++)
    {
        // @TODO Fix bug direction for eg with 7 cannons
        // TraceLog(LOG_INFO, "%d", (int)floor((float)item->multiShoot.numCannon/2));
        float radian = (player->lastRadian * (180 / PI) - 40 * (int)floor((float)item->multiShoot.numCannon / 2) + i * 40) / (180 / PI);
        // calcPosRadianX = (calcPosRadianX * (180 / PI) - 40 * (int)floor((float)item->multiShoot.numCannon/2) + i * 40) / (180 / PI);
        // calcPosRadianY = (calcPosRadianY * (180 / PI) - 40 * (int)floor((float)item->multiShoot.numCannon/2) + i * 40) / (180 / PI);
        player->bullets[player->lastBullet] = (Bullet){
            player->id,
            (Physic){
                {player->p.pos.x + 20 + calcPosRadianX * 22 - 5, player->p.pos.y + 20 + calcPosRadianY * 22 - 5},
                {5, 5},
                {cosf(radian) * player->charge + delta_x * 2, sinf(radian) * player->charge + delta_y * 2},
                {0, 0, 0, 0, 0}},
            {player->charge + delta_x * 2, player->charge + delta_y * 2},
            radian,
            false,
            false,
            true,
            player->color};
        player->lastBullet += 1;
        if (player->lastBullet >= sizeof(player->bullets) / sizeof(player->bullets[0]))
        {
            player->lastBullet = 0;
        }
    }
}

void UpdateItemMultiShoot(Item *item)
{
    if (!item->active)
        return;
    double elapsedTime = GetTime() - item->timer;
    if (item->maxTimer < elapsedTime)
    {
        item->active = false;
    }
}

void DrawItemMultiShoot(Item *item)
{
    if (!item->active)
        return;
    Player player = players[item->player_id - 1];

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
    for (int i = 0; i < item->multiShoot.numCannon; i++)
    {
        DrawRectanglePro((Rectangle){player.p.pos.x + 20, player.p.pos.y + 20, 33, 14}, (Vector2){0, 7}, player.radian * (180 / PI) - 40 * (int)floor((float)item->multiShoot.numCannon / 2) + i * 40, blackColor);
    }
    // Draw Border of the Tank
    DrawTextureEx(playerBorderTexture, (Vector2){player.p.pos.x, player.p.pos.y}, 0, 1, whiteColor);

    // Draw WHITE 2 and COLOR 3 Cannon
    for (int i = 0; i < item->multiShoot.numCannon; i++)
    {
        DrawRectanglePro((Rectangle){player.p.pos.x + 20, player.p.pos.y + 20, 32, 12}, (Vector2){0, 6}, player.radian * (180 / PI) - 40 * (int)floor((float)item->multiShoot.numCannon / 2) + i * 40, WHITE);
    }
    for (int i = 0; i < item->multiShoot.numCannon; i++)
    {
        DrawRectanglePro((Rectangle){player.p.pos.x + 20, player.p.pos.y + 20, 30, 8}, (Vector2){0, 4}, player.radian * (180 / PI) - 40 * (int)floor((float)item->multiShoot.numCannon / 2) + i * 40, color);
    }

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

    // Draw the progress bar of the charge
    if (player.charge != 2)
    {
        DrawRing(
            (Vector2){player.p.pos.x + player.p.size.x / 2, player.p.pos.y + player.p.size.y / 2},
            38.0f,
            47.0f,
            player.radian * (180 / PI) * -1 + 270 - (player.charge - 2) * 4 - 2,
            player.radian * (180 / PI) * -1 + 270 + (player.charge - 2) * 4 + 2,
            0,
            Fade(WHITE, 0.6));

        DrawRing(
            (Vector2){player.p.pos.x + player.p.size.x / 2, player.p.pos.y + player.p.size.y / 2},
            40.0f,
            45.0f,
            player.radian * (180 / PI) * -1 + 270 - (player.charge - 2) * 4,
            player.radian * (180 / PI) * -1 + 270 + (player.charge - 2) * 4,
            0,
            Fade(player.color, 0.8));
    }
}
