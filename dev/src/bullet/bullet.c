#include "../../../lib/raylib/src/raylib.h"
#include "../../../lib/raylib/src/raymath.h"

#include "bullet.h"
#include <stdbool.h>


void InitBullet(void) {

}

void UpdateBullet(Bullet *bullet) {
    if (bullet->inactive) return;
    float delta = GetFrameTime();

    bullet->p.collision[0] = false;
    bullet->p.collision[1] = false;
    bullet->p.collision[2] = false;
    bullet->p.collision[3] = false;
    bullet->p.collision[4] = false;

    bullet->p.pos.x = bullet->p.pos.x + cos(bullet->radian) * (bullet->speed.x * delta * 100);
    bullet->p.pos.y = bullet->p.pos.y + sin(bullet->radian) * (bullet->speed.y * delta * 100);

    if (bullet->speed.x < 1 && bullet->speed.y < 1 && bullet->speed.x > -1 && bullet->speed.y > -1) {
        bullet->inactive = true;
    }
}

void BulletBounce(Bullet *bullet) {
    if(bullet->p.collision[0]) {
        if(bullet->p.collision[1]) bullet->speed.y *= -1;
        else if(bullet->p.collision[2]) bullet->speed.y *= -1;
        else if(bullet->p.collision[3]) bullet->speed.x *= -1;
        else if(bullet->p.collision[4]) bullet->speed.x *= -1;
        bullet->speed.x *= 0.8;
        bullet->speed.y *= 0.8;
    }
}

void DrawBullet(Bullet bullet) {
    if (bullet.inactive) {
        DrawCircleV((Vector2){bullet.p.pos.x + 5, bullet.p.pos.y + 5}, bullet.p.size.x, Fade(bullet.COLOR, 0.2)); 
    }
    else {
        DrawCircleV((Vector2){bullet.p.pos.x + 5, bullet.p.pos.y + 5}, bullet.p.size.x, bullet.COLOR); 
    }
    // DrawText(TextFormat("X %f/ Y %f", bullet.speed.x, bullet.speed.y), bullet.p.pos.x - 30, bullet.p.pos.y - 14, 12, bullet.COLOR);
    // Rectangle bulletBox = {bullet.p.pos.x, bullet.p.pos.y, bullet.p.size.x * 2, bullet.p.size.y * 2};
    // DrawRectangleRec(bulletBox, Fade(BLUE, 0.4));
    // DrawRectangleRec((Rectangle){bullet.p.pos.x, bullet.p.pos.y, bullet.p.size.x, bullet.p.size.y}, Fade(PURPLE, 0.8));
}
