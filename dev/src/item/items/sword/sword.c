#include "../../../../../lib/raylib/src/raylib.h"
#include "../../../../../lib/raylib/src/raymath.h"

#include "./sword.h"
#include "../../item.h"
#include "../../../gameplay/gameplay.h"
#include "../../../player/player.h"
#include "../../../tool/tool.h"

#define SWORD_RANGE 80.0f
#define SWORD_HALF_ARC_RADIANS (50.0f * DEG2RAD)
#define SWORD_SWING_DURATION 0.2f

Item InitItemSword(int player_id, float maxTimer)
{
    float defaultMaxTimer = 10.0f;
    if (maxTimer > -1.0f)
        defaultMaxTimer = maxTimer;

    return (Item){
        .player_id = player_id,
        .type = SWORD,
        .active = true,
        .timer = GetTime(),
        .maxTimer = defaultMaxTimer,
        .defaultShoot = false,
        .defaultDisplay = false,
        .ShootItem = ShootItemSword,
        .UpdateItem = UpdateItemSword,
        .DrawItem = DrawItemSword,
        .sword = {.lastSwingTime = -1.0, .swingRadian = 0.0f}};
}

void ShootItemSword(Item *item, float calcPosRadianX, float calcPosRadianY, float delta_x, float delta_y)
{
    if (!item->active)
        return;

    Player *player = &players[item->player_id - 1];
    const Vector2 playerCenter = {
        player->p.pos.x + player->p.size.x / 2.0f,
        player->p.pos.y + player->p.size.y / 2.0f};
    item->sword.lastSwingTime = GetTime();
    item->sword.swingRadian = player->lastRadian;

    for (int i = 0; i < NUMBER_EIGHT; i++)
    {
        Player *target = &players[i];
        if (!target->id || target->id == player->id || target->life <= 0 || target->invincible != 0)
            continue;
        if (ColorToInt(target->color) == ColorToInt(player->color))
            continue;

        const Vector2 targetCenter = {
            target->p.pos.x + target->p.size.x / 2.0f,
            target->p.pos.y + target->p.size.y / 2.0f};
        const Vector2 toTarget = Vector2Subtract(targetCenter, playerCenter);
        if (Vector2LengthSqr(toTarget) > SWORD_RANGE * SWORD_RANGE)
            continue;

        if (Vector2LengthSqr(toTarget) > 0.0f)
        {
            const float targetRadian = atan2f(toTarget.y, toTarget.x);
            const float angleDifference = atan2f(
                sinf(targetRadian - player->lastRadian),
                cosf(targetRadian - player->lastRadian));
            if (fabsf(angleDifference) > SWORD_HALF_ARC_RADIANS)
                continue;
        }

        target->life--;
        target->invincible = DELAY_INVINCIBLE;
        GamepadPlayerLife(target->gamepadId, target->life);

        Vector2 hitDirection = toTarget;
        if (Vector2LengthSqr(hitDirection) > 0.0f)
            hitDirection = Vector2Scale(Vector2Normalize(hitDirection), 6.0f);
        InitParticles(targetCenter, hitDirection, 0.1f, target->color, 120.0f, target->shootParticle, 20);
    }
}

void UpdateItemSword(Item *item)
{
    if (!item->active)
        return;

    double elapsedTime = GetTime() - item->timer;
    if (item->maxTimer < elapsedTime)
    {
        item->active = false;
    }
}

void DrawItemSword(Item *item)
{
    if (!item->active)
        return;

    const Player player = players[item->player_id - 1];

    // Init Color
    Color color = player.color;
    Color whiteColor = WHITE;
    if (player.invincible)
    {
        // Lighten color
        color = LightenColor(color, 0.5);
        whiteColor = Fade(WHITE, 0.5);
    }

    const double swingElapsed = GetTime() - item->sword.lastSwingTime;
    float swordRadian = player.radian;
    if (item->sword.lastSwingTime >= 0.0 && swingElapsed < SWORD_SWING_DURATION)
    {
        const float swingProgress = (float)(swingElapsed / SWORD_SWING_DURATION);
        swordRadian = item->sword.swingRadian + Lerp(-SWORD_HALF_ARC_RADIANS, SWORD_HALF_ARC_RADIANS, swingProgress);
        DrawRing(
            (Vector2){player.p.pos.x + player.p.size.x / 2.0f, player.p.pos.y + player.p.size.y / 2.0f},
            52.0f,
            60.0f,
            item->sword.swingRadian * RAD2DEG - 50.0f,
            item->sword.swingRadian * RAD2DEG - 50.0f + 100.0f * swingProgress,
            24,
            Fade(WHITE, 0.65f * (1.0f - swingProgress)));
    }

    const Vector2 swordDirection = {cosf(swordRadian), sinf(swordRadian)};
    const Vector2 swordNormal = {-swordDirection.y, swordDirection.x};
    const Vector2 playerCenter = {
        player.p.pos.x + player.p.size.x / 2.0f,
        player.p.pos.y + player.p.size.y / 2.0f};
    const Vector2 swordBase = Vector2Add(playerCenter, Vector2Scale(swordDirection, 17.0f));
    const Vector2 swordTip = Vector2Add(playerCenter, Vector2Scale(swordDirection, 68.0f));

    DrawLineEx(swordBase, swordTip, 10.0f, BLACK);
    DrawLineEx(swordBase, swordTip, 7.0f, WHITE);
    DrawLineEx(Vector2Add(swordBase, Vector2Scale(swordDirection, 5.0f)), swordTip, 3.0f, color);
    DrawLineEx(
        Vector2Add(swordBase, Vector2Scale(swordNormal, -9.0f)),
        Vector2Add(swordBase, Vector2Scale(swordNormal, 9.0f)),
        6.0f,
        BLACK);
    DrawCircleV(swordBase, 4.0f, color);

    // Draw Border of the Tank
    DrawTextureEx(playerBorderTexture, (Vector2){player.p.pos.x, player.p.pos.y}, 0, 1, whiteColor);

    // Draw Body / Template of the tank
    DrawTextureEx(playerBodyTexture, (Vector2){player.p.pos.x + 3, player.p.pos.y + 3}, 0, 1, color);
    DrawTexturePro(playerTemplatesTextures[player.id], (Rectangle){0, 0, 32, 32}, (Rectangle){player.p.pos.x + 20, player.p.pos.y + 20, 32, 32}, (Vector2){16, 16}, player.radian * (180 / PI) + 90, WHITE);

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
