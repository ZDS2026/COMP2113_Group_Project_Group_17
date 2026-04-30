#include "difficulty.h"

namespace {
constexpr MT_DifficultyConfig kEasyCfg{10, 10, 1.0f, 0.10f};
constexpr MT_DifficultyConfig kMediumCfg{15, 15, 1.4f, 0.15f};
constexpr MT_DifficultyConfig kHardCfg{20, 20, 1.8f, 0.25f};
constexpr int kMinMapSize = 5;
} // namespace

bool mt_diff_is_valid(const MT_DifficultyConfig& cfg) {
    if (cfg.map_width < kMinMapSize || cfg.map_height < kMinMapSize) return false;
    if (cfg.enemy_stat_mult <= 0.0f) return false;
    if (cfg.event_trigger_prob < 0.0f || cfg.event_trigger_prob > 1.0f) return false;
    return true;
}

MT_DifficultyConfig mt_diff_get_config(MT_Difficulty level) {
    MT_DifficultyConfig cfg = kEasyCfg;
    switch (level) {
        case MT_EASY:
            cfg = kEasyCfg;
            break;
        case MT_MEDIUM:
            cfg = kMediumCfg;
            break;
        case MT_HARD:
            cfg = kHardCfg;
            break;
        default:
            cfg = kEasyCfg;
            break;
    }
    if (!mt_diff_is_valid(cfg)) return kEasyCfg;
    return cfg;
}