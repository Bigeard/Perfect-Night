#include "../../../lib/raylib/src/raylib.h"
#include "../../../lib/raylib/src/raymath.h"
#include "stdio.h"

#include "box.h"
#include "../gameplay/gameplay.h"
#include "../physic/physic.h"


void InitBox(void) {

}

void UpdateBox(Box *box) {

}

void DrawBox(Box box) {
    if (activeDev) {
        // Draw Cell Position
        char cellBox[54];
        sprintf(cellBox, "W: %d / H: %d", (int)box.p.size.x, (int)box.p.size.y);
        DrawText(cellBox, box.p.pos.x+1, box.p.pos.y, 12, BLACK);
        char cellBox1[54];
        sprintf(cellBox1, "X: %d / Y: %d", (int)box.p.pos.x, (int)box.p.pos.y);
        DrawText(cellBox1, box.p.pos.x+1, box.p.pos.y+12, 12, BLACK);
    }

    // Draw Box
    Rectangle envBox = {box.p.pos.x, box.p.pos.y, box.p.size.x, box.p.size.y};
    DrawRectangleRec(envBox, Fade(box.color, 0.4));
}
