#ifndef __TILEMAP__
#define __TILEMAP__

#include <SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include "gf2d_graphics.h"
#include "gf2d_sprite.h"
#include "simple_logger.h"
#include "ds_priority_queue.h"
#include "shape.h"

//one tile...trying something. currently not in use
typedef struct tile_s 
{
	unsigned int id;
	unsigned int x;
	unsigned int y;
	int frame;
	Rect *boundingBox;
}Tile;

typedef struct tilemap_s 
{
	int tileWidth;
	int tileHeight;
	int tilesPerLine;
	Sprite *tilemapSprite;
	//const LL_Node *tiles;
	//PriorityQueue *tiles_head;
	//PriorityQueue *tiles_tail;
	int tiles[512];
	int space[512];
	unsigned int width;
	unsigned int height;
	int xPos;						/**<Starting x position of tilemap*/
	int yPos;						/**<Starting y position of tilemap*/
	Rect *boundingBox;
}TileMap;

/**
 * @brief Creates a tilemap struct
 * @returns Pointer to a new tilemap
 */
TileMap * tilemap_init();

/**
 * @brief Inits a tilemap from a file
 * @param file The file to read from
 * @param neW_tilemap The TileMap that will be inited
 */
void tilemap_load_from_file(FILE * file, TileMap * new_tilemap);

/**
 * @brief Draws a tile map to the screen
 * @param tilemap The sprite containing the tilemap information
 * @param tiles Int array signifying which tile to load on which spot
 * @param width The number of tiles to draw in a line
 * @param height The number of lines of tiles to draw
 * @param xPos Where to starting drawing the tilemap
 * @param yPos Where to starting drawing the tilemap
 * @return 0 if successful
 */
int tilemap_draw(Sprite *tilemap, const int *tiles, unsigned int width, unsigned int height, int xPos, int yPos);

/**
 * @brief Draws a tile map from the data in a tilemap struct
 * @param tilemap Struct to draw from
 * @returns 0 if successful, -1 if tilemap is NULL
 */
int tilemap_draw_from_data(TileMap * tilemap);

int tilemap_find_tile(int mx, int my, TileMap * map);
void tilemap_clear(TileMap * map);

#endif // !__TILEMAP__
