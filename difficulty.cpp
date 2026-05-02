#include "difficulty.h"

DifficultyConfig get_difficulty_config(DifficultyLevel level) {
    DifficultyConfig cfg{};
    switch (level) {
        case DIFF_EASY:
            cfg = {11, 11, 8, 5, 2, 2, 140, 24, 8, 0.9f, 0.9f, 3};
            break;
        case DIFF_MEDIUM:
            cfg = {15, 15, 16, 6, 3, 3, 120, 22, 6, 1.1f, 1.1f, 4};
            break;
        case DIFF_HARD:
            cfg = {21, 21, 30, 7, 4, 4, 110, 20, 5, 1.3f, 1.3f, 5};
            break;
        default:
            cfg = {11, 11, 8, 5, 2, 2, 140, 24, 8, 0.9f, 0.9f, 3};
            break;
    }
    return cfg;
}
