#ifndef MT_ENTITY_H
#define MT_ENTITY_H
#include "common.h"

struct MT_Enemy {
    int hp;
    int atk;
    int def;
    char symbol; // 'E'=普通, 'B'=Boss
};

/** @brief 根据难度与层级生成敌人属性 */
MT_Enemy mt_entity_spawn_enemy(MT_Difficulty diff, int tier);
#endif