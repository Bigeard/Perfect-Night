#include "../../../lib/raylib/src/raylib.h"
#include <string.h>

#include "bullet.h"
#include "../gameplay/gameplay.h"
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
    else {
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

void DrawBullet(Bullet bullet)
{
    float centerBulletX = bullet.p.pos.x + bullet.p.size.x;
    float centerBulletY = bullet.p.pos.y + bullet.p.size.y;

    if (!bullet.inactive)
    {
        DrawCircle(centerBulletX, centerBulletY, bullet.p.size.x * 1.46f, BLACK);
        DrawCircle(centerBulletX, centerBulletY, bullet.p.size.x * 1.3f, WHITE);
        DrawCircle(centerBulletX, centerBulletY, bullet.p.size.x, DarkenColor(bullet.COLOR, 0.9f));
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

void BulletValueToData(Bullet bullet, char *dataToSend)
{
    strcat(dataToSend, TextFormat("1,%f,%f,", bullet.p.pos.x, bullet.p.pos.y));
}