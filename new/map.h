#ifndef MT2_MAP_H
#define MT2_MAP_H

#include "common.h"

/**
 * Allocate a dynamic 2D map grid.
 * Input: width, height.
 * Output: allocated char** with all rows.
 */
char** create_map(int width, int height);

/**
 * Release dynamic 2D map grid memory.
 * Input: map and height.
 * Output: map pointer set to nullptr.
 */
void destroy_map(char**& map, int height);

/**
 * Generate a maze-like map with a guaranteed path from start to goal.
 * Input: map pointer, width, height, and random seed source.
 * Output: map filled with floor '.' and walls '#'.
 */
void generate_map_with_path(char** map, int width, int height);

/**
 * Check whether a tile can be entered by player.
 * Input: map, dimensions, target row/col.
 * Output: true if tile is in range and not a wall.
 */
bool is_walkable(char** map, int width, int height, int r, int c);

#endif
