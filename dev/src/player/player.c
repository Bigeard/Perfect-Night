#include "../../../lib/raylib/src/raylib.h"
#include "../../../lib/raylib/src/raymath.h"
#include <emscripten/emscripten.h>
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

    if (player->ammunition < 3) {
        player->ammunitionLoad = player->ammunitionLoad - delta;
    }
    if (player->ammunitionLoad <= 0) {
        player->ammunition++;
        player->ammunitionLoad = 600;
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
                        {cos(player->radian) * player->charge, sin(player->radian) * player->charge},
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
                        {cos(player->radian) * player->charge, sin(player->radian) * player->charge},
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
                        {cos(player->radian) * player->charge, sin(player->radian) * player->charge},
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
    if ((player->p.pos.x >= arenaSizeX || 
        player->p.pos.x + player->p.size.x <= 0) ||
        (player->p.pos.y >= arenaSizeY || 
        player->p.pos.y + player->p.size.y <= 0)) {

        if (player->life > 0) player->life--;
        player->p.pos = (Vector2) { player->spawn.x, player->spawn.y };
        // player->p.vel = (Vector2) { 0, 0 };
        // player->speed = (Vector2) { 3.5, 3.5 };
        // player->damagesTaken = 0;
        // player->invincible = 300;
    }
}

void CollisionBulletPlayer(bool bulletCollision, Bullet *bullet, Player *player, Rectangle recPlayer) {
    if (bullet->inactive) return;
    if (player->life <= 0) return;
    if (player->invincible == 0 && ColorToInt(bullet->COLOR) != ColorToInt(player->COLORS[0])) {
        if (bulletCollision) {
            player->life--;
            player->invincible = 300;
        }
    }
}

Color LightenColor(Color color, float percentage) {
    return (Color) {
        (255 - color.r) * (1 - percentage) + color.r, 
        (255 - color.g) * (1 - percentage) + color.g, 
        (255 - color.b) * (1 - percentage) + color.b, 
        color.a
    };
}

void DrawPlayer(Player player) {
    if (!player.id) return;
    if (player.life <= 0) return;

    // Init Color
    Color color = player.COLORS[1];
    Color whiteColor = WHITE;
    Color blackColor = BLACK;

    if (player.invincible) {
        // Lighten color
        color = LightenColor(color, 0.5);
        whiteColor = Fade(WHITE, 0.5);
    }
    
    // Draw BLACK 1 Cannon
    DrawRectanglePro((Rectangle){ player.p.pos.x + 20, player.p.pos.y + 20, 33, 14 }, (Vector2){ 0, 7 }, player.radian * (180 / PI), blackColor);
    
    // Draw Body of the Tank
    DrawTextureEx(playerBodyTexture, (Vector2) {player.p.pos.x, player.p.pos.y}, 0, 1, whiteColor);

    // Draw WHITE 2 and COLOR 3 Cannon
    DrawRectanglePro((Rectangle){ player.p.pos.x + 20, player.p.pos.y + 20, 32, 12 }, (Vector2){ 0, 6 }, player.radian * (180 / PI), WHITE);
    DrawRectanglePro((Rectangle){ player.p.pos.x + 20, player.p.pos.y + 20, 30, 8 }, (Vector2){ 0, 4 }, player.radian * (180 / PI), color);

    // Draw Face / Template of the tank
    DrawTextureEx(playerFaceTexture, (Vector2) {player.p.pos.x+3, player.p.pos.y+3}, 0, 1, color);
    DrawTexturePro(playerTemplateTexture, (Rectangle) {0,0,32,32}, (Rectangle) {player.p.pos.x+20, player.p.pos.y+20,32,32}, (Vector2) {16, 16}, player.radian * (180 / PI) + 90, WHITE);

    // Draw the progress bar of the charge
    DrawRectangleRec((Rectangle){ player.p.pos.x - 17 + 19, player.p.pos.y - 50 + 39, (player.charge - 2) * 2.7, 6 }, Fade(WHITE, 0.4));
    DrawRectangleRec((Rectangle){ player.p.pos.x - 17 + 20, player.p.pos.y - 50 + 40, (player.charge - 2) * 2.6, 4 }, Fade(player.COLORS[1], 0.8));

    for (int a = 0; a < 3; a++) {
        int ammunitionDisplay = player.ammunition;
        if (player.ammunition != 3) ammunitionDisplay++;
        float calcRadian = player.radian+PI+(a*0.7 + 0.7/2 - (ammunitionDisplay * 0.7 / 2));
        float loadColor = 1;
        if (a >= player.ammunition) {
            loadColor = (600.f - player.ammunitionLoad) / 600.f;
        }
        float ammunitionPosX = (player.p.pos.x + player.p.size.x / 2.0f) + 25 * cos(calcRadian);
        float ammunitionPosY = (player.p.pos.y + player.p.size.x / 2.0f) + 25 * sin(calcRadian);
        DrawCircle(ammunitionPosX, ammunitionPosY, 7.3, Fade(BLACK, loadColor+0.3));
        DrawCircle(ammunitionPosX, ammunitionPosY, 6.5,WHITE);
        DrawCircle(ammunitionPosX, ammunitionPosY, 5, LightenColor(player.COLORS[0], loadColor));
        if (a >= player.ammunition) break;
    }



    if (activeDev) {
        // DrawRectangleRec((Rectangle) { player.p.pos.x, player.p.pos.y, player.p.size.x - 0.1, player.p.size.y - 0.1 }, RED);
    }
}

void DrawSpawnPlayer(Player player) {
    DrawRectangleLinesEx((Rectangle){player.spawn.x, player.spawn.y, player.p.size.x, player.p.size.y}, 2.5, player.COLORS[0]);
}

void DrawStatsPlayer(Player player) {
    if (activeDev) {
    
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
}