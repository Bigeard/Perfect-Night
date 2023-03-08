#include "../../../../../lib/raylib/src/raylib.h"
#include "../../../../../lib/raylib/src/raymath.h"

#include "./bonus_ammunition.h"
#include "../../item.h"
#include "../../../gameplay/gameplay.h"
#include "../../../player/player.h"
#include "../../../tool/tool.h"

Item InitItemBonusAmmunition(int player_id)
{
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
        {1, 0.0, 0, 1}};
    Player player = players[player_id - 1];
    if (ColorToInt(themeColor[item.bonusAmmunition.idColorA]) == ColorToInt(player.color))
    {
        item.bonusAmmunition.idColorA = item.bonusAmmunition.idColorB;
        item.bonusAmmunition.idColorB++;
    }
    if (ColorToInt(themeColor[item.bonusAmmunition.idColorB]) == ColorToInt(player.color))
    {
        item.bonusAmmunition.idColorB++;
    }
    if (item.bonusAmmunition.idColorB >= sizeof(themeColor) / sizeof(themeColor[0]))
    {
        item.bonusAmmunition.idColorB = 0;
    }
    return item;
}

void ShootItemBonusAmmunition(Item *item, float calcPosRadianX, float calcPosRadianY, float delta_x, float delta_y)
{
    if (!item->active)
        return;
    Player *player = &players[item->player_id - 1];
    player->ammunition++;
}

void UpdateItemBonusAmmunition(Item *item)
{
    if (!item->active)
        return;
    const Player player = players[item->player_id - 1];
    if (item->maxTimer < GetTime() - item->timer)
    {
        item->active = false;
    }
    item->bonusAmmunition.animationTimer += 0.004f;
    if (item->bonusAmmunition.animationTimer >= 1.0f)
    {
        item->bonusAmmunition.idColorA = item->bonusAmmunition.idColorB;
        item->bonusAmmunition.idColorB++;
        if (ColorToInt(themeColor[item->bonusAmmunition.idColorB]) == ColorToInt(player.color))
        {
            item->bonusAmmunition.idColorB++;
        }
        if (item->bonusAmmunition.idColorB >= sizeof(themeColor) / sizeof(themeColor[0]))
        {
            item->bonusAmmunition.idColorB = 0;
        }
        if (ColorToInt(themeColor[item->bonusAmmunition.idColorB]) == ColorToInt(player.color))
        {
            item->bonusAmmunition.idColorB++;
        }
        item->bonusAmmunition.animationTimer = 0.0f;
    }
}

void DrawItemBonusAmmunition(Item *item)
{
    if (!item->active)
        return;
    const Player player = players[item->player_id - 1];

    // Init Color
    Color color = player.color;
    Color whiteColor = WHITE;
    Color blackColor = BLACK;

    if (player.invincible)
    {
        // Lighten color
        color = LightenColor(color, 0.5);
        whiteColor = Fade(WHITE, 0.5);
    }

    // Draw BLACK 1 Cannon
    DrawRectanglePro((Rectangle){player.p.pos.x + 20, player.p.pos.y + 20, 33, 14}, (Vector2){0, 7}, player.radian * (180 / PI), blackColor);

    // Draw Border of the Tank
    DrawTextureEx(playerBorderTexture, (Vector2){player.p.pos.x, player.p.pos.y}, 0, 1, whiteColor);

    // Draw WHITE 2 and COLOR 3 Cannon
    DrawRectanglePro((Rectangle){player.p.pos.x + 20, player.p.pos.y + 20, 32, 12}, (Vector2){0, 6}, player.radian * (180 / PI), WHITE);
    DrawRectanglePro((Rectangle){player.p.pos.x + 20, player.p.pos.y + 20, 30, 8}, (Vector2){0, 4}, player.radian * (180 / PI), color);

    // Draw Body / Template of the tank
    DrawTextureEx(playerBodyTexture, (Vector2){player.p.pos.x + 3, player.p.pos.y + 3}, 0, 1, color);
    DrawTexturePro(playerTemplatesTextures[player.id], (Rectangle){0, 0, 32, 32}, (Rectangle){player.p.pos.x + 20, player.p.pos.y + 20, 32, 32}, (Vector2){16, 16}, player.radian * (180 / PI) + 90, WHITE);

    for (int a = 0; a < player.ammunition; a++)
    {
        float calcRadian = player.radian + PI + (a * 0.7 + 0.7 / 2 - (player.ammunition * 0.7 / 2));
        float ammunitionPosX = (player.p.pos.x + player.p.size.x / 2.0f) + 25 * cosf(calcRadian);
        float ammunitionPosY = (player.p.pos.y + player.p.size.x / 2.0f) + 25 * sinf(calcRadian);
        DrawCircle(ammunitionPosX, ammunitionPosY, 7.3, BLACK);
        DrawCircle(ammunitionPosX, ammunitionPosY, 6.5, WHITE);
        DrawCircle(ammunitionPosX, ammunitionPosY, 5, LerpColor(themeColor[item->bonusAmmunition.idColorA], themeColor[item->bonusAmmunition.idColorB], item->bonusAmmunition.animationTimer));
    }

    // Draw the progress bar of the charge
    if (player.charge != 2)
    {
        DrawRing(
            (Vector2){player.p.pos.x + player.p.size.x / 2, player.p.pos.y + player.p.size.y / 2},
            38.0f,
            47.0f,
            player.radian * (180 / PI) * -1 + 270 - (player.charge - 2) * 4 - 2,
            player.radian * (180 / PI) * -1 + 270 + (player.charge - 2) * 4 + 2,
            0,
            Fade(WHITE, 0.6));

        DrawRing(
            (Vector2){player.p.pos.x + player.p.size.x / 2, player.p.pos.y + player.p.size.y / 2},
            40.0f,
            45.0f,
            player.radian * (180 / PI) * -1 + 270 - (player.charge - 2) * 4,
            player.radian * (180 / PI) * -1 + 270 + (player.charge - 2) * 4,
            0,
            Fade(player.color, 0.8));
    }

    // Animation pick-up item
    double elapsedTime = GetTime() - item->timer;
    if (elapsedTime <= 4.0)
    {
        DrawTexture(
            BonusAmmunitionTexture,
            (player.p.pos.x + player.p.size.x / 2.0f) - 17.0f,
            player.p.pos.y - 27.0f,
            Fade(player.color, 1.0f - ((float)elapsedTime / 4.0f)));
    }
}
