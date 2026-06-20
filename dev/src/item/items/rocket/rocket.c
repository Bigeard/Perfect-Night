#include "../../../../../lib/raylib/src/raylib.h"
#include "../../../../../lib/raylib/src/raymath.h"

#include "./rocket.h"
#include "../../item.h"
#include "../../../gameplay/gameplay.h"
#include "../../../player/player.h"
#include "../../../tool/tool.h"

Item InitItemRocket(int player_id, float maxTimer)
{
    float defaultMaxTimer = 10.0f;
    if (maxTimer > -1.0f)
        defaultMaxTimer = maxTimer;

    return (Item){
        .player_id = player_id,
        .type = ROCKET,
        .active = true,
        .timer = GetTime(),
        .maxTimer = defaultMaxTimer,
        .defaultShoot = false,
        .defaultDisplay = false,
        .ShootItem = ShootItemRocket,
        .UpdateItem = UpdateItemRocket,
        .DrawItem = DrawItemRocket,
        .rocket = {.sizeExplosion = 140}};
}

void ShootItemRocket(Item *item, float calcPosRadianX, float calcPosRadianY, float delta_x, float delta_y)
{
    if (!item->active)
        return;

    Player *player = &players[item->player_id - 1];
    player->bullets[player->lastBullet] = (Bullet){
        .playerId = player->id,
        .p = (Physic){
            {player->p.pos.x + 20.0f + calcPosRadianX*24.0f - 7.0f,
             player->p.pos.y + 20.0f + calcPosRadianY*24.0f - 7.0f},
            {7.0f, 7.0f},
            {cosf(player->lastRadian)*player->charge + delta_x*2.0f,
             sinf(player->lastRadian)*player->charge + delta_y*2.0f},
            {false, false, false, false, false}},
        .speed = {player->charge + delta_x*2.0f, player->charge + delta_y*2.0f},
        .radian = player->lastRadian,
        .isNew = true,
        .startPos = {player->p.pos.x + player->p.size.x/2.0f,
                     player->p.pos.y + player->p.size.y/2.0f},
        .COLOR = player->color,
        .explosionRadius = (float)item->rocket.sizeExplosion};
}

void UpdateItemRocket(Item *item)
{
    double elapsedTime = GetTime() - item->timer;
    if (item->maxTimer < elapsedTime)
    {
        item->active = false;
    }
}

void DrawItemRocket(Item *item)
{
    if (!item->active)
        return;
    Player player = players[item->player_id - 1];

    // Init Color
    Color color = player.color;
    Color whiteColor = WHITE;
    // Color blackColor = BLACK;

    if (player.invincible)
    {
        // Lighten color
        color = LightenColor(color, 0.5);
        whiteColor = Fade(WHITE, 0.5);
    }

    // Draw the oversized explosive launcher.
    DrawRectanglePro((Rectangle){player.p.pos.x + 20.0f, player.p.pos.y + 20.0f, 42.0f, 20.0f}, (Vector2){0.0f, 10.0f}, player.radian*(180.0f/PI), BLACK);

    // Draw Border of the Tank
    DrawTextureEx(playerBorderTexture, (Vector2){player.p.pos.x, player.p.pos.y}, 0, 1, whiteColor);

    DrawRectanglePro((Rectangle){player.p.pos.x + 20.0f, player.p.pos.y + 20.0f, 39.0f, 16.0f}, (Vector2){0.0f, 8.0f}, player.radian*(180.0f/PI), WHITE);
    DrawRectanglePro((Rectangle){player.p.pos.x + 20.0f, player.p.pos.y + 20.0f, 34.0f, 10.0f}, (Vector2){0.0f, 5.0f}, player.radian*(180.0f/PI), color);

    // Draw Body / Template of the tank
    DrawTextureEx(playerBodyTexture, (Vector2){player.p.pos.x + 3, player.p.pos.y + 3}, 0, 1, color);
    DrawTexturePro(playerTemplatesTextures[player.id], (Rectangle){0, 0, 32, 32}, (Rectangle){player.p.pos.x + 20, player.p.pos.y + 20, 32, 32}, (Vector2){16, 16}, player.radian * (180 / PI) + 90, WHITE);

    const float elapsedTime = (float)(GetTime() - item->timer);
    if (elapsedTime <= 4.0f)
    {
        const Vector2 iconCenter = {player.p.pos.x + player.p.size.x/2.0f, player.p.pos.y - 20.0f};
        const float alpha = 1.0f - elapsedTime/4.0f;
        DrawCircle(iconCenter.x, iconCenter.y, 14.0f, Fade(BLACK, alpha));
        DrawCircle(iconCenter.x, iconCenter.y, 11.0f, Fade(WHITE, alpha));
        DrawCircle(iconCenter.x, iconCenter.y, 8.0f, Fade(player.color, alpha));
    }

    for (int a = 0; a < player.ammunition; a++)
    {
        const float calcRadian = player.radian + PI + ((float)a*0.7f + 0.35f - ((float)player.ammunition*0.35f));
        const float ammunitionPosX = (player.p.pos.x + player.p.size.x/2.0f) + 25.0f*cosf(calcRadian);
        const float ammunitionPosY = (player.p.pos.y + player.p.size.x/2.0f) + 25.0f*sinf(calcRadian);
        DrawCircle(ammunitionPosX, ammunitionPosY, 7.3f, BLACK);
        DrawCircle(ammunitionPosX, ammunitionPosY, 6.5f, WHITE);
        DrawCircle(ammunitionPosX, ammunitionPosY, 3.0f, DarkenColor(player.color, 0.7f));
    }

    // Use the same circular charge display as the standard bouncing bullet.
    if (player.charge != 2.0f)
    {
        const float chargeAngle = player.radian*(180.0f/PI) + 180.0f;
        const float chargeArc = (player.charge - 2.0f)*4.0f;
        const Vector2 center = {player.p.pos.x + player.p.size.x/2.0f, player.p.pos.y + player.p.size.y/2.0f};

        DrawRing(center, 38.0f, 47.0f, chargeAngle - chargeArc - 2.0f, chargeAngle + chargeArc + 2.0f, 0, Fade(WHITE, 0.6f));
        DrawRing(center, 40.0f, 45.0f, chargeAngle - chargeArc, chargeAngle + chargeArc, 0, Fade(player.color, 0.8f));
    }
}
