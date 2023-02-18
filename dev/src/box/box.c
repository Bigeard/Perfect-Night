#include "../../../lib/raylib/src/raylib.h"

#include "box.h"
#include "../gameplay/gameplay.h"
#include "../physic/physic.h"

void InitBox(void) {}

void UpdateBox(Box *box) {}

void DrawBox(Box box)
{
	// Draw Box
	Rectangle envBox = {box.p.pos.x, box.p.pos.y, box.p.size.x, box.p.size.y};
	DrawRectangleRec(envBox, Fade(box.color, 0.5f));

	// Draw QrCode
	if (box.displayQrCode && numberPlayer != 0)
	{
		DrawTexture(qrCodeTexture, box.p.pos.x + 8.0f, box.p.pos.y + 8.0f, WHITE);
	}

	// *** DEV INFO ***
	if (activeDev)
	{
		// Draw Cell Position
		DrawText(TextFormat("W: %d / H: %d", (int)box.p.size.x, (int)box.p.size.y),
				 box.p.pos.x + 1.0f, box.p.pos.y, 12, BLACK);
		DrawText(TextFormat("X: %d / Y: %d", (int)box.p.pos.x, (int)box.p.pos.y),
				 box.p.pos.x + 1, box.p.pos.y + 12.0f, 12.0f, BLACK);
	}
}
