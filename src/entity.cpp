#include "entity.h"

/**
 * @brief 根据难度与层级生成敌人属性
 * 属性公式：(基础值 + 层级成长) * 难度系数
 */
MT_Enemy mt_entity_spawn_enemy(MT_Difficulty diff, int tier) {
    MT_Enemy e;
    
    // 1. 基础属性定义
    // tier 从 1 开始，随着层级增加，怪物的属性会线性提升
    int base_hp = 30 + (tier * 15);   // 每升一层增加 15 血量
    int base_atk = 10 + (tier * 4);   // 每升一层增加 4 攻击
    int base_def = 2 + (tier * 2);    // 每升一层增加 2 防御

    // 2. 判定是否为 Boss (假设每 5 层一个 Boss)
    if (tier > 0 && tier % 5 == 0) {
        e.symbol = 'B';
        base_hp *= 2;    // Boss 血量翻倍
        base_atk += 5;   // Boss 额外攻击补偿
    } else {
        e.symbol = 'E';
    }

    // 3. 应用难度系数
    // 这里的 MT_EASY, MT_MEDIUM, MT_HARD 对应 common.h 中的定义
    float mult = 1.0f;
    if (diff == MT_EASY) {
        mult = 0.8f;   // 简单模式削弱
    } else if (diff == MT_MEDIUM) {
        mult = 1.2f;   // 普通模式标准
    } else {
        mult = 1.6f;   // 困难模式增强
    }

    // 4. 最终赋值（使用 int 强制转换，确保数值稳定）
    e.hp  = (int)(base_hp * mult);
    e.atk = (int)(base_atk * mult);
    e.def = (int)(base_def * mult);

    return e;
}
