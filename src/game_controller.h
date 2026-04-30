#ifndef MT_CONTROLLER_H
#define MT_CONTROLLER_H
#include "common.h"
#include "difficulty.h"
#include "player.h"
#include "map.h"
#include "event.h"

class MT_GameController {
public:
    /**
     * @brief 初始化游戏主控器
     * @param diff 由 main 传入的难度枚举
     * @note 本构造函数负责创建玩家与地图初始资源
     */
    MT_GameController(MT_Difficulty diff);
    /**
     * @brief 运行主循环
     * @return 0 表示通关，1 表示中途退出或失败
     * @note 生命周期约定：run_loop 结束前会释放 player/grid 的动态内存
     */
    int run_loop();
private:
    MT_Difficulty difficulty;
    MT_DifficultyConfig config;
    MT_GameState state;
    MT_Position player_pos;
    MT_Player player;
    char** grid;          // 地图二维数组；由 map 创建/销毁
    std::mt19937 rng;     // 随机数引擎；由 controller 持有并传给 event
};
#endif