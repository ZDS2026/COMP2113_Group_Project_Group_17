#ifndef MT_DIFFICULTY_H
#define MT_DIFFICULTY_H
#include "common.h"

struct MT_DifficultyConfig {
    int map_width;
    int map_height;
    float enemy_stat_mult;  // 敌人HP/ATK/DEF缩放系数
    float event_trigger_prob; // 随机事件触发概率(0.0~1.0)
};

/**
 * @brief 根据难度返回内置配置参数
 * @param level 难度枚举
 * @return 包含地图尺寸、敌人倍率、事件概率的配置
 * @note 若未来接入 config_parser，可由外部配置覆盖该默认值
 */
MT_DifficultyConfig mt_diff_get_config(MT_Difficulty level);
#endif