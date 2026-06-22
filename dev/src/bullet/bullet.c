#include "../../../lib/raylib/src/raylib.h"
#include "../../../lib/raylib/src/raymath.h"
#include <stdio.h>
#include <string.h>

#include "bullet.h"
#include "../gameplay/gameplay.h"
#include "../player/player.h"
#include "../particle/particle.h"
#include "../tool/tool.h"

void InitBullet(void) {}

void UpdateBullet(Bullet *bullet)
{
    if (bullet->inactive)
        return;

    bullet->p.collision[0] = false;
    bullet->p.collision[1] = false;
    bullet->p.collision[2] = false;
    bullet->p.collision[3] = false;
    bullet->p.collision[4] = false;

    bullet->p.pos.x += bullet->p.vel.x;
    bullet->p.pos.y += bullet->p.vel.y;

    if (activePerf)
    {
        bullet->trailCount = 0;
    }
    else if (bullet->distanceLaser <= 0.0f)
    {
        bullet->trail[bullet->trailIndex] = (Vector2){
            bullet->p.pos.x + bullet->p.size.x,
            bullet->p.pos.y + bullet->p.size.y};
        bullet->trailIndex = (bullet->trailIndex + 1)%BULLET_TRAIL_LENGTH;
        if (bullet->trailCount < BULLET_TRAIL_LENGTH)
            bullet->trailCount++;
    }

    if (bullet->p.vel.x < 1.0f && bullet->p.vel.y < 1.0f &&
        bullet->p.vel.x > -1.0f && bullet->p.vel.y > -1.0f)
    {
        bullet->inactive = true;
    }
    if ((bullet->p.pos.x >= arenaSizeX ||
         bullet->p.pos.x + bullet->p.size.x <= 0.0f) ||
        (bullet->p.pos.y >= arenaSizeY ||
         bullet->p.pos.y + bullet->p.size.y <= 0.0f))
    {
        bullet->timerInactive++;
        if (bullet->timerInactive > 100)
        {
            bullet->inactive = true;
        }
    }
    else
    {
        bullet->timerInactive = 0;
    }
}

void BulletBounce(Bullet *bullet)
{
    if (bullet->inactive)
        return;
    if (bullet->p.collision[0])
    {
        if (bullet->p.collision[1]) // Top
            bullet->p.vel.y *= -1.0f;
        else if (bullet->p.collision[2])
        { // Bottom
            bullet->p.vel.y *= -1.0f;
        }
        if (bullet->p.collision[3])
            bullet->p.vel.x *= -1.0f; // Left
        else if (bullet->p.collision[4])
            bullet->p.vel.x *= -1.0f; // Right
    }
}

void ExplodeBullet(Bullet *bullet)
{
    if (bullet->explosionRadius <= 0.0f || bullet->explosionTime > 0.0)
        return;

    bullet->inactive = true;
    bullet->p.vel = (Vector2){0.0f, 0.0f};
    bullet->explosionTime = GetTime();
    bullet->explosionPosition = (Vector2){
        bullet->p.pos.x + bullet->p.size.x,
        bullet->p.pos.y + bullet->p.size.y};

    for (int i = 0; i < NUMBER_EIGHT; i++)
    {
        Player *player = &players[i];
        if (!player->id || player->life <= 0 || player->invincible != 0)
            continue;
        if (ColorToInt(player->color) == ColorToInt(bullet->COLOR))
            continue;

        const Vector2 playerCenter = {
            player->p.pos.x + player->p.size.x/2.0f,
            player->p.pos.y + player->p.size.y/2.0f};
        if (Vector2Distance(playerCenter, bullet->explosionPosition) > bullet->explosionRadius)
            continue;

        player->life--;
        player->invincible = DELAY_INVINCIBLE;
        GamepadPlayerLife(player->gamepadId, player->life);

        Vector2 blastDirection = Vector2Subtract(playerCenter, bullet->explosionPosition);
        if (Vector2LengthSqr(blastDirection) > 0.0f)
            blastDirection = Vector2Scale(Vector2Normalize(blastDirection), 8.0f);
        InitParticles(playerCenter, blastDirection, 0.1f, player->color, 120.0f, player->shootParticle, 20);
    }
}

void DrawBullet(Bullet bullet)
{
    float centerBulletX = bullet.p.pos.x + bullet.p.size.x;
    float centerBulletY = bullet.p.pos.y + bullet.p.size.y;

    if (!activePerf && !bullet.inactive && bullet.distanceLaser <= 0.0f && bullet.trailCount > 1)
    {
        const int first = (bullet.trailIndex - bullet.trailCount + BULLET_TRAIL_LENGTH)%BULLET_TRAIL_LENGTH;
        Vector2 previous = bullet.trail[first];
        for (int i = 1; i < bullet.trailCount; i++)
        {
            const Vector2 current = bullet.trail[(first + i)%BULLET_TRAIL_LENGTH];
            const float strength = (float)(i + 1)/(float)bullet.trailCount;
            DrawLineEx(
                previous,
                current,
                fmaxf(1.0f, bullet.p.size.x*0.85f*strength),
                Fade(DarkenColor(bullet.COLOR, 0.65f), 0.5f*strength));
            previous = current;
        }
    }

    if (!bullet.inactive)
    {
        DrawCircle(centerBulletX, centerBulletY, bullet.p.size.x * 1.46f, BLACK);
        DrawCircle(centerBulletX, centerBulletY, bullet.p.size.x * 1.3f, WHITE);
        DrawCircle(centerBulletX, centerBulletY, bullet.p.size.x, DarkenColor(bullet.COLOR, 0.9f));
    }

    if (bullet.explosionTime > 0.0)
    {
        const float elapsed = (float)(GetTime() - bullet.explosionTime);
        const float duration = 0.4f;
        if (elapsed < duration)
        {
            const float progress = elapsed/duration;
            const float radius = bullet.explosionRadius*progress;
            const float alpha = 1.0f - progress;
            DrawCircleGradient(bullet.explosionPosition, radius, Fade(WHITE, alpha*0.7f), Fade(bullet.COLOR, 0.0f));
            DrawRing(bullet.explosionPosition, fmaxf(0.0f, radius - 7.0f), radius, 0.0f, 360.0f, 48, Fade(bullet.COLOR, alpha));
        }
    }

    // *** DEV INFO ***
    if (activeDev)
    {
        DrawText(
            TextFormat("B Vel: X %f/ Y %f", bullet.p.vel.x, bullet.p.vel.y),
            (int)bullet.p.pos.x, (int)(bullet.p.pos.y - 20.0f), 16,
            DarkenColor(bullet.COLOR, 0.9f));
        DrawRectangleRec(
            (Rectangle){
                bullet.p.pos.x,
                bullet.p.pos.y,
                bullet.p.size.x * 2.0f,
                bullet.p.size.y * 2.0f},
            Fade(PURPLE, 0.5f));
    }
}

void BulletValueToData(Bullet bullet, char *dataToSend, size_t dataSize)
{
    if (bullet.playerId && !bullet.inactive)
    {
        const size_t len = strlen(dataToSend);
        if (len < dataSize)
        {
            snprintf(dataToSend + len, dataSize - len, "1,%f,%f,%d,", bullet.p.pos.x, bullet.p.pos.y, bullet.playerId);
        }
    }
}
