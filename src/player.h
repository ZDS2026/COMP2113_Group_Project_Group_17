#ifndef MT_PLAYER_H
#define MT_PLAYER_H
#include "common.h"

struct MT_Player {
    int hp;
    int max_hp;
    int atk;
    int def;
    int keys;
    int* inventory;      // 动态数组：存放物品ID
    int inv_count;       // 当前物品数
    int inv_cap;         // 当前容量
};

/**
 * @brief 初始化玩家属性与动态背包
 * @note 该函数会分配 inventory 动态内存；调用方必须配对调用 destroy
 */
void mt_player_init(MT_Player& p, int start_hp, int start_atk, int start_def);

/**
 * @brief 释放背包动态内存
 * @note 可安全重复调用（当 inventory 已置空时无副作用）
 */
void mt_player_destroy(MT_Player& p);

/**
 * @brief 背包扩容至 2 倍
 * @return true 表示扩容完成
 */
bool mt_player_expand_inventory(MT_Player& p);

/**
 * @brief 使用背包中指定索引的物品
 * @param inv_index 背包下标（0-based）
 * @note 当前版本只实现“应用效果”，物品删除逻辑留给后续成员完善
 */
void mt_player_use_item(MT_Player& p, int inv_index);
#endif