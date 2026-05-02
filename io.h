#ifndef MT2_IO_H
#define MT2_IO_H

#include "common.h"
#include <string>

/**
 * Save full game state to file.
 * Input: all runtime state fields and output path.
 * Output: true on success, false on failure.
 */
bool save_game(
    const std::string& path,
    DifficultyLevel level,
    const Player& player,
    char** map,
    int width,
    int height,
    const Monster* monsters,
    int monster_count,
    const Item* items,
    int item_count,
    int current_floor,
    int total_floors
);

/**
 * Load full game state from file.
 * Input: path and mutable outputs.
 * Output: true on success and all outputs filled.
 */
bool load_game(
    const std::string& path,
    DifficultyLevel& level,
    Player& player,
    char**& map,
    int& width,
    int& height,
    Monster*& monsters,
    int& monster_count,
    Item*& items,
    int& item_count,
    int& current_floor,
    int& total_floors
);

#endif
