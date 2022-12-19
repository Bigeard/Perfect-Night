#include "../../../lib/raylib/src/raylib.h"
#include "../../../lib/raylib/src/raymath.h"
#include <emscripten/emscripten.h>
#include <math.h>
#include "stdio.h"

#include "player.h"
#include "../gameplay/gameplay.h"
#include "../bullet/bullet.h"


EM_JS(float, GetJoystickMobileLeftX, (const char* id), { return listGamepad.get(Module.UTF8ToString(id)).axes[0] });
EM_JS(float, GetJoystickMobileLeftY, (const char* id), { return listGamepad.get(Module.UTF8ToString(id)).axes[1] });
EM_JS(float, GetJoystickMobileRightX, (const char* id), { return listGamepad.get(Module.UTF8ToString(id)).axes[2] });
EM_JS(float, GetJoystickMobileRightY, (const char* id), { return listGamepad.get(Module.UTF8ToString(id)).axes[3] });


Texture2D playerBodyTexture;
Texture2D playerFaceTexture;
Texture2D playerTemplateTexture;


void InitPlayer(void) {
    Image playerBodyImage = LoadImage("resources/player-body.png");
    playerBodyTexture = LoadTextureFromImage(playerBodyImage);

    Image playerFaceImage = LoadImage("resources/player-face.png");
    playerFaceTexture = LoadTextureFromImage(playerFaceImage);

    Image playerFaceTemplate = LoadImage("resources/player-template.png");
    playerTemplateTexture = LoadTextureFromImage(playerFaceTemplate);
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

    else if (player->INPUT_TYPE == GAMEPAD) {
        int gamepadId;
        sscanf(player->gamepadId, "%d", &gamepadId);
        // Move Player
        float joystickLeftRectX = GetGamepadAxisMovement(gamepadId, 0);
        float joystickLeftRectY = GetGamepadAxisMovement(gamepadId, 1);
        float joystickLeftX = joystickLeftRectX / sqrt(1 + pow(joystickLeftRectX, 2) * pow(joystickLeftRectY, 2));
        float joystickLeftY = joystickLeftRectY / sqrt(1 + pow(joystickLeftRectX, 2) * pow(joystickLeftRectY, 2));

        // delta_x = player->speed.x * GetGamepadAxisMovement(gamepadId, 0);
        // delta_y = player->speed.y * GetGamepadAxisMovement(gamepadId, 1);
        delta_x = player->speed.x * joystickLeftX;
        delta_y = player->speed.y * joystickLeftY;

        // Move Cannon
        float joystickRightRectX = GetGamepadAxisMovement(gamepadId, 2);
        float joystickRightRectY = GetGamepadAxisMovement(gamepadId, 3);
        
        float joystickRightX = joystickRightRectX / sqrt(1 + pow(joystickRightRectX, 2) * pow(joystickRightRectY, 2));
        float joystickRightY = joystickRightRectY / sqrt(1 + pow(joystickRightRectX, 2) * pow(joystickRightRectY, 2));
        
        // float joystickRightX = joystickRightRectX * sqrt(pow(pow(joystickRightRectX, 2), pow(joystickRightRectY, 2)));
        // float joystickRightY = joystickRightRectY * sqrt(pow(pow(joystickRightRectX, 2), pow(joystickRightRectY, 2)));
        

        if (joystickRightY != 0.0f || joystickRightY != 0.0f) {
            player->radian = atan2f(joystickRightY, joystickRightX);
        }

        // Charge / Shoot Bullet
        // float distance = sqrt(pow(joystickRightX, 2) + pow(joystickRightY, 2));
        // TraceLog(LOG_INFO, "Distance: %f", distance);
        // if (distance >= 1.0f && player->ammunition > 0) {
        if (IsGamepadButtonDown(gamepadId, 12) && player->ammunition > 0) {
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

    else if (player->INPUT_TYPE == MOBILE) {
        // Move Player
        delta_x = player->speed.x * GetJoystickMobileLeftX(player->gamepadId);
        delta_y = player->speed.y * GetJoystickMobileLeftY(player->gamepadId);

        // Move Cannon
        float joystickRightX = GetJoystickMobileRightX(player->gamepadId);
        float joystickRightY = GetJoystickMobileRightY(player->gamepadId);
        if (joystickRightY != 0.0f || joystickRightX != 0.0f) {
            player->radian = atan2f(joystickRightY, joystickRightX);
        }

        // Charge / Shoot Bullet
        float distance = sqrt(pow(joystickRightY, 2) + pow(joystickRightX, 2));
        if (distance > 0.1 && player->ammunition > 0) {
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
    // float arenaSize = 800;
    if ((player->p.pos.x >= arenaSize || 
        player->p.pos.x + player->p.size.x <= 0) ||
        (player->p.pos.y >= arenaSize || 
        player->p.pos.y + player->p.size.y <= 0)) {

        if (player->life > 0) player->life--;
        player->p.pos = (Vector2) { player->spawn.x, player->spawn.y };
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

    DrawRectanglePro((Rectangle){ player.p.pos.x + 16, player.p.pos.y + 16, 29, 12 }, (Vector2){ 0, 6 }, player.radian * (180 / PI) , BLACK);
    // if (player.invincible != 0) {
    //     // DrawRectangleRounded((Rectangle) { player.p.pos.x+1, player.p.pos.y+1, 30, 30 }, 0.3, 1, Fade(player.COLORS[2], 0.5));
    //     DrawTextureEx(playerBodyTexture, (Vector2) {player.p.pos.x, player.p.pos.y}, 0, 1, WHITE);
    // }
    // else {
        // Draw body of the tank
        // DrawRectangleRounded((Rectangle) { player.p.pos.x+1, player.p.pos.y+1, 30, 30 }, 0.3, 1, player.COLORS[2]);
        DrawTextureEx(playerBodyTexture, (Vector2) {player.p.pos.x, player.p.pos.y}, 0, 1, WHITE);
    // }

    Rectangle playerCanon = { player.p.pos.x + 16, player.p.pos.y + 16, 26, 6 };
    Vector2 originCanon = { 0, 3 };
    DrawRectanglePro((Rectangle){ player.p.pos.x + 16, player.p.pos.y + 16, 28, 10 }, (Vector2){ 0, 5 }, player.radian * (180 / PI) , WHITE);
    DrawRectanglePro(playerCanon, originCanon, player.radian * (180 / PI), player.COLORS[1]);

    DrawTextureEx(playerFaceTexture, (Vector2) {player.p.pos.x, player.p.pos.y}, 0, 1, player.COLORS[2]);
    DrawTexturePro(playerTemplateTexture, (Rectangle) {0,0,32,32}, (Rectangle) {player.p.pos.x+16, player.p.pos.y+16,32,32}, (Vector2) {16, 16}, player.radian * (180 / PI) + 90, WHITE);

    DrawRectangleRec((Rectangle){ player.p.pos.x - 17 + 19, player.p.pos.y - 50 + 39, (player.charge - 2) * 2.7, 6 }, Fade(WHITE, 0.4));
    DrawRectangleRec((Rectangle){ player.p.pos.x - 17 + 20, player.p.pos.y - 50 + 40, (player.charge - 2) * 2.6, 4 }, Fade(player.COLORS[1], 0.8));

    // if (player.invincible != 0) {
    //     DrawRectangleRounded((Rectangle) { player.p.pos.x, player.p.pos.y, player.p.size.x, player.p.size.y }, 0.3, 1, Fade(player.COLORS[2], 0.5));
    // }
    // else {
    //     // Draw body of the tank
    //     DrawRectangleRounded((Rectangle) { player.p.pos.x, player.p.pos.y, player.p.size.x, player.p.size.y }, 0.3, 1, player.COLORS[2]);
    // }



    // // Draw cannon of the tank
    // Rectangle playerCanon = { player.p.pos.x + 20, player.p.pos.y + 20, 30, 10 };
    // Vector2 originCanon = { 0, 5 };
    // DrawRectanglePro((Rectangle){ player.p.pos.x + 20, player.p.pos.y + 20, 32, 12 }, (Vector2){ 0, 6 }, player.radian * (180 / PI) , WHITE);
    // DrawCircle(player.p.pos.x + 20, player.p.pos.y + 20, 14, WHITE);
    // DrawRectanglePro(playerCanon, originCanon, player.radian * (180 / PI), player.COLORS[1]);
    // DrawCircle(player.p.pos.x + 20, player.p.pos.y + 20, 13, player.COLORS[1]);

    // // Draw life of the tank
    // float posX = player.p.pos.x - 5 + 20;
    // if (player.life == 1) {
    //     posX = player.p.pos.x - 2 + 20;
    // }
    // DrawText(TextFormat("%d", player.life), posX, player.p.pos.y - 28.5 + 40, 20, WHITE);
    // // DrawText(TextFormat("%f %f", cos(player.radian), sin(+ player.radian)), player.p.pos.x + 40, player.p.pos.y + 40, 10, BLACK);
    // // DrawText(TextFormat("%f %f", , , player.p.pos.x + 40, player.p.pos.y + 40, 10, BLACK);

    // // Draw load of the shoot
    // DrawRectangleRec((Rectangle){ player.p.pos.x - 17 + 20, player.p.pos.y - 50 + 40, (player.charge - 2) * 2.6, 4 }, Fade(player.COLORS[1], 0.4));
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
        DrawTextPro(GetFontDefault(), TextFormat("%d", player.life), (Vector2){ 180, 100 + 120 * (player.id - 1) }, (Vector2){ 40, 0 }, 20, 40, 10, colorDisplay0);
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
        DrawTextPro(GetFontDefault(), TextFormat("%d", player.life), (Vector2){ GetScreenWidth() - 130, -40 + 120 * (player.id - 1) }, (Vector2){ 40, 0 }, -20, 40, 10, colorDisplay0);
    }
}