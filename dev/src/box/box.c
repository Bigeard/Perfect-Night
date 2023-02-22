#include "../../../lib/raylib/src/raylib.h"

#include "box.h"
#include "../gameplay/gameplay.h"
#include "../physic/physic.h"
#include "../tool/tool.h"

void InitBox(void) {}

void UpdateBox(Box *box) {}

void DrawBox(Box box)
{
	if (!box.id)
		return;

	DrawRectangle(box.p.pos.x, box.p.pos.y, box.p.size.x, box.p.size.y, GRAY); // or LIGHTGREY what is the best 🤔
	DrawRectangle(box.p.pos.x + 3, box.p.pos.y + 3, box.p.size.x - 6, box.p.size.y - 6, box.color);

	// @FPS take a lot of performance
	DrawTextureTiled(PattenSlashTexture, (Rectangle){0.0f, 0.0f, 16.0f, 16.0f}, (Rectangle){box.p.pos.x + 3.0f, box.p.pos.y + 3.0f, box.p.size.x - 6.0f, box.p.size.y - 6.0f},
					 (Vector2){0.0f, 0.0f}, 0.0f, 1.0f, Fade(box.color, 0.4f));

	// Draw QrCode
	if (box.displayQrCode && numberPlayer != 0)
	{
		DrawTexture(qrCodeTexture, box.p.pos.x + 8.0f, box.p.pos.y + 8.0f, WHITE);
		DrawRectangle(box.p.pos.x + 8.0f, box.p.pos.y + 8.0f, box.p.size.x - 16.0f, box.p.size.y - 16.0f, Fade(BLACK, 0.4f));
	}

	// *** DEV INFO ***
	if (activeDev)
	{
		// Draw Cell Position
		DrawText(TextFormat("W: %d / H: %d", (int)box.p.size.x, (int)box.p.size.y),
				 box.p.pos.x + 1.0f, box.p.pos.y, 12, WHITE);
		DrawText(TextFormat("X: %d / Y: %d", (int)box.p.pos.x, (int)box.p.pos.y),
				 box.p.pos.x + 1, box.p.pos.y + 12.0f, 12.0f, WHITE);
	}
}
