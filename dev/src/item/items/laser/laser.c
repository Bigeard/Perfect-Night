#include "../../../../../lib/raylib/src/raylib.h"
#include "../../../../../lib/raylib/src/raymath.h"

#include "../../../gameplay/gameplay.h"
#include "../../../player/player.h"
#include "../../../tool/tool.h"
#include "./laser.h"
#include "../../item.h"


Item InitItemLaser(int player_id) {
    return (Item) {
        player_id,
        "Laser",
        true,
        GetTime(),
        10.0,
        false,
        false,
        ShootItemLaser,
        UpdateItemLaser,
        DrawItemLaser,
        { 1 }
    };
}

void ShootItemLaser(Item *item, float calcPosRadianX, float calcPosRadianY, float delta_x, float delta_y) {
    // Player *player = &players[item->player_id-1];
    // @TODO
}

void UpdateItemLaser(Item *item) {
    double elapsedTime = GetTime() - item->timer; 
    if (item->maxTimer < elapsedTime) {
        item->active = false;
    }
}

void DrawItemLaser(Item *item) {
    Player player = players[item->player_id-1];

    // Init Color
    Color color = player.COLORS[1];
    Color whiteColor = WHITE;
    // Color blackColor = BLACK;

    if (player.invincible) {
        // Lighten color
        color = LightenColor(color, 0.5);
        whiteColor = Fade(WHITE, 0.5);
    }

    // Draw BLACK 1 Cannon
    // @TODO

    // Draw Body of the Tank
    DrawTextureEx(playerBodyTexture, (Vector2) {player.p.pos.x, player.p.pos.y}, 0, 1, whiteColor);

    // Draw WHITE 2 and COLOR 3 Cannon
    // @TODO

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
