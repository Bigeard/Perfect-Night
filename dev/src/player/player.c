#include "../../../raylib/src/raylib.h"
#include "../../../raylib/src/raymath.h"
#include <emscripten/emscripten.h>
#include "stdio.h"

#include "player.h"
#include "../bullet/bullet.h"

EM_JS(float, GetJoystickMobileLeftX, (), { return virtualGamepad.Gamepad.axes[0] });
EM_JS(float, GetJoystickMobileLeftY, (), { return virtualGamepad.Gamepad.axes[1] });
EM_JS(float, GetJoystickMobileRightX, (), { return virtualGamepad.Gamepad.axes[2] });
EM_JS(float, GetJoystickMobileRightY, (), { return virtualGamepad.Gamepad.axes[3] });


void InitPlayer(void) {

}

void UpdatePlayer(Player *player) {
    float delta = GetFrameTime();

    if (player->life <= 0) {
        // @TODO Take position of winner
        return;
    }

    if (player->invincible > 1) {
        player->invincible = player->invincible - delta;
    } else {
        player->invincible = 0;
    }

    if (player->ammunition != 5) {
        player->ammunitionLoad = player->ammunitionLoad - delta;
    }
    if (player->ammunitionLoad <= 0) {
        player->ammunition++;
        player->ammunitionLoad = 300;
    }

    player->p.collision[0] = false;
    player->p.collision[1] = false;
    player->p.collision[2] = false;
    player->p.collision[3] = false;
    player->p.collision[4] = false;

    float delta_x = player->p.vel.x;
    float delta_y = player->p.vel.y;

    if (player->INPUT_TYPE == MOUSE || player->INPUT_TYPE == KEYBOARD) {
        // Move Player
        if (IsKeyDown(player->KEY[0])) delta_y -= player->speed.y; // Up
        if (IsKeyDown(player->KEY[1])) delta_y += player->speed.y; // Down
        if (IsKeyDown(player->KEY[2])) delta_x -= player->speed.x; // Left
        if (IsKeyDown(player->KEY[3])) delta_x += player->speed.x; // Right

        // Charge / Shoot Bullet
        if ((IsGestureDetected(GESTURE_DRAG) || IsKeyDown(player->KEY[4])) && player->ammunition > 0) { 
            player->timeShoot += 2;
            player->speed = (Vector2){ 1.5, 1.5 };
            if (player->charge < 15 && ((int)(player->timeShoot * delta * 10)%2) == 1 && player->life > 0) {
                player->charge = player->charge + 0.5;
                player->timeShoot = 0;
            }
        }
        else {
            if (player->charge > 2) {
                // Loop bullet (Allow the ball to be replaced one after the other)
                player->lastBullet += 1;
                if (player->lastBullet >= sizeof(player->bullets)/sizeof(player->bullets[0])) {
                    player->lastBullet = 0;
                }

                float calcPosRadianX = cos(player->radian);
                float calcPosRadianY = sin(player->radian);

                if ((calcPosRadianX > 0.6 && calcPosRadianX < 0.8) ||
                    (calcPosRadianX < -0.6 && calcPosRadianX > -0.8)) {
                    calcPosRadianX = lround(cos(player->radian));
                }
                if ((calcPosRadianY > 0.6 && calcPosRadianY < 0.8) ||
                    (calcPosRadianY < -0.6 && calcPosRadianY > -0.8)) {
                    calcPosRadianY = lround(sin(player->radian));
                }

                player->bullets[player->lastBullet] = (Bullet) { 
                    player->id,
                    (Physic) {
                        {player->p.pos.x + 20 + calcPosRadianX * 22 - 5, player->p.pos.y + 20 + calcPosRadianY * 22 - 5},
                        {5, 5},
                        {0, 0},
                        {0, 0, 0, 0, 0}
                    }, 
                    { player->charge, player->charge }, 
                    player->radian,
                    false,
                    false,
                    true,
                    player->COLORS[0]
                };
                // Remove ammunition
                player->ammunition--;
                player->canShoot = false;
                player->speed = (Vector2){ 3.5, 3.5 };
            }
            player->charge = 2;
        }
        // Rotate Cannon
        if (IsKeyDown(player->KEY[5])) player->radian -= 3.2 * delta;
        if (IsKeyDown(player->KEY[6])) player->radian += 3.2 * delta;
    }

    if (player->INPUT_TYPE == GAMEPAD) {
        // Move Player
        delta_x = player->speed.x * GetGamepadAxisMovement(player->gamepadId, 0);
        delta_y = player->speed.y * GetGamepadAxisMovement(player->gamepadId, 1);

        // Move Cannon
        if (GetGamepadAxisMovement(player->gamepadId, 3) != 0.0f || GetGamepadAxisMovement(player->gamepadId, 2) != 0.0f) {
            player->radian = atan2f(GetGamepadAxisMovement(player->gamepadId, 3), GetGamepadAxisMovement(player->gamepadId, 2));
        }

        // Charge / Shoot Bullet
        if (IsGamepadButtonDown(player->gamepadId, 12) && player->ammunition > 0) {
            player->timeShoot += 2;
            player->speed = (Vector2){ 1.5, 1.5 };
            if (player->charge < 15 && ((int)(player->timeShoot * delta * 10)%2) == 1 && player->life > 0) {
                player->charge = player->charge + 0.5;
                player->timeShoot = 0;
            }
        }
        else {
            if (player->charge > 2) {
                // Loop bullet (Allow the ball to be replaced one after the other)
                player->lastBullet += 1;
                if (player->lastBullet >= sizeof(player->bullets)/sizeof(player->bullets[0])) {
                    player->lastBullet = 0;
                }

                float calcPosRadianX = cos(player->radian);
                float calcPosRadianY = sin(player->radian);

                if ((calcPosRadianX > 0.6 && calcPosRadianX < 0.8) ||
                    (calcPosRadianX < -0.6 && calcPosRadianX > -0.8)) {
                    calcPosRadianX = lround(cos(player->radian));
                }
                if ((calcPosRadianY > 0.6 && calcPosRadianY < 0.8) ||
                    (calcPosRadianY < -0.6 && calcPosRadianY > -0.8)) {
                    calcPosRadianY = lround(sin(player->radian));
                }

                player->bullets[player->lastBullet] = (Bullet) { 
                    player->id,
                    (Physic) {
                        {player->p.pos.x + 20 + calcPosRadianX * 22 - 5, player->p.pos.y + 20 + calcPosRadianY * 22 - 5},
                        {5, 5},
                        {0, 0},
                        {0, 0, 0, 0, 0}
                    }, 
                    { player->charge, player->charge }, 
                    player->radian,
                    false,
                    false,
                    true,
                    player->COLORS[0]
                };
                // player->bullets[player->lastBullet] = (Bullet) { 
                //     player->id,
                //     (Physic) {
                //         {player->p.pos.x + 20 - 5, player->p.pos.y + 20 - 5},
                //         {5, 5},
                //         {0, 0},
                //         {0, 0, 0, 0, 0}
                //     }, 
                //     { player->charge, player->charge }, 
                //     player->radian,
                //     false,
                //     player->COLORS[0]
                // };

                // Remove ammunition
                player->ammunition--;
                player->canShoot = false;
                player->speed = (Vector2){ 3.5, 3.5 };
            }
            player->charge = 2;
        }
    }

    if (player->INPUT_TYPE == MOBILE) {
        // Move Player
        delta_x = player->speed.x * GetJoystickMobileLeftX();
        delta_y = player->speed.y * GetJoystickMobileLeftY();

        // Move Cannon
        if (GetJoystickMobileRightY() != 0.0f || GetJoystickMobileRightX() != 0.0f) {
            player->radian = atan2f(GetJoystickMobileRightY(), GetJoystickMobileRightX());
        }

        // Charge / Shoot Bullet
        float distance = sqrt(pow(GetJoystickMobileRightX(), 2) + pow(GetJoystickMobileRightY(), 2));
        if (distance > 0.9 && player->ammunition > 0) {
            player->timeShoot += 2;
            player->speed = (Vector2){ 1.5, 1.5 };
            if (player->charge < 15 && ((int)(player->timeShoot * delta * 10)%2) == 1 && player->life > 0) {
                player->charge = player->charge + 0.5;
                player->timeShoot = 0;
            }
        }
        else {
            if (player->charge > 2) {
                // Loop bullet (Allow the ball to be replaced one after the other)
                player->lastBullet += 1;
                if (player->lastBullet >= sizeof(player->bullets)/sizeof(player->bullets[0])) {
                    player->lastBullet = 0;
                }

                float calcPosRadianX = cos(player->radian);
                float calcPosRadianY = sin(player->radian);

                if ((calcPosRadianX > 0.6 && calcPosRadianX < 0.8) ||
                    (calcPosRadianX < -0.6 && calcPosRadianX > -0.8)) {
                    calcPosRadianX = lround(cos(player->radian));
                }
                if ((calcPosRadianY > 0.6 && calcPosRadianY < 0.8) ||
                    (calcPosRadianY < -0.6 && calcPosRadianY > -0.8)) {
                    calcPosRadianY = lround(sin(player->radian));
                }

                player->bullets[player->lastBullet] = (Bullet) { 
                    player->id,
                    (Physic) {
                        {player->p.pos.x + 20 + calcPosRadianX * 22 - 5, player->p.pos.y + 20 + calcPosRadianY * 22 - 5},
                        {5, 5},
                        {0, 0},
                        {0, 0, 0, 0, 0}
                    }, 
                    { player->charge, player->charge }, 
                    player->radian,
                    false,
                    false,
                    true,
                    player->COLORS[0]
                };

                // Remove ammunition
                player->ammunition--;
                player->canShoot = false;
                player->speed = (Vector2){ 3.5, 3.5 };
            }
            player->charge = 2;
        }
    }

    player->p.pos.x = player->p.pos.x + delta_x;
    player->p.pos.y = player->p.pos.y + delta_y;

    // Out of area
    float arenaSize = 800;
    if ((player->p.pos.x >= (float) arenaSize || 
        player->p.pos.x + player->p.size.x <= 0) ||
        (player->p.pos.y >= (float) arenaSize || 
        player->p.pos.y + player->p.size.y <= 0)) {

        if (player->life > 0) player->life--;
        player->p.pos = (Vector2) { arenaSize / 2 - player->p.size.x / 2, arenaSize / 2 - player->p.size.y / 2 };
        player->p.vel = (Vector2) { 0, 0 };
        player->speed = (Vector2) { 3.5, 3.5 };
        player->damagesTaken = 0;
        player->invincible = 300;
    }
}

void CollisionBulletPlayer(Bullet *bullet, Player *player, Rectangle recPlayer) {
    if (bullet->inactive) return;
    if (player->life <= 0) return;
    if (bullet->playerId != player->id) {
        bool collision = CheckCollisionCircleRec((Vector2){bullet->p.pos.x + 4, bullet->p.pos.y + 4}, bullet->p.size.x-1, recPlayer);
        if (collision && player->invincible == 0) {
            player->life--;
            player->invincible = 300;
        }
    }
}

void DrawPlayer(Player player) {
    if (player.life <= 0) return;

    if (player.invincible != 0) {
        DrawRectangleRounded((Rectangle) { player.p.pos.x, player.p.pos.y, player.p.size.x, player.p.size.y }, 0.3, 1, Fade(player.COLORS[2], 0.5));
    }
    else {
        // Draw body of the tank
        DrawRectangleRounded((Rectangle) { player.p.pos.x, player.p.pos.y, player.p.size.x, player.p.size.y }, 0.3, 1, player.COLORS[2]);
    }

    // Draw cannon of the tank
    Rectangle playerCanon = { player.p.pos.x + 20, player.p.pos.y + 20, 30, 10 };
    Vector2 originCanon = { 0, 5 };
    DrawRectanglePro((Rectangle){ player.p.pos.x + 20, player.p.pos.y + 20, 32, 12 }, (Vector2){ 0, 6 }, player.radian * (180 / PI) , WHITE);
    DrawCircle(player.p.pos.x + 20, player.p.pos.y + 20, 14, WHITE);
    DrawRectanglePro(playerCanon, originCanon, player.radian * (180 / PI), player.COLORS[1]);
    DrawCircle(player.p.pos.x + 20, player.p.pos.y + 20, 13, player.COLORS[1]);

    // Draw life of the tank
    float posX = player.p.pos.x - 5 + 20;
    if (player.life == 1) {
        posX = player.p.pos.x - 2 + 20;
    }
    DrawText(TextFormat("%d", player.life), posX, player.p.pos.y - 28.5 + 40, 20, WHITE);
    // DrawText(TextFormat("%f %f", cos(player.radian), sin(+ player.radian)), player.p.pos.x + 40, player.p.pos.y + 40, 10, BLACK);
    // DrawText(TextFormat("%f %f", , , player.p.pos.x + 40, player.p.pos.y + 40, 10, BLACK);

    // Draw load of the shoot
    DrawRectangleRec((Rectangle){ player.p.pos.x - 17 + 20, player.p.pos.y - 50 + 40, (player.charge - 2) * 2.6, 4 }, Fade(player.COLORS[1], 0.4));
}

void DrawStatsPlayer(Player player) {
    Color colorDisplay0 = player.COLORS[0];
    Color colorDisplay1 = player.COLORS[1];
    if (player.life < 1) {
        colorDisplay0 = Fade(player.COLORS[0], 0.2);
        colorDisplay1 = Fade(player.COLORS[1], 0.2);
    }
    if (player.id % 2) {
        DrawRectanglePro((Rectangle){ 300, 80 + 120 * player.id, 64, 72 }, (Vector2){ 66, 6 }, -270, WHITE);
        DrawRectanglePro((Rectangle){ 300, 80 + 120 * player.id, 210, 110 }, (Vector2){ 205, 5 }, 20, WHITE);
        DrawRectanglePro((Rectangle){ 300, 80 + 120 * player.id, 210, 110 }, (Vector2){ 205, 5 }, 25, WHITE);

        DrawRectanglePro((Rectangle){ 300, 80 + 120 * player.id, 60, 60 }, (Vector2){ 60, 0 }, -270, colorDisplay1);
        DrawRectanglePro((Rectangle){ 300, 80 + 120 * player.id, 200, 100 }, (Vector2){ 200, 0 }, 20, colorDisplay1);
        DrawRectanglePro((Rectangle){ 300, 80 + 120 * player.id, 200, 100 }, (Vector2){ 200, 0 }, 25, colorDisplay1);

        if (player.id == 1) DrawText(TextFormat("P%d", player.id), 256, 30 + 120 * player.id, 30, WHITE);
        else DrawText(TextFormat("P%d", player.id), 253, 30 + 120 * player.id, 30, WHITE);
        
        DrawRectanglePro((Rectangle){ 300, 80 + 120 * player.id, 190, 90 }, (Vector2){ 195, -5 }, 20, WHITE);

        DrawTextPro(GetFontDefault(), TextFormat("%d", player.ammunition), (Vector2){ 180, 185 + 120 * (player.id - 1) }, (Vector2){ 40, 0 }, 20, 60, 10, colorDisplay0);
        DrawTextPro(GetFontDefault(), TextFormat("%d", player.ammunitionLoad), (Vector2){ 250, 190 + 120 * (player.id - 1) }, (Vector2){ 40, 0 }, 20, 40, 10, colorDisplay0);
    }
    else {
        DrawRectanglePro((Rectangle){ GetScreenWidth() - 300, 80 + 120 * (player.id - 1), 64, 72 }, (Vector2){ -2, 6 }, 270, WHITE);
        DrawRectanglePro((Rectangle){ GetScreenWidth() - 300, 80 + 120 * (player.id - 1), 210, 110 }, (Vector2){ 5, 5 }, -20, WHITE);
        DrawRectanglePro((Rectangle){ GetScreenWidth() - 300, 80 + 120 * (player.id - 1), 210, 110 }, (Vector2){ 5, 5 }, -25, WHITE);

        DrawRectanglePro((Rectangle){ GetScreenWidth() - 300, 80 + 120 * (player.id - 1), 60, 60 }, (Vector2){ 0, 0 }, 270, colorDisplay1);
        DrawRectanglePro((Rectangle){ GetScreenWidth() - 300, 80 + 120 * (player.id - 1), 200, 100 }, (Vector2){ 0, 0 }, -20, colorDisplay1);
        DrawRectanglePro((Rectangle){ GetScreenWidth() - 300, 80 + 120 * (player.id - 1), 200, 100 }, (Vector2){ 0, 0 }, -25, colorDisplay1);

        DrawText(TextFormat("P%d", player.id), GetScreenWidth() - 288, 30 + 120 * (player.id - 1), 30, WHITE);
        
        DrawRectanglePro((Rectangle){ GetScreenWidth() - 300, 80 + 120 * (player.id - 1), 190, 90 }, (Vector2){ -5, -5 }, -20, WHITE);

        DrawTextPro(GetFontDefault(), TextFormat("%d", player.ammunition), (Vector2){ GetScreenWidth() - 130, 45 + 120 * (player.id - 1) }, (Vector2){ 40, 0 }, -20, 60, 10, colorDisplay0);
        DrawTextPro(GetFontDefault(), TextFormat("%d", player.ammunitionLoad), (Vector2){ GetScreenWidth() - 250, 70 + 120 * (player.id - 1) }, (Vector2){ 40, 0 }, -20, 40, 10, colorDisplay0);
    }
}