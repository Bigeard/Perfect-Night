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

	if (box.collision)
	{
		// float time = GetTime();
		// BeginShaderMode(neonShader);
		DrawRectangle(box.p.pos.x, box.p.pos.y, box.p.size.x, box.p.size.y, (Color){143, 151, 160, 255}); // or LIGHTGREY what is the best 🤔
		DrawRectangle(box.p.pos.x + 3, box.p.pos.y + 3, box.p.size.x - 6, box.p.size.y - 6, box.color);
		// EndShaderMode();

		// @FPS take a lot of performance
		if (!activePerf)
		{
			DrawTextureTiled(PattenSlashTexture, (Rectangle){0.0f, 0.0f, 16.0f, 16.0f}, (Rectangle){box.p.pos.x + 3.0f, box.p.pos.y + 3.0f, box.p.size.x - 6.0f, box.p.size.y - 6.0f},
							 (Vector2){0.0f, 0.0f}, 0.0f, 1.0f, Fade(box.color, 0.4f));
		}
	}

	if (box.score != -1 && colorScore[box.score] != -1)
	{
		const char *text = TextFormat("%d", colorScore[box.score]);
		DrawRectangleLinesEx((Rectangle){box.p.pos.x, box.p.pos.y, box.p.size.x, box.p.size.y}, 2.5f, Fade(themeColor[box.score], 0.4f));
		DrawText(
			text,
			box.p.pos.x + box.p.size.x / 2.0f - MeasureText(text, BoxesScoreFontSize[box.score]) / 2.0f,
			box.p.pos.y + box.p.size.y / 2.0f - BoxesScoreFontSize[box.score] / 2.0f + MeasureText(text, BoxesScoreFontSize[box.score]) * 0.1,
			BoxesScoreFontSize[box.score],
			Fade(themeColor[box.score], 0.4f));
	}

	// Draw QrCode
	if (box.displayQrCode)
	{
		float margin = 0;
		if (box.collision)
		{
			margin = 10;
		}
		DrawRectangleRounded(
			(Rectangle){
				box.p.pos.x + 1 + margin,
				box.p.pos.y + margin,
				box.p.size.x - 2 - margin * 2,
				box.p.size.y - 2 - margin * 2},
			0.08f,
			1.0f,
			(Color){116, 116, 116, 255});
		DrawTexture(qrCodeTexture, box.p.pos.x + 8.0f + 1, box.p.pos.y + 8.0f, WHITE);
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
