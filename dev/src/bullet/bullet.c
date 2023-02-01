#include "../../../lib/raylib/src/raylib.h"
#include "../../../lib/raylib/src/raymath.h"

#include "bullet.h"
#include <stdbool.h>


void InitBullet(void) {

}

void UpdateBullet(Bullet *bullet) {
    if (bullet->inactive) return;
    bullet->p.collision[0] = false;
    bullet->p.collision[1] = false;
    bullet->p.collision[2] = false;
    bullet->p.collision[3] = false;
    bullet->p.collision[4] = false;

    bullet->p.pos.x += bullet->p.vel.x;
    bullet->p.pos.y += bullet->p.vel.y;

    if (bullet->p.vel.x < 1 && bullet->p.vel.y < 1 && bullet->p.vel.x > -1 && bullet->p.vel.y > -1) {
        bullet->inactive = true;
    }
}

void BulletBounce(Bullet *bullet) {
    // if(bullet->p.collision[0]) {
        // if(bullet->p.collision[1]) bullet->p.vel.y *= -1; // Top
        // else if(bullet->p.collision[2]) bullet->p.vel.y *= -1; // Bottom
        // else if(bullet->p.collision[3]) bullet->p.vel.x *= -1; // Left
        // else if(bullet->p.collision[4]) bullet->p.vel.x *= -1; // Right
        // bullet->p.vel.x *= 0.8;
        // bullet->p.vel.y *= 0.8;
    // }
}

void DrawBullet(Bullet bullet) {
    if (bullet.inactive) {
        DrawCircle(bullet.p.pos.x + 5, bullet.p.pos.y + 5, bullet.p.size.x, Fade(bullet.COLOR, 0.2)); 
    }
    else {
        DrawCircle(bullet.p.pos.x + 5, bullet.p.pos.y + 5, 7.3, BLACK);
        DrawCircle(bullet.p.pos.x + 5, bullet.p.pos.y + 5, 6.5, WHITE);
        DrawCircle(bullet.p.pos.x + 5, bullet.p.pos.y + 5, bullet.p.size.x, bullet.COLOR); 
    }
    // DrawText(TextFormat("Bullet Vel: X %f/ Y %f", bullet.p.vel.x, bullet.p.vel.y), bullet.p.pos.x - 30, bullet.p.pos.y - 14, 20, bullet.COLOR);
    // DrawCircleV((Vector2){bullet.p.pos.x + 5, bullet.p.pos.y + 5}, bullet.p.size.x-1, GREEN); 
    // DrawRectangleRec((Rectangle){bullet.p.pos.x + 2.5, bullet.p.pos.y + 2.5, bullet.p.size.x * 2 - 5, bullet.p.size.y  * 2 - 5}, PURPLE);
}
