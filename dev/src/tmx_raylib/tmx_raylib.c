#include <stdlib.h>
#include <stdio.h>
#include "../../../lib/raylib/src/raylib.h"
#include "../../../lib/tmx/src/tmx.h"

#include "../player/player.h"
#include "../box/box.h"


void *raylib_tex_loader(const char *path) {
	Texture2D *returnValue = malloc(sizeof(Texture2D));
	*returnValue = LoadTexture(path);
	return returnValue;
}

void raylib_free_tex(void *ptr) {
	UnloadTexture(*((Texture2D *) ptr));
	free(ptr);
}

Color int_to_color(int color) {
	tmx_col_bytes res = tmx_col_to_bytes(color);
	return *((Color*)&res);
}

#define LINE_THICKNESS 2.5

void draw_polyline(double offset_x, double offset_y, double **points, int points_count, Color color) {
	int i;
	for (i=1; i<points_count; i++) {
		DrawLineEx((Vector2){offset_x + points[i-1][0], offset_y + points[i-1][1]},
		           (Vector2){offset_x + points[i][0], offset_y + points[i][1]},
		           LINE_THICKNESS, color);
	}
}

void draw_polygon(double offset_x, double offset_y, double **points, int points_count, Color color) {
	draw_polyline(offset_x, offset_y, points, points_count, color);
	if (points_count > 2) {
		DrawLineEx((Vector2){offset_x + points[0][0], offset_y + points[0][1]},
		           (Vector2){offset_x + points[points_count-1][0], offset_y + points[points_count-1][1]},
		           LINE_THICKNESS, color);
	}
}

void draw_objects(tmx_object_group *objgr) {
	tmx_object *head = objgr->head;

	while (head) {
		if (head->visible) {
			tmx_property *prop = tmx_get_property(head->properties, "color");
			Color color = int_to_color(prop->value.color);

			if (head->obj_type == OT_SQUARE) {
				DrawRectangleLinesEx((Rectangle){head->x, head->y, head->width, head->height}, LINE_THICKNESS, color);
			}
			else if (head->obj_type  == OT_POLYGON) {
				draw_polygon(head->x, head->y, head->content.shape->points, head->content.shape->points_len, color);
			}
			else if (head->obj_type == OT_POLYLINE) {
				draw_polyline(head->x, head->y, head->content.shape->points, head->content.shape->points_len, color);
			}
			else if (head->obj_type == OT_ELLIPSE) {
				DrawEllipseLines(head->x + head->width/2.0, head->y + head->height/2.0, head->width/2.0, head->height/2.0, color);
			}
		}
		head = head->next;
	}
}

void draw_image_layer(tmx_image_layer *image) {
	Texture2D *texture = (Texture2D*)image->image;
	DrawTexture(*texture, 0, 0, WHITE);
}

void draw_tile(void *image, unsigned int sx, unsigned int sy, unsigned int sw, unsigned int sh,
               unsigned int dx, unsigned int dy, float opacity, unsigned int flags) {
    Texture2D *texture = (Texture2D*)image;
    int op = 0xFF * opacity;
    DrawTextureRec(*texture, (Rectangle) {sx, sy, sw, sh}, (Vector2) {dx, dy}, (Color) {op, op, op, op});
}

void draw_layer(tmx_map *map, tmx_layer *layer) {
	unsigned long i, j;
	unsigned int gid, x, y, w, h, flags;
	float op;
	tmx_tileset *ts;
	tmx_image *im;
	void* image;
	op = layer->opacity;
	for (i=0; i<map->height; i++) {
		for (j=0; j<map->width; j++) {
			gid = (layer->content.gids[(i*map->width)+j]) & TMX_FLIP_BITS_REMOVAL;
			if (map->tiles[gid] != NULL) {
				ts = map->tiles[gid]->tileset;
				im = map->tiles[gid]->image;
				x  = map->tiles[gid]->ul_x;
				y  = map->tiles[gid]->ul_y;
				w  = ts->tile_width;
				h  = ts->tile_height;
				if (im) {
                    image = im->resource_image;
				}
				else {
                    image = ts->image->resource_image;
				}
				flags = (layer->content.gids[(i*map->width)+j]) & ~TMX_FLIP_BITS_REMOVAL;
                draw_tile(image, x, y, w, h, j*ts->tile_width, i*ts->tile_height, op, flags);
			}
		}
	}
}

void draw_all_layers(tmx_map *map, tmx_layer *layers) {
	while (layers) {
		if (layers->visible) {

			if (layers->type == L_GROUP) {
				draw_all_layers(map, layers->content.group_head); // recursive call
			}
			else if (layers->type == L_OBJGR) {
				draw_objects(layers->content.objgr);
			}
			else if (layers->type == L_IMAGE) {
				draw_image_layer(layers->content.image_layer);
			}
			else if (layers->type == L_LAYER) {
				draw_layer(map, layers);
			}
		}
		layers = layers->next;
	}
}

void render_map(tmx_map *map) {
	ClearBackground(int_to_color(map->backgroundcolor));
	draw_all_layers(map, map->ly_head);
}

void tmx_init_object(tmx_layer *layer, Player *players, Box *boxes) {
	int box_number = 0;
	while (layer) {
        if (layer->visible && layer->type == L_OBJGR) {
	        tmx_object *head = layer->content.objgr->head;
	        while (head) {
                if (head->visible && head->obj_type == OT_SQUARE && layer->id == 3) {
					tmx_property *id = tmx_get_property(head->properties, "player_id");
                    players[id->value.integer-1].spawn = (Vector2){ head->x, head->y };
                    players[id->value.integer-1].p.pos = (Vector2){ head->x, head->y };
					
					tmx_property *color = tmx_get_property(head->properties, "color");
					Color color_player = int_to_color(color->value.color);
					players[id->value.integer-1].COLORS[0] = color_player;
					players[id->value.integer-1].COLORS[1] = color_player;
					players[id->value.integer-1].COLORS[2] = color_player;
                }
                if (head->visible && head->obj_type == OT_SQUARE && layer->id == 4) {
					boxes[box_number] = (Box) {head->id, {{head->x, head->y}, {head->width, head->height}, {0, 0}}, GRAY };
					box_number++;
				}
        		head = head->next;
            }
        }
		layer = layer->next;
    }
}
