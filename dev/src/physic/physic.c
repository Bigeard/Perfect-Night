#include "../../../raylib/src/raylib.h"
#include "../../../raylib/src/raymath.h"
#include "stdio.h"

#include "physic.h"


void CollisionPhysic(Physic *phyA, Rectangle recA, Rectangle recB) {
    bool collision = false;
    if (phyA->pos.x != recA.x) { // For circle
        collision = CheckCollisionCircleRec((Vector2){phyA->pos.x, phyA->pos.y}, phyA->size.x*2, recB);
    } else {
        collision = CheckCollisionRecs(recA, recB);
    }

    Rectangle boxCollision = {0};
    bool top = false;
    bool bottom = false;
    bool left = false;
    bool right = false;

    if (collision) {
        boxCollision = GetCollisionRec(recA, recB);

        if (boxCollision.width + boxCollision.height >= 5 || boxCollision.width >= 10 || boxCollision.height >= 10){
            phyA->collision[0] = collision;
            // Detect Colision Position
            if (recB.x == boxCollision.x && recA.x < boxCollision.x && 
                recB.y == boxCollision.y && recA.y < boxCollision.y) {
                // top, left
                if (boxCollision.height > boxCollision.width) left = true;
                else top = true;
            }
            if (recB.x < boxCollision.x && recA.x == boxCollision.x && 
                recB.y == boxCollision.y && recA.y < boxCollision.y) {
                // top, right
                if (boxCollision.height > boxCollision.width) right = true;
                else top = true;
            }
            if (recB.x == boxCollision.x && recA.x < boxCollision.x && 
                recB.y < boxCollision.y && recA.y == boxCollision.y) {
                // bottom, left
                if (boxCollision.height > boxCollision.width) left = true;
                else bottom = true;
            }
            if (recB.x < boxCollision.x && recA.x == boxCollision.x && 
                recB.y < boxCollision.y && recA.y == boxCollision.y) {
                // bottom, right
                if (boxCollision.height > boxCollision.width) right = true;
                else bottom = true;
            }
        }
    }
    if (phyA->collision[0]) {
        // Condition
        if (top) { // top
            phyA->collision[1] = top;
            phyA->pos.y = recB.y - (0.1 + recA.height);
        }
        if (bottom) { // bottom
            phyA->collision[1] = bottom;
            phyA->pos.y = recB.y + recB.height + 0.1;
        }
        if (left) { // left
            phyA->collision[3] = left;
            phyA->pos.x = recB.x - (0.1 + recA.width);
        }
        if (right) { // right
            phyA->collision[4] = right;
            phyA->pos.x = recB.x + recB.width + 0.1;
        }
    }
}
