#include "../../../lib/raylib/src/raylib.h"

#include "./item.h"
#include "../gameplay/gameplay.h"
#include "../player/player.h"


void InitItemWithTypeItem(int player_id, enum TypeItem type, float maxTimer)
{
    // @TODO change logic with player (include problem)
    Player *player = &players[player_id - 1];
    if (maxTimer == 0) {
        maxTimer = FLT_MAX;
    }
    if (type == RANDOM)
    {
        type = GetRandomValue(1, 8);
    }
    switch (type)
    {
    case BONUS_AMMUNITION:
        player->item = InitItemBonusAmmunition(player->id, maxTimer);
        break;
    case BONUS_LIFE:
        player->item = InitItemBonusLife(player->id, maxTimer);
        break;
    case BONUS_SPEED:
        player->item = InitItemBonusSpeed(player->id, maxTimer);
        break;
    case LASER:
        player->item = InitItemLaser(player->id, maxTimer);
        break;
    case MULTI_SHOOT:
        player->item = InitItemMultiShoot(player->id, maxTimer);
        break;
    case ROCKET:
        player->item = InitItemRocket(player->id, maxTimer);
        break;
    case SWORD:
        player->item = InitItemSword(player->id, maxTimer);
        break;
    case NOTHING:
        player->item = InitItemNothing(player->id, maxTimer);
        break;
    case WALL:
        player->item = InitItemWall(player->id, maxTimer);
        break;
    case RANDOM:
        break;
    }
}
