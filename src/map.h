#ifndef MT_MAP_H
#define MT_MAP_H
#include "common.h"

/**
 * @brief 动态分配二维字符数组地图
 * @return char**，行优先布局，尺寸为 [height][width]
 * @note 调用方需配对调用 mt_map_destroy 释放
 */
char** mt_map_create(int width, int height);

/**
 * @brief 释放动态地图内存
 * @param grid [in,out] 传引用便于内部置空，避免悬空指针
 */
void mt_map_destroy(char**& grid, int height);

/**
 * @brief 根据难度生成基础墙体/地板/出口
 * @note 本函数只负责“基础地形”，敌人/道具/事件由其他模块追加
 */
void mt_map_generate(char** grid, int w, int h, MT_Difficulty diff);

/**
 * @brief 检测坐标是否可移动（非墙且未越界）
 * @return true 可移动，false 不可移动
 */
bool mt_map_is_walkable(const char** grid, int w, int h, int r, int c);
#endif