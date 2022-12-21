#include "../../../lib/raylib/src/raylib.h"
#include "../../../lib/raylib/src/raymath.h"
#include "stdio.h"

#include "physic.h"
#include "../line_segment/line_segment.h"
#include <stdbool.h>


void CollisionPhysic(Physic *phyA, Rectangle recA, Rectangle recB) {
    // bool collision = false;
    // if (phyA->pos.x != recA.x) { // For circle
    //     collision = CheckCollisionCircleRec((Vector2){phyA->pos.x, phyA->pos.y}, phyA->size.x*2, recB);
    // } else {
    //     collision = CheckCollisionRecs(recA, recB);
    // }

    // collision = CheckCollisionRecs(recB, recA) || CheckCollisionRecSegment(segment, recB);


    // Rectangle boxCollision = {0};
    // bool top = false;
    // bool bottom = false;
    // bool left = false;
    // bool right = false;

    // if (collision) {
    //     boxCollision = GetCollisionRec(recA, recB);

    //     if (boxCollision.width + boxCollision.height >= 5 || boxCollision.width >= 10 || boxCollision.height >= 10){
    //         phyA->collision[0] = collision;
    //         // Detect Colision Position
    //         if (recB.x == boxCollision.x && recA.x < boxCollision.x && 
    //             recB.y == boxCollision.y && recA.y < boxCollision.y) {
    //             // top, left
    //             if (boxCollision.height > boxCollision.width) left = true;
    //             else top = true;
    //         }
    //         if (recB.x < boxCollision.x && recA.x == boxCollision.x && 
    //             recB.y == boxCollision.y && recA.y < boxCollision.y) {
    //             // top, right
    //             if (boxCollision.height > boxCollision.width) right = true;
    //             else top = true;
    //         }
    //         if (recB.x == boxCollision.x && recA.x < boxCollision.x && 
    //             recB.y < boxCollision.y && recA.y == boxCollision.y) {
    //             // bottom, left
    //             if (boxCollision.height > boxCollision.width) left = true;
    //             else bottom = true;
    //         }
    //         if (recB.x < boxCollision.x && recA.x == boxCollision.x && 
    //             recB.y < boxCollision.y && recA.y == boxCollision.y) {
    //             // bottom, right
    //             if (boxCollision.height > boxCollision.width) right = true;
    //             else bottom = true;
    //         }
    //     }
    // }
    // if (phyA->collision[0]) {
    //     // Condition
    //     if (top) { // top
    //         phyA->collision[1] = top;
    //         phyA->pos.y = recB.y - (0.1 + recA.height);
    //     }
    //     if (bottom) { // bottom
    //         phyA->collision[2] = bottom;
    //         phyA->pos.y = recB.y + recB.height + 0.1;
    //     }
    //     if (left) { // left
    //         phyA->collision[3] = left;
    //         phyA->pos.x = recB.x - (0.1 + recA.width);
    //     }
    //     if (right) { // right
    //         phyA->collision[4] = right;
    //         phyA->pos.x = recB.x + recB.width + 0.1;
    //     }
    // }

    // Check for collisions between the two objects using a line segment
    Vector2 start = { phyA->pos.x, phyA->pos.y };
    Vector2 end = { phyA->pos.x + phyA->vel.x, phyA->pos.y + phyA->vel.y };
    LineSegment segment = { start, end };

    if (CheckCollisionRecs(recB, recA) || CheckCollisionRecSegment(segment, recB))
    {
      phyA->collision[0] = true;

      // if (phyA->vel.x > 0) phyA->pos.x = recB.x - recA.width;
      // else if (phyA->vel.x < 0) phyA->pos.x = recB.x + recB.width;
      // if (phyA->vel.y > 0) phyA->pos.y = recB.y - recA.height;
      // else if (phyA->vel.y < 0) phyA->pos.y = recB.y + recB.height;

      Rectangle intersect = GetCollisionRec(recA, recB);
      // TraceLog(LOG_INFO, "Collision: A x%d y%d / B x%d y%d", recA.x, recA.y, recB.x, recB.x);
      // Reposition the objects so they no longer intersect
      if (intersect.width > intersect.height)
      {
          // Collision occurred on the top or bottom
          if (recA.y < recB.y) { // Top
            phyA->collision[1] = true;
            phyA->pos.y -= intersect.height;
          }
          else { // Bottom
            phyA->collision[2] = true;
            phyA->pos.y += intersect.height;
          }
      }
      else
      {
          // Collision occurred on the left or right side
          if (recA.x < recB.x) { // Left
            phyA->collision[3] = true;
            phyA->pos.x -= intersect.width;
          }
          else { // Right
            phyA->collision[4] = true;
            phyA->pos.x +=  intersect.width;
          }
      }

      if(phyA->collision[1]) phyA->vel.y *= -1; // Top
      else if(phyA->collision[2]) {
        phyA->vel.y *= -1; // Bottom
        phyA->vel.x *= -1; // Bottom
      }
      else if(phyA->collision[3]) phyA->vel.x *= -1; // Left
      else if(phyA->collision[4]) phyA->vel.x *= -1; // Right
      phyA->vel.x *= 0.8;
      phyA->vel.y *= 0.8;
      
      // phyA->vel.x = -phyA->vel.x;
      // phyA->vel.y = -phyA->vel.y;

      // Collision detected! Get the intersecting area
      // Rectangle intersect = GetCollisionRec(recA, recB);
      // // TraceLog(LOG_INFO, "Collision: A x%d y%d / B x%d y%d", recA.x, recA.y, recB.x, recB.x);
      //   // Reposition the objects so they no longer intersect
      //   if (intersect.width < intersect.height)
      //   {
      //       // Collision occurred on the left or right side
      //       if (recA.x < recB.x) { // Left
      //         phyA->collision[3] = true;
      //         phyA->pos.x -= intersect.width;
      //       }
      //       else { // Right
      //         phyA->collision[4] = true;
      //         phyA->pos.x +=  intersect.width;
      //       } 
      //   }
      //   else
      //   {
      //       // Collision occurred on the top or bottom
      //       if (recA.y < recB.y) { // Top
      //         phyA->collision[2] = true;
      //         phyA->pos.y -= intersect.height;
      //       }
      //       else { // Bottom
      //         phyA->collision[1] = true;
      //         phyA->pos.y += intersect.height;
      //       }
      //   }

    }

    // Rectangle collision = GetCollisionRec(recA, recB);
    // if (collision.width > 0 && collision.height > 0) {
    //   // Handle the collision
    //   printf("Collision detected!\n");
    //   phyA->collision[0] = true;

    //   // Determine the direction of the collision
    //   if (collision.x == recA.x) {
    //     // Collision from the left
    //     printf("Collision from the left\n");
    //     phyA->collision[3] = true;
    //     phyA->pos.x = recB.x + recB.width + 0.1;
    //   } else if (collision.x + collision.width == recA.x + recA.width) {
    //     // Collision from the right
    //     printf("Collision from the right\n");
    //     phyA->collision[4] = true;
    //     phyA->pos.x = recB.x - (0.1 + recA.width);
    //   } else if (collision.y == recA.y) {
    //     // Collision from the top
    //     printf("Collision from the top\n");
    //     phyA->collision[1] = true;
    //     phyA->pos.y = recB.y - (0.1 + recA.height);
    //   } else if (collision.y + collision.height == recA.y + recA.height) {
    //     // Collision from the bottom
    //     printf("Collision from the bottom\n");
    //     phyA->collision[2] = true;
    //     phyA->pos.y = recB.y + recB.height + 0.1;
    //   }
    // }

}

void ResetCollision(Physic *phy) {
  phy->collision[0] = false;
  phy->collision[1] = false;
  phy->collision[2] = false;
  phy->collision[3] = false;
  phy->collision[4] = false;
}