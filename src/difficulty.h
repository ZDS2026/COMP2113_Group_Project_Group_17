#ifndef MT_DIFFICULTY_H
#define MT_DIFFICULTY_H
#include "common.h"

struct MT_DifficultyConfig {
    int map_width;
    int map_height;
    float enemy_stat_mult;    // Enemy HP/ATK/DEF scaling factor.
    float event_trigger_prob; // Random event trigger probability (0.0~1.0).
};

/**
 * @brief Validate whether a difficulty config is usable.
 * @param cfg Config to validate.
 * @return true if valid; false if fallback should be used.
 */
bool mt_diff_is_valid(const MT_DifficultyConfig& cfg);

/**
 * @brief Get built-in config by difficulty.
 * @param level Difficulty enum.
 * @return Config with map size, enemy scaling, and event probability.
 * @note Can be overridden by external config in future integration.
 */
MT_DifficultyConfig mt_diff_get_config(MT_Difficulty level);
#endif