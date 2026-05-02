#ifndef MT2_DIFFICULTY_H
#define MT2_DIFFICULTY_H

#include "common.h"

/**
 * Get static configuration by selected difficulty.
 * Input: difficulty enum.
 * Output: full game config for map, monsters, items, and player stats.
 */
DifficultyConfig get_difficulty_config(DifficultyLevel level);

#endif
