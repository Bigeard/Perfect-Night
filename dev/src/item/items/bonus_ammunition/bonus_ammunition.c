#include "../../../../../lib/raylib/src/raylib.h"
#include "../../../../../lib/raylib/src/raymath.h"

#include "./bonus_ammunition.h"
#include "../../item.h"
#include "../../../gameplay/gameplay.h"
#include "../../../player/player.h"
#include "../../../tool/tool.h"


Item InitItemBonusAmmunition(int player_id) {
    Item item = {
        player_id,
        "Bonus Amm.",
        true,
        GetTime(),
        5.0,
        true,
        false,
        ShootItemBonusAmmunition,
        UpdateItemBonusAmmunition,
        DrawItemBonusAmmunition,
        { 1, 0.0, 0, 1 }
    };
    Player player = players[player_id-1];
    if (ColorToInt(themeColor[item.bonusAmmunition.idColorA]) == ColorToInt(player.COLORS[0])) {
        item.bonusAmmunition.idColorA = item.bonusAmmunition.idColorB;
        item.bonusAmmunition.idColorB++;
    }
    if (ColorToInt(themeColor[item.bonusAmmunition.idColorB]) == ColorToInt(player.COLORS[0])) {
        item.bonusAmmunition.idColorB++;
    }
    if (item.bonusAmmunition.idColorB >= sizeof(themeColor)/sizeof(themeColor[0])) {
        item.bonusAmmunition.idColorB = 0;
    }
    return item;
}

void ShootItemBonusAmmunition(Item *item, float calcPosRadianX, float calcPosRadianY, float delta_x, float delta_y) {
    if (!item->active) return;
    Player *player = &players[item->player_id-1];
    player->ammunition++;
}

void UpdateItemBonusAmmunition(Item *item) {
    if (!item->active) return;
    Player player = players[item->player_id-1];
    double elapsedTime = GetTime() - item->timer; 
    if (item->maxTimer < elapsedTime) {
        item->active = false;
    }
    item->bonusAmmunition.animationTimer += 0.004f;
    if (item->bonusAmmunition.animationTimer >= 1.0f)
    {
        item->bonusAmmunition.idColorA = item->bonusAmmunition.idColorB;
        item->bonusAmmunition.idColorB++;
        if (ColorToInt(themeColor[item->bonusAmmunition.idColorB]) == ColorToInt(player.COLORS[0])) {
            item->bonusAmmunition.idColorB++;
        }
        if (item->bonusAmmunition.idColorB >= sizeof(themeColor)/sizeof(themeColor[0])) {
            item->bonusAmmunition.idColorB = 0;
        }
        if (ColorToInt(themeColor[item->bonusAmmunition.idColorB]) == ColorToInt(player.COLORS[0])) {
            item->bonusAmmunition.idColorB++;
        }
        item->bonusAmmunition.animationTimer = 0.0f;
    }
}

void DrawItemBonusAmmunition(Item *item) {
    if (!item->active) return;
    Player player = players[item->player_id-1];

    double elapsedTime = GetTime() - item->timer; 

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

    for (int a = 0; a < player.ammunition; a++) {
        float calcRadian = player.radian+PI+(a*0.7 + 0.7/2 - (player.ammunition * 0.7 / 2));
        float ammunitionPosX = (player.p.pos.x + player.p.size.x / 2.0f) + 25 * cos(calcRadian);
        float ammunitionPosY = (player.p.pos.y + player.p.size.x / 2.0f) + 25 * sin(calcRadian);
        DrawCircle(ammunitionPosX, ammunitionPosY, 7.3, BLACK);
        DrawCircle(ammunitionPosX, ammunitionPosY, 6.5,WHITE);
        DrawCircle(ammunitionPosX, ammunitionPosY, 5, LerpColor(
            themeColor[item->bonusAmmunition.idColorA], 
            themeColor[item->bonusAmmunition.idColorB], 
            item->bonusAmmunition.animationTimer
        ));
    }

    // Draw the progress bar of the charge
    DrawRectangleRec((Rectangle){ player.p.pos.x - 17 + 19, player.p.pos.y - 50 + 39, (player.charge - 2) * 2.7, 6 }, Fade(WHITE, 0.4));
    DrawRectangleRec((Rectangle){ player.p.pos.x - 17 + 20, player.p.pos.y - 50 + 40, (player.charge - 2) * 2.6, 4 }, Fade(player.COLORS[1], 0.8));

    if (elapsedTime <= 2.0) {
        DrawTexture(
            BonusAmmunitionTexture, 
            (player.p.pos.x + player.p.size.x / 2) - 17,
            player.p.pos.y - 27,
            Fade(player.COLORS[0], 1-(elapsedTime/2))
        );
    }
}
