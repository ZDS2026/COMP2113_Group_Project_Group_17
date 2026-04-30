#include "difficulty.h"

MT_DifficultyConfig mt_diff_get_config(MT_Difficulty level) {
    MT_DifficultyConfig cfg{};
    switch (level) {
        case MT_EASY:
            cfg = {10, 10, 1.0f, 0.10f};
            break;
        case MT_MEDIUM:
            cfg = {15, 15, 1.4f, 0.15f};
            break;
        case MT_HARD:
            cfg = {20, 20, 1.8f, 0.25f};
            break;
        default:
            cfg = {10, 10, 1.0f, 0.10f};
            break;
    }
    return cfg;
}