#include "common.h"

std::string monster_name(MonsterType type) {
    switch (type) {
        case MON_SLIME: return "Slime";
        case MON_SKELETON: return "Skeleton";
        case MON_ORC: return "Orc";
        case MON_WARLOCK: return "Warlock";
        case MON_BOSS: return "Boss";
        default: return "Unknown";
    }
}

char monster_symbol(MonsterType type) {
    switch (type) {
        case MON_SLIME: return 's';
        case MON_SKELETON: return 'k';
        case MON_ORC: return 'o';
        case MON_WARLOCK: return 'w';
        case MON_BOSS: return 'B';
        default: return '?';
    }
}

std::string item_name(ItemType type) {
    switch (type) {
        case ITEM_POTION: return "Potion";
        case ITEM_SWORD: return "Sword";
        case ITEM_SHIELD: return "Shield";
        default: return "None";
    }
}
