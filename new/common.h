#ifndef MT2_COMMON_H
#define MT2_COMMON_H

#include <string>

enum DifficultyLevel {
    DIFF_EASY = 1,
    DIFF_MEDIUM = 2,
    DIFF_HARD = 3
};

enum ItemType {
    ITEM_NONE = 0,
    ITEM_POTION = 1,
    ITEM_SWORD = 2,
    ITEM_SHIELD = 3
};

enum MonsterType {
    MON_SLIME = 0,
    MON_SKELETON = 1,
    MON_ORC = 2,
    MON_WARLOCK = 3,
    MON_BOSS = 4
};

struct Position {
    int r;
    int c;
};

struct Player {
    int hp;
    int max_hp;
    int atk;
    int def;
    Position pos;
};

struct Monster {
    MonsterType type;
    Position pos;
    int hp;
    int atk;
    int def;
    bool alive;
    /** Skeleton patrol step; other types may leave (0,0). */
    int patrol_dr;
    int patrol_dc;
};

struct Item {
    ItemType type;
    Position pos;
    bool active;
};

struct DifficultyConfig {
    int width;
    int height;
    int monster_count;
    int potion_count;
    int sword_count;
    int shield_count;
    int player_start_hp;
    int player_start_atk;
    int player_start_def;
    float monster_atk_scale;
    float monster_hp_scale;
};

std::string monster_name(MonsterType type);
char monster_symbol(MonsterType type);
std::string item_name(ItemType type);

#endif
