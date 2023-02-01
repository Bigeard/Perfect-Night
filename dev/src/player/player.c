#include "../../../lib/raylib/src/raylib.h"
#include "../../../lib/raylib/src/raymath.h"
#include <emscripten/emscripten.h>
#include "stdio.h"

#include "player.h"
#include "../gameplay/gameplay.h"
#include "../bullet/bullet.h"
#include "../tool/tool.h"

int maxAmmunition = 4;
float delayAmmunition = 3.0f;

EM_JS(float, GetJoystickMobileLeftX, (const char* id), { return listGamepad.get(Module.UTF8ToString(id)).axes[0] });
EM_JS(float, GetJoystickMobileLeftY, (const char* id), { return listGamepad.get(Module.UTF8ToString(id)).axes[1] });
EM_JS(float, GetJoystickMobileRightX, (const char* id), { return listGamepad.get(Module.UTF8ToString(id)).axes[2] });
EM_JS(float, GetJoystickMobileRightY, (const char* id), { return listGamepad.get(Module.UTF8ToString(id)).axes[3] });


Texture2D playerBodyTexture;
Texture2D playerFaceTexture;
Texture2D playerTemplatesTextures[9];


void InitPlayer(void) {
    Image playerBodyImage = LoadImage("resources/player-body.png");
    playerBodyTexture = LoadTextureFromImage(playerBodyImage);

    Image playerFaceImage = LoadImage("resources/player-face.png");
    playerFaceTexture = LoadTextureFromImage(playerFaceImage);

    Image playerFaceTemplate = LoadImage("resources/player-template.png");
    Image playerFaceTemplate1 = LoadImage("resources/player-1-template.png");
    Image playerFaceTemplate2 = LoadImage("resources/player-2-template.png");
    Image playerFaceTemplate3 = LoadImage("resources/player-3-template.png");
    Image playerFaceTemplate4 = LoadImage("resources/player-4-template.png");
    Image playerFaceTemplate5 = LoadImage("resources/player-5-template.png");
    Image playerFaceTemplate6 = LoadImage("resources/player-6-template.png");
    Image playerFaceTemplate7 = LoadImage("resources/player-7-template.png");
    Image playerFaceTemplate8 = LoadImage("resources/player-8-template.png");

    // playerTemplateTexture = LoadTextureFromImage(playerFaceTemplate);
    playerTemplatesTextures[0] = LoadTextureFromImage(playerFaceTemplate);
    playerTemplatesTextures[1] = LoadTextureFromImage(playerFaceTemplate1);
    playerTemplatesTextures[2] = LoadTextureFromImage(playerFaceTemplate2);
    playerTemplatesTextures[3] = LoadTextureFromImage(playerFaceTemplate3);
    playerTemplatesTextures[4] = LoadTextureFromImage(playerFaceTemplate4);
    playerTemplatesTextures[5] = LoadTextureFromImage(playerFaceTemplate5);
    playerTemplatesTextures[6] = LoadTextureFromImage(playerFaceTemplate6);
    playerTemplatesTextures[7] = LoadTextureFromImage(playerFaceTemplate7);
    playerTemplatesTextures[8] = LoadTextureFromImage(playerFaceTemplate8);
}

void UpdatePlayer(Player *player) {
    float delta = GetFrameTime();

    if (player->life <= 0) {
        // @TODO Take position of winner
        return;
    }

    // Load Invincible
    if (player->invincible > 1) {
        player->invincible = player->invincible - delta;
    } else {
        player->invincible = 0;
    }

    // Load Ammunition
    if (player->ammunition < maxAmmunition) {
        player->ammunitionLoad = player->ammunitionLoad - delta;
    }
    if (player->ammunitionLoad <= 0.0f) {
        player->ammunition++;
        player->ammunitionLoad = delayAmmunition;
    }

    player->p.collision[0] = false;
    player->p.collision[1] = false;
    player->p.collision[2] = false;
    player->p.collision[3] = false;
    player->p.collision[4] = false;

    float delta_x = player->p.vel.x;
    float delta_y = player->p.vel.y;

    // MOBILE CONTROLLER
    if (player->INPUT_TYPE == MOBILE) {
        // Move Player
        delta_x = player->speed.x * GetJoystickMobileLeftX(player->gamepadId);
        delta_y = player->speed.y * GetJoystickMobileLeftY(player->gamepadId);

        // Move Cannon
        float joystickRightX = GetJoystickMobileRightX(player->gamepadId);
        float joystickRightY = GetJoystickMobileRightY(player->gamepadId);

        float joystickLeftX = GetJoystickMobileLeftX(player->gamepadId);
        float joystickLeftY = GetJoystickMobileLeftY(player->gamepadId);

        float distance = sqrt(pow(joystickRightY, 2) + pow(joystickRightX, 2));

        if (joystickLeftX != 0.0f || joystickLeftY != 0.0f) {
            player->radian = atan2f(joystickLeftY, joystickLeftX);
        }

        if ((joystickRightX != 0.0f || joystickRightY != 0.0f) && distance > 1.0f) {
            player->radian = atan2f(joystickRightY, joystickRightX);
            player->lastRadian = atan2f(joystickRightY, joystickRightX);
        }
        if(distance < 1.0f && distance > 0.1) {
            player->lastRadian = player->radian;
        }

        // Charge / Shoot Bullet
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

                float calcPosRadianX = cos(player->lastRadian);
                float calcPosRadianY = sin(player->lastRadian);

                if ((calcPosRadianX > 0.6 && calcPosRadianX < 0.8) ||
                    (calcPosRadianX < -0.6 && calcPosRadianX > -0.8)) {
                    calcPosRadianX = lround(cos(player->lastRadian));
                }
                if ((calcPosRadianY > 0.6 && calcPosRadianY < 0.8) ||
                    (calcPosRadianY < -0.6 && calcPosRadianY > -0.8)) {
                    calcPosRadianY = lround(sin(player->lastRadian));
                }

                player->bullets[player->lastBullet] = (Bullet) { 
                    player->id,
                    (Physic) {
                        {player->p.pos.x + 20 + calcPosRadianX * 22 - 5, player->p.pos.y + 20 + calcPosRadianY * 22 - 5},
                        {5, 5},
                        {cos(player->lastRadian) * player->charge + delta_x * 2, sin(player->lastRadian) * player->charge + delta_y * 2},
                        {0, 0, 0, 0, 0}
                    }, 
                    { player->charge + delta_x * 2, player->charge + delta_y * 2 }, 
                    player->lastRadian,
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
    if ((player->p.pos.x >= arenaSizeX || 
        player->p.pos.x + player->p.size.x <= 0) ||
        (player->p.pos.y >= arenaSizeY || 
        player->p.pos.y + player->p.size.y <= 0)) {

        if (lastOutsidePlayer->id != player->id) {
            outsidePlayer = player;
        }

    } else {
        if (outsidePlayer->id == player->id) {
            outsidePlayer = NULL;
        }
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
    DrawTexturePro(playerTemplatesTextures[player.id], (Rectangle) {0,0,32,32}, (Rectangle) {player.p.pos.x+20, player.p.pos.y+20,32,32}, (Vector2) {16, 16}, player.radian * (180 / PI) + 90, WHITE);

    for (int a = 0; a < maxAmmunition; a++) {
        if (a >= player.ammunition) break;
        int ammunitionDisplay = player.ammunition;
        // if (player.ammunition != maxAmmunition) ammunitionDisplay++;
        // float calcRadian = player.radian+PI+(a*0.7 + 0.7/2 - (ammunitionDisplay * 0.7 / 2));
        float calcRadian = player.radian+PI+(a*0.7 + 0.7/2 - (ammunitionDisplay * 0.7 / 2));
        float loadColor = 1;
        // if (a >= player.ammunition) {
        //     loadColor = (delayAmmunition - player.ammunitionLoad) / delayAmmunition;
        // }
        float ammunitionPosX = (player.p.pos.x + player.p.size.x / 2.0f) + 25 * cos(calcRadian);
        float ammunitionPosY = (player.p.pos.y + player.p.size.x / 2.0f) + 25 * sin(calcRadian);
        DrawCircle(ammunitionPosX, ammunitionPosY, 7.3, Fade(BLACK, loadColor+0.3));
        DrawCircle(ammunitionPosX, ammunitionPosY, 6.5,WHITE);
        DrawCircle(ammunitionPosX, ammunitionPosY, 5, ReverseColor(player.COLORS[0], loadColor));
        // if (a >= player.ammunition) break;
    }

    // Draw the progress bar of the charge
    DrawRectangleRec((Rectangle){ player.p.pos.x - 17 + 19, player.p.pos.y - 50 + 39, (player.charge - 2) * 2.7, 6 }, Fade(WHITE, 0.4));
    DrawRectangleRec((Rectangle){ player.p.pos.x - 17 + 20, player.p.pos.y - 50 + 40, (player.charge - 2) * 2.6, 4 }, Fade(player.COLORS[1], 0.8));

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