#include "combat.h"
#include <cmath>

MT_CombatResult mt_combat_resolve(const MT_Player& p, const MT_Enemy& e) {
    MT_CombatResult res = {false, 0, 0};
    int p_dmg = std::max(1, p.atk - e.def);
    int e_dmg = std::max(1, e.atk - p.def);
    
    // 玩家先手，计算需要几回合击败敌人
    int rounds_to_kill_e = std::ceil((float)e.hp / p_dmg);
    // 敌人能反击的次数
    int counter_rounds = rounds_to_kill_e - 1; 
    int total_dmg = counter_rounds * e_dmg;
    
    res.damage_taken = total_dmg;
    res.player_won = (p.hp > total_dmg);
    res.rounds = rounds_to_kill_e;
    return res;
}