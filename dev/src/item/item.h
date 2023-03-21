#ifndef ITEM_H
#define ITEM_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../../lib/raylib/src/raylib.h"

#include "items/bonus_ammunition/bonus_ammunition.h"
#include "items/bonus_life/bonus_life.h"
#include "items/bonus_speed/bonus_speed.h"
#include "items/laser/laser.h"
#include "items/nothing/nothing.h"
#include "items/multi_shoot/multi_shoot.h"
#include "items/rocket/rocket.h"
#include "items/sword/sword.h"

#define FLT_MAX 3.402823466e+38F /* max float value */

    enum TypeItem
    {
        RANDOM = 0,
        BONUS_AMMUNITION = 1,
        BONUS_LIFE = 2,
        BONUS_SPEED = 3,
        LASER = 4,
        MULTI_SHOOT = 5,
        ROCKET = 6,
        SWORD = 7,
        NOTHING = 8
    };

    typedef struct Item
    {
        int player_id;
        enum TypeItem type;
        bool active;
        double timer;
        double maxTimer;
        bool defaultShoot;
        bool defaultDisplay;
        void (*ShootItem)(struct Item *, const float, const float, const float, const float);
        void (*UpdateItem)(struct Item *);
        void (*DrawItem)(struct Item *);
        union
        {
            BonusAmmunition bonusAmmunition;
            BonusLife bonusLife;
            BonusSpeed bonusSpeed;
            Laser laser;
            Nothing nothing;
            MultiShoot multiShoot;
            Rocket rocket;
            Sword sword;
        };
    } Item;

    void InitItemWithTypeItem(int player_id, enum TypeItem type, float maxTimer);

    Item InitItemBonusAmmunition(int player_id, float maxTimer);
    void ShootItemBonusAmmunition(Item *item, float calcPosRadianX, float calcPosRadianY, float delta_x, float delta_y);
    void UpdateItemBonusAmmunition(Item *item);
    void DrawItemBonusAmmunition(Item *item);

    Item InitItemBonusLife(int player_id, float maxTimer);
    void ShootItemBonusLife(Item *item, float calcPosRadianX, float calcPosRadianY, float delta_x, float delta_y);
    void UpdateItemBonusLife(Item *item);
    void DrawItemBonusLife(Item *item);

    Item InitItemBonusSpeed(int player_id, float maxTimer);
    void ShootItemBonusSpeed(Item *item, float calcPosRadianX, float calcPosRadianY, float delta_x, float delta_y);
    void UpdateItemBonusSpeed(Item *item);
    void DrawItemBonusSpeed(Item *item);

    Item InitItemLaser(int player_id, float maxTimer);
    void ShootItemLaser(Item *item, float calcPosRadianX, float calcPosRadianY, float delta_x, float delta_y);
    void UpdateItemLaser(Item *item);
    void DrawItemLaser(Item *item);

    Item InitItemNothing(int player_id, float maxTimer);
    void ShootItemNothing(Item *item, float calcPosRadianX, float calcPosRadianY, float delta_x, float delta_y);
    void UpdateItemNothing(Item *item);
    void DrawItemNothing(Item *item);

    Item InitItemMultiShoot(int player_id, float maxTimer);
    void ShootItemMultiShoot(Item *item, float calcPosRadianX, float calcPosRadianY, float delta_x, float delta_y);
    void UpdateItemMultiShoot(Item *item);
    void DrawItemMultiShoot(Item *item);

    Item InitItemRocket(int player_id, float maxTimer);
    void ShootItemRocket(Item *item, float calcPosRadianX, float calcPosRadianY, float delta_x, float delta_y);
    void UpdateItemRocket(Item *item);
    void DrawItemRocket(Item *item);

    Item InitItemSword(int player_id, float maxTimer);
    void ShootItemSword(Item *item, float calcPosRadianX, float calcPosRadianY, float delta_x, float delta_y);
    void UpdateItemSword(Item *item);
    void DrawItemSword(Item *item);

#ifdef __cplusplus
}
#endif

#endif // ITEM_H
