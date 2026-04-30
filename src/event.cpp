#include "event.h"
#include "entity.h"
#include <random>

MT_EventType mt_event_roll(float base_prob, std::mt19937& rng) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    if(dist(rng) > base_prob) return MT_EVT_NONE;
    std::uniform_int_distribution<int> evt_dist(1, 4);
    return (MT_EventType)evt_dist(rng);
}

void mt_event_apply(MT_Player& p, MT_EventType evt, MT_Difficulty diff, MT_Enemy& out_enemy) {
    switch(evt) {
        case MT_EVT_TREASURE:
            p.hp += 40;
            if(p.inv_count == p.inv_cap) mt_player_expand_inventory(p);
            p.inventory[p.inv_count++] = 1; // 血瓶
            break;
        case MT_EVT_TRAP:
            p.hp -= 25;
            break;
        case MT_EVT_AMBUSH:
            out_enemy = mt_entity_spawn_enemy(diff, 1);
            // 此处仅赋值，战斗结算由A调用combat模块完成
            break;
        case MT_EVT_BLESSING:
            p.def += 5;
            break;
        default: break;
    }
}