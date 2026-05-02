#ifndef MT2_RENDERER_H
#define MT2_RENDERER_H

#include "common.h"
#include <deque>
#include <string>

struct RenderTargetInfo {
    bool has_target;
    std::string name;
    int hp;
    int atk;
    int effective_atk;
};

/**
 * Render map, right-side status panel, and bottom logs.
 * Inputs: map, entities, dimensions, and recent log lines.
 * Output: drawn frame to terminal.
 */
void draw_frame(
    char** map,
    int width,
    int height,
    const Player& player,
    const Monster* monsters,
    int monster_count,
    const Item* items,
    int item_count,
    const RenderTargetInfo& target_info,
    const std::deque<std::string>& logs,
    int floor_index,
    int floor_total
);

#endif
