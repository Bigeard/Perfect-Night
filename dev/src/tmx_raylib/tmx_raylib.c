#include <stdlib.h>
#include <stdio.h>
#include "../../../lib/raylib/src/raylib.h"
#include "../../../lib/tmx/src/tmx.h"

#include "../gameplay/gameplay.h"
#include "../player/player.h"
#include "../box/box.h"
#include "../loot/loot.h"

void *raylib_tex_loader(const char *path)
{
	Texture2D *returnValue = malloc(sizeof(Texture2D));
	*returnValue = LoadTexture(path);
	return returnValue;
}

void raylib_free_tex(void *ptr)
{
	UnloadTexture(*((Texture2D *)ptr));
	free(ptr);
}

Color int_to_color(int color)
{
	tmx_col_bytes res = tmx_col_to_bytes(color);
	return *((Color *)&res);
}

#define LINE_THICKNESS 2.5

void draw_polyline(double offset_x, double offset_y, double **points, int points_count, Color color)
{
	int i;
	for (i = 1; i < points_count; i++)
	{
		DrawLineEx((Vector2){offset_x + points[i - 1][0], offset_y + points[i - 1][1]},
				   (Vector2){offset_x + points[i][0], offset_y + points[i][1]},
				   LINE_THICKNESS, color);
	}
}

void draw_polygon(double offset_x, double offset_y, double **points, int points_count, Color color)
{
	draw_polyline(offset_x, offset_y, points, points_count, color);
	if (points_count > 2)
	{
		DrawLineEx((Vector2){offset_x + points[0][0], offset_y + points[0][1]},
				   (Vector2){offset_x + points[points_count - 1][0], offset_y + points[points_count - 1][1]},
				   LINE_THICKNESS, color);
	}
}

void draw_objects(tmx_object_group *objgr)
{
	tmx_object *head = objgr->head;

	while (head)
	{
		if (head->visible)
		{
			tmx_property *propColor = tmx_get_property(head->properties, "color");
			Color color = int_to_color(propColor->value.color);

			if (head->obj_type == OT_SQUARE)
			{
				DrawRectangleLinesEx((Rectangle){head->x, head->y, head->width, head->height}, LINE_THICKNESS, color);
			}
			else if (head->obj_type == OT_POLYGON)
			{
				draw_polygon(head->x, head->y, head->content.shape->points, head->content.shape->points_len, color);
			}
			else if (head->obj_type == OT_POLYLINE)
			{
				draw_polyline(head->x, head->y, head->content.shape->points, head->content.shape->points_len, color);
			}
			else if (head->obj_type == OT_ELLIPSE)
			{
				DrawEllipseLines(head->x + head->width / 2.0, head->y + head->height / 2.0, head->width / 2.0, head->height / 2.0, color);
			}
		}
		head = head->next;
	}
}

void draw_image_layer(tmx_image_layer *image)
{
	Texture2D *texture = (Texture2D *)image->image;
	DrawTexture(*texture, 0, 0, WHITE);
}

void draw_tile(void *image, unsigned int sx, unsigned int sy, unsigned int sw, unsigned int sh,
			   unsigned int dx, unsigned int dy, float opacity, unsigned int flags)
{
	Texture2D *texture = (Texture2D *)image;
	int op = 0xFF * opacity;
	DrawTextureRec(*texture, (Rectangle){sx, sy, sw, sh}, (Vector2){dx, dy}, (Color){op, op, op, op});
}

void draw_layer(tmx_map *map, tmx_layer *layer)
{
	unsigned long i, j;
	unsigned int gid, x, y, w, h, flags;
	float op;
	tmx_tileset *ts;
	tmx_image *im;
	void *image;
	op = layer->opacity;
	for (i = 0; i < map->height; i++)
	{
		for (j = 0; j < map->width; j++)
		{
			gid = (layer->content.gids[(i * map->width) + j]) & TMX_FLIP_BITS_REMOVAL;
			if (map->tiles[gid] != NULL)
			{
				ts = map->tiles[gid]->tileset;
				im = map->tiles[gid]->image;
				x = map->tiles[gid]->ul_x;
				y = map->tiles[gid]->ul_y;
				w = ts->tile_width;
				h = ts->tile_height;
				if (im)
				{
					image = im->resource_image;
				}
				else
				{
					image = ts->image->resource_image;
				}
				flags = (layer->content.gids[(i * map->width) + j]) & ~TMX_FLIP_BITS_REMOVAL;
				draw_tile(image, x, y, w, h, j * ts->tile_width, i * ts->tile_height, op, flags);
			}
		}
	}
}

void draw_all_layers(tmx_map *map, tmx_layer *layers)
{
	while (layers)
	{
		if (layers->visible)
		{

			if (layers->type == L_GROUP)
			{
				draw_all_layers(map, layers->content.group_head); // recursive call
			}
			// else if (layers->type == L_OBJGR) {
			// 	draw_objects(layers->content.objgr);
			// }
			else if (layers->type == L_IMAGE)
			{
				draw_image_layer(layers->content.image_layer);
			}
			else if (layers->type == L_LAYER)
			{
				draw_layer(map, layers);
			}
		}
		layers = layers->next;
	}
}

void render_map(tmx_map *map)
{
	draw_all_layers(map, map->ly_head);
}

void tmx_init_object(tmx_layer *layer, Player *players, Box *boxes, Loot *loots)
{
	int box_number = 0;
	int loot_number = 0;
	while (layer)
	{
		if (layer->visible && layer->type == L_OBJGR)
		{
			tmx_object *head = layer->content.objgr->head;
			while (head)
			{
				if (head->visible && head->obj_type == OT_SQUARE && layer->id == 3)
				{
					tmx_property *id = tmx_get_property(head->properties, "playerId");
					players[id->value.integer - 1].spawn.x = (float)head->x;
					players[id->value.integer - 1].spawn.y = (float)head->y;
					players[id->value.integer - 1].p.pos.x = (float)head->x;
					players[id->value.integer - 1].p.pos.y = (float)head->y;

					tmx_property *color = tmx_get_property(head->properties, "color");
					Color playerColor = int_to_color(color->value.color);
					players[id->value.integer - 1].color = playerColor;
				}
				if (head->visible && head->obj_type == OT_SQUARE && layer->id == 4)
				{
					tmx_property *displayQrCode = tmx_get_property(head->properties, "displayQrCode");
					tmx_property *collision = tmx_get_property(head->properties, "collision");
					tmx_property *color = tmx_get_property(head->properties, "color");
					tmx_property *score = tmx_get_property(head->properties, "score");
					Color boxColor = (Color){33, 37, 48, 255}; // Default box color
					if (color)
					{
						boxColor = int_to_color(color->value.color);
					}
					int boxScore = -1;
					if (score) {
						boxScore = score->value.integer;
						BoxesScoreSize[boxScore].x = (int)head->width;
						BoxesScoreSize[boxScore].y = (int)head->height;
					}
					boxes[box_number] = (Box){
						head->id,
						{{(float)head->x, (float)head->y}, {(float)head->width, (float)head->height}, {0, 0}},
						boxColor,
						displayQrCode->value.boolean,
						boxScore,
						collision->value.boolean};

					box_number++;
				}
				if (head->visible && head->obj_type == OT_SQUARE && layer->id == 5)
				{
					tmx_property *type = tmx_get_property(head->properties, "type");
					tmx_property *delay = tmx_get_property(head->properties, "delay");
					loots[loot_number] = (Loot){
						head->id,
						{{(float)head->x, (float)head->y}, {(float)head->width, (float)head->height}, {0, 0}},
						true,
						type->value.integer,
						0.0,
						delay->value.decimal,
					};
					loot_number++;
				}
				head = head->next;
			}
		}
		layer = layer->next;
	}
}
