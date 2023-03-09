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
        DrawItemLaser,
        {1.0f, 0.0f}};
}

void ShootItemLaser(Item *item, float calcPosRadianX, float calcPosRadianY, float delta_x, float delta_y)
{
    Player *player = &players[item->player_id - 1];
    // item->laser.laserRadian = player.lastRadian;
    // item->laser.laserStartPosX = player.p.pos.x + player.p.size.x / 2;
    // item->laser.laserStartPosY = player.p.pos.y + player.p.size.y / 2;
    // item->laser.animationTimerLaser = 20000.0f;

    player->bullets[player->lastBullet] = (Bullet){
        player->id,
        (Physic){
            {player->p.pos.x + 20.0f + calcPosRadianX * 22.0f - 2.5f, player->p.pos.y + 20.0f + calcPosRadianY * 22.0f - 2.5f},
            {2.5f, 2.5f},
            {cosf(player->lastRadian) * 20, sinf(player->lastRadian) * 20},
            {false, false, false, false, false}},
        {20, 20},
        player->lastRadian,
        false,
        false,
        true,
        player->color};
}

void UpdateItemLaser(Item *item)
{
    if (!item->active)
        return;

    if (item->laser.animationTimerLaser > 0)
    {
        item->laser.animationTimerLaser -= 0.1;
    }

    // Vector2 ray_origin = {
    //     item->laser.laserStartPosX,
    //     item->laser.laserStartPosY};
    // Vector2 ray_dir = {
    //     item->laser.laserStartPosX + (10000 * cosf(item->laser.laserRadian)),
    //     item->laser.laserStartPosY + (10000 * sinf(item->laser.laserRadian))};
    // Vector2 contact_point = (Vector2){0.0f, 0.0f};
    // Vector2 contact_normal = (Vector2){0.0f, 0.0f};
    // float near_contact_time = 0.0f;
    // Vector2 probableContactPoints[2];

    // float rayDistance = sqrtf(powf(item->laser.laserStartPosX - ray_dir.x, 2.0f) + powf(item->laser.laserStartPosX - ray_dir.y, 2.0f));

    // // if there are collision with a box
    // for (int i = 0; i < boxesLength; i++)
    // {
    //     const Rectangle recBox = {boxes[i].p.pos.x, boxes[i].p.pos.y, boxes[i].p.size.x, boxes[i].p.size.y};
    //     if (boxes[i].collision && RayVsRect2D(ray_origin, ray_dir, recBox, &contact_point, &contact_normal, &near_contact_time, probableContactPoints))
    //     {
    //         float contactDistance = sqrtf(powf(item->laser.laserStartPosX - contact_point.x, 2.0f) + powf(item->laser.laserStartPosX - contact_point.y, 2.0f));
    //         if (rayDistance > contactDistance)
    //         {
    //             ray_dir.x = contact_point.x;
    //             ray_dir.y = contact_point.y;
    //             rayDistance = contactDistance;
    //         }

    //         if (item->laser.laserStartPosX == item->laser.laserEndPosX && item->laser.laserStartPosY == item->laser.laserEndPosY)
    //         {
    //             item->laser.animationTimerLaser = 0.0f;
    //         }
    //     }
    // }

    // // if there are collision with a player
    // for (int i = 0; i < NUMBER_EIGHT; i++)
    // {
    //     const Rectangle recPlayer = {players[i].p.pos.x, players[i].p.pos.y, players[i].p.size.x, players[i].p.size.y};
    //     if (RayVsRect2D(ray_origin, ray_dir, recPlayer, &contact_point, &contact_normal, &near_contact_time, probableContactPoints))
    //     {
    //         if (ColorToInt(players[i].color) != ColorToInt(players[item->player_id - 1].color))
    //         {
    //             players[i].life--;
    //         }
    //         ray_dir.x = contact_point.x;
    //         ray_dir.y = contact_point.y;
    //         if (item->laser.laserStartPosX == item->laser.laserEndPosX && item->laser.laserStartPosY == item->laser.laserEndPosY)
    //         {
    //             item->laser.animationTimerLaser = 0.0f;
    //         }
    //     }
    // }

    // item->laser.laserEndPosX = ray_dir.x;
    // item->laser.laserEndPosY = ray_dir.y;

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

    
    // for bullets 

    //     DrawLine(
    //         item->laser.laserStartPosX,
    //         item->laser.laserStartPosY,
    //         item->laser.laserEndPosX,
    //         item->laser.laserEndPosY,
    //         player.color);

}
