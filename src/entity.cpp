#include "entity.h"

MT_Enemy mt_entity_spawn_enemy(MT_Difficulty diff, int tier) {
    MT_Enemy e;
    e.symbol = 'E';
    int base_hp = 30, base_atk = 10, base_def = 2;
    float mult = (diff==MT_EASY)?1.0f:(diff==MT_MEDIUM)?1.4f:1.8f;
    e.hp = base_hp * mult; e.atk = base_atk * mult; e.def = base_def * mult;
    return e;
}