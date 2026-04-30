#ifndef MT_COMBAT_H
#define MT_COMBAT_H
#include "player.h"
#include "entity.h"

struct MT_CombatResult {
    bool player_won;
    int damage_taken;
    int rounds;
};

/**
 * @brief 回合制战斗计算（纯计算函数）
 * @param p 玩家快照（只读）
 * @param e 敌人快照（只读）
 * @return 战斗结果结构体，不直接修改玩家对象
 * @note controller 可依据结果决定是否扣血、掉落、状态切换
 */
MT_CombatResult mt_combat_resolve(const MT_Player& p, const MT_Enemy& e);
#endif