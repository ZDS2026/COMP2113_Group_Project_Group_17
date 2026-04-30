#ifndef MT_FILEIO_H
#define MT_FILEIO_H
#include "common.h"
#include "player.h"
#include "map.h"

/**
 * @brief 存档：保存玩家、地图、难度、坐标到文本
 * @param p 玩家快照（只读）
 * @param grid 地图网格（只读）
 * @param w 地图宽度
 * @param h 地图高度
 * @param pos 玩家当前位置
 * @param diff 当前难度
 * @param path 存档文件路径（如 data/save.dat）
 * @return true=写入成功，false=打开文件失败
 */
bool mt_io_save_game(const MT_Player& p, const char** grid, int w, int h, const MT_Position& pos, MT_Difficulty diff, const std::string& path);

/**
 * @brief 读档：加载并重建动态地图内存，恢复状态
 * @param p [in,out] 玩家对象（会覆盖属性与背包）
 * @param grid [in,out] 旧地图将被释放，并重建新地图
 * @param w [out] 地图宽度
 * @param h [out] 地图高度
 * @param pos [out] 玩家坐标
 * @param diff [out] 难度
 * @param path 存档文件路径
 * @return true=读取成功，false=文件不存在或不可读
 */
bool mt_io_load_game(MT_Player& p, char**& grid, int& w, int& h, MT_Position& pos, MT_Difficulty& diff, const std::string& path);
#endif