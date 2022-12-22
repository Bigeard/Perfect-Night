#ifndef PLAYER_H
#define PLAYER_H

#ifdef __cplusplus
extern "C" {
#endif


#include "../physic/physic.h"
#include "../bullet/bullet.h"

// INPUT_TYPE
#define MOUSE 0
#define KEYBOARD 1
#define GAMEPAD 2
#define MOBILE 3

typedef struct Player {
    int id; // Identifier
    char* gamepadId; // Gamepad identifier
    int life; // Number of life (by default: 3)
    int invincible; // Time of invincibility
    int damagesTaken; // Percentage of damages token
    int ammunition; // Ammunition
    int ammunitionLoad; // Ammunition loading
    Physic p; // Physic (position, velocity...)
    Vector2 spawn;
    Vector2 speed;
    float charge;
    bool canShoot;
    int timeShoot;
    float radian; // Determine the position of the cannon
    Bullet bullets[4]; // Array of bullet
    int lastBullet; // Allow the ball to be replaced one after the other
    Color COLORS[3];
    int INPUT_TYPE; // Type of input (mouse, keyboard, gamepad)
    int KEY[7]; // Key you can press to move or do an action
} Player;

void InitPlayer(void);
void UpdatePlayer(Player *player);
void CollisionBulletPlayer(Bullet *bullet, Player *player, Rectangle recPlayer);
void DrawPlayer(Player player);
void DrawSpawnPlayer(Player player);
void DrawStatsPlayer(Player player);


#ifdef __cplusplus
}
#endif

#endif // PLAYER_H
