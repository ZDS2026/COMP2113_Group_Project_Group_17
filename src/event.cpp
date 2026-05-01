#include "event.h"
#include "entity.h"
#include <random>
#include <algorithm> // 用于 std::max

MT_EventType mt_event_roll(float base_prob, std::mt19937& rng) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    // 1. 判定是否触发事件
    if(dist(rng) > base_prob) return MT_EVT_NONE;

    // 2. 权重随机：通过 1-100 的随机数来实现不同事件的概率分布
    // 建议：宝藏(30%)，陷阱(30%)，伏击(30%)，祝福(10%)
    std::uniform_int_distribution<int> weight_dist(1, 100);
    int roll = weight_dist(rng);

    if (roll <= 30) return MT_EVT_TREASURE;
    if (roll <= 60) return MT_EVT_TRAP;
    if (roll <= 90) return MT_EVT_AMBUSH;
    return MT_EVT_BLESSING;
}

void mt_event_apply(MT_Player& p, MT_EventType evt, MT_Difficulty diff, MT_Enemy& out_enemy) {
    // 根据难度设定数值系数
    float intensity = (diff == MT_EASY) ? 0.7f : (diff == MT_MEDIUM) ? 1.0f : 1.5f;

    switch(evt) {
        case MT_EVT_TREASURE:
            // 奖励随难度反向缩放（简单难度奖励更多）
            p.hp += (int)(50 / intensity); 
            // 尝试放入血瓶（ID: 1），逻辑由 Role B 定义
            if(p.inv_count == p.inv_cap) {
                mt_player_expand_inventory(p);
            }
            if(p.inv_count < p.inv_cap) {
                p.inventory[p.inv_count++] = 1; 
            }
            break;

        case MT_EVT_TRAP:
            // 陷阱伤害随难度正向缩放
            {
                int damage = (int)(30 * intensity);
                p.hp = std::max(0, p.hp - damage); // 确保血量不低于0
            }
            break;

        case MT_EVT_AMBUSH:
            // 伏击：生成一个与玩家当前能力匹配的敌人
            // 注意：此处假设 controller 传入的 diff 是正确的
            // 这里的 tier 可以设为 1，或通过某种方式获取当前层数，目前按 1 处理
            out_enemy = mt_entity_spawn_enemy(diff, 1);
            break;

        case MT_EVT_BLESSING:
            // 祝福：永久提升属性
            p.def += (int)(3 * (2.0f - intensity)); // 简单难度祝福更强
            p.atk += (int)(5 * (2.0f - intensity));
            break;

        default: 
            break;
    }
}
