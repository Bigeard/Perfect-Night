#include "../../../lib/raylib/src/raylib.h"
#include "../../../lib/raylib/src/raymath.h"
#include <stdio.h>
#include <string.h>

#include "bullet.h"
#include "../gameplay/gameplay.h"
#include "../player/player.h"
#include "../particle/particle.h"
#include "../tool/tool.h"

#define MAX_BULLET_IMPACTS 64
#define BULLET_IMPACT_DURATION 0.58f
#define BULLET_IMPACT_LENGTH 80.0f
#define BULLET_IMPACT_SEGMENTS 28
#define BULLET_IMPACT_EXPAND_END 0.38f

typedef struct BulletImpact
{
    Vector2 start;
    Vector2 end;
    Vector2 contact;
    Color color;
    double createdAt;
    bool active;
} BulletImpact;

static BulletImpact bulletImpacts[MAX_BULLET_IMPACTS] = {0};
static int nextBulletImpact = 0;

void InitBullet(void) {}

void AddBulletLineImpact(Vector2 wallStart, Vector2 wallEnd, Vector2 contact, Color color)
{
    Vector2 direction = Vector2Subtract(wallEnd, wallStart);
    const float wallLength = Vector2Length(direction);
    if (wallLength <= 0.0f)
        return;

    direction = Vector2Scale(direction, 1.0f/wallLength);
    const float contactDistance = Clamp(
        Vector2DotProduct(Vector2Subtract(contact, wallStart), direction),
        0.0f,
        wallLength);
    const float halfLength = BULLET_IMPACT_LENGTH/2.0f;
    const float startDistance = Clamp(contactDistance - halfLength, 0.0f, wallLength);
    const float endDistance = Clamp(contactDistance + halfLength, 0.0f, wallLength);

    bulletImpacts[nextBulletImpact] = (BulletImpact){
        .start = Vector2Add(wallStart, Vector2Scale(direction, startDistance)),
        .end = Vector2Add(wallStart, Vector2Scale(direction, endDistance)),
        .contact = Vector2Add(wallStart, Vector2Scale(direction, contactDistance)),
        .color = color,
        .createdAt = GetTime(),
        .active = true};
    nextBulletImpact = (nextBulletImpact + 1)%MAX_BULLET_IMPACTS;
}

void AddBulletBoxImpact(const Bullet *bullet, Rectangle wall)
{
    const Vector2 center = {
        bullet->p.pos.x + bullet->p.size.x,
        bullet->p.pos.y + bullet->p.size.y};

    if (bullet->p.collision[1])
    {
        AddBulletLineImpact(
            (Vector2){wall.x, wall.y},
            (Vector2){wall.x + wall.width, wall.y},
            center,
            bullet->COLOR);
    }
    else if (bullet->p.collision[2])
    {
        AddBulletLineImpact(
            (Vector2){wall.x, wall.y + wall.height},
            (Vector2){wall.x + wall.width, wall.y + wall.height},
            center,
            bullet->COLOR);
    }

    if (bullet->p.collision[3])
    {
        AddBulletLineImpact(
            (Vector2){wall.x, wall.y},
            (Vector2){wall.x, wall.y + wall.height},
            center,
            bullet->COLOR);
    }
    else if (bullet->p.collision[4])
    {
        AddBulletLineImpact(
            (Vector2){wall.x + wall.width, wall.y},
            (Vector2){wall.x + wall.width, wall.y + wall.height},
            center,
            bullet->COLOR);
    }
}

void DrawBulletImpacts(void)
{
    const double now = GetTime();
    for (int i = 0; i < MAX_BULLET_IMPACTS; i++)
    {
        if (!bulletImpacts[i].active)
            continue;

        const float elapsed = (float)(now - bulletImpacts[i].createdAt);
        if (elapsed >= BULLET_IMPACT_DURATION)
        {
            bulletImpacts[i].active = false;
            continue;
        }

        const float progress = elapsed/BULLET_IMPACT_DURATION;
        const Vector2 line = Vector2Subtract(bulletImpacts[i].end, bulletImpacts[i].start);
        const float lineLengthSquared = Vector2LengthSqr(line);
        if (lineLengthSquared <= 0.0f)
            continue;

        const float contactProgress = Clamp(
            Vector2DotProduct(Vector2Subtract(bulletImpacts[i].contact, bulletImpacts[i].start), line)/lineLengthSquared,
            0.0f,
            1.0f);

        for (int segment = 0; segment < BULLET_IMPACT_SEGMENTS; segment++)
        {
            const float t0 = (float)segment/(float)BULLET_IMPACT_SEGMENTS;
            const float t1 = (float)(segment + 1)/(float)BULLET_IMPACT_SEGMENTS;
            const float midpoint = (t0 + t1)/2.0f;
            float signedDistance;
            if (midpoint < contactProgress)
                signedDistance = -(contactProgress - midpoint)/fmaxf(contactProgress, 0.0001f);
            else
                signedDistance = (midpoint - contactProgress)/fmaxf(1.0f - contactProgress, 0.0001f);

            const float distance = fabsf(signedDistance);
            float alpha = 0.0f;
            if (progress < BULLET_IMPACT_EXPAND_END)
            {
                float expansion = progress/BULLET_IMPACT_EXPAND_END;
                expansion = expansion*expansion*(3.0f - 2.0f*expansion);
                if (distance < expansion)
                    alpha = 1.0f - distance/expansion;
            }
            else
            {
                const float dissipate = (progress - BULLET_IMPACT_EXPAND_END)/(1.0f - BULLET_IMPACT_EXPAND_END);
                const float front = dissipate;
                const float width = 0.30f - dissipate*0.18f;
                alpha = fmaxf(0.0f, 1.0f - fabsf(distance - front)/width)*(1.0f - dissipate);
            }

            if (alpha <= 0.0f)
                continue;

            const Vector2 segmentStart = Vector2Add(bulletImpacts[i].start, Vector2Scale(line, t0));
            const Vector2 segmentEnd = Vector2Add(bulletImpacts[i].start, Vector2Scale(line, t1));
            DrawLineEx(segmentStart, segmentEnd, 10.0f, Fade(bulletImpacts[i].color, alpha*0.22f));
            DrawLineEx(segmentStart, segmentEnd, 4.0f, Fade(bulletImpacts[i].color, alpha));
        }
    }
}

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
