#include "../../../../../lib/raylib/src/raylib.h"
#include "../../../../../lib/raylib/src/raymath.h"
#include "../../../gameplay/gameplay.h"
#include "../../../player/player.h"
#include "../../../tool/tool.h"
#include "./multi_shot.h"
#include "../../item.h"


Item InitItemMultiShot(int player_id) {
    return (Item) {
        player_id,
        "Multi Shoot",
        true,
        GetTime(),
        8,
        false,
        false,
        ShootItemMultiShot,
        UpdateItemMultiShot,
        DrawItemMultiShot,
        { 3 }
    };
}

void ShootItemMultiShot(Item *item, float calcPosRadianX, float calcPosRadianY, float delta_x, float delta_y) {
    if (!item->active) return;
    Player *player = &players[item->player_id-1];
    for (int i = 0; i < item->multiShot.numCannon; i++) {
        // @TODO Fix bug direction for eg with 7 cannons
        // TraceLog(LOG_INFO, "%d", (int)floor((float)item->multiShot.numCannon/2));
        float radian = (player->lastRadian * (180 / PI) - 40 * (int)floor((float)item->multiShot.numCannon/2) + i * 40) / (180 / PI);
        // calcPosRadianX = (calcPosRadianX * (180 / PI) - 40 * (int)floor((float)item->multiShot.numCannon/2) + i * 40) / (180 / PI);
        // calcPosRadianY = (calcPosRadianY * (180 / PI) - 40 * (int)floor((float)item->multiShot.numCannon/2) + i * 40) / (180 / PI);
        player->bullets[player->lastBullet] = (Bullet) {
            player->id,
            (Physic) {
                {player->p.pos.x + 20 + calcPosRadianX * 22 - 5, player->p.pos.y + 20 + calcPosRadianY * 22 - 5},
                {5, 5},
                {cos(radian) * player->charge + delta_x * 2, sin(radian) * player->charge + delta_y * 2},
                {0, 0, 0, 0, 0}
            }, 
            { player->charge + delta_x * 2, player->charge + delta_y * 2 }, 
            radian,
            false,
            false,
            true,
            player->COLORS[0]
        };
        player->lastBullet += 1;
        if (player->lastBullet >= sizeof(player->bullets)/sizeof(player->bullets[0])) {
            player->lastBullet = 0;
        }
    }
}

void UpdateItemMultiShot(Item *item) {
    if (!item->active) return;
    double elapsedTime = GetTime() - item->timer; 
    if (item->maxTimer < elapsedTime) {
        item->active = false;
    }
}

void DrawItemMultiShot(Item *item) {
    if (!item->active) return;
    Player player = players[item->player_id-1];

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
    for (int i = 0; i < item->multiShot.numCannon; i++) {
        DrawRectanglePro((Rectangle){ player.p.pos.x + 20, player.p.pos.y + 20, 33, 14 }, (Vector2){ 0, 7 }, player.radian * (180 / PI) - 40 * (int)floor((float)item->multiShot.numCannon/2) + i * 40, blackColor);
    }
    // Draw Body of the Tank
    DrawTextureEx(playerBodyTexture, (Vector2) {player.p.pos.x, player.p.pos.y}, 0, 1, whiteColor);

    // Draw WHITE 2 and COLOR 3 Cannon
    for (int i = 0; i < item->multiShot.numCannon; i++) {
        DrawRectanglePro((Rectangle){ player.p.pos.x + 20, player.p.pos.y + 20, 32, 12 }, (Vector2){ 0, 6 }, player.radian * (180 / PI) - 40 * (int)floor((float)item->multiShot.numCannon/2) + i * 40, WHITE);
    }
    for (int i = 0; i < item->multiShot.numCannon; i++) {
        DrawRectanglePro((Rectangle){ player.p.pos.x + 20, player.p.pos.y + 20, 30, 8 }, (Vector2){ 0, 4 }, player.radian * (180 / PI) - 40 * (int)floor((float)item->multiShot.numCannon/2) + i * 40, color);
    }

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
}
