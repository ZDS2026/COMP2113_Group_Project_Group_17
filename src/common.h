#ifndef MT_COMMON_H
#define MT_COMMON_H

#include <iostream>
#include <string>
#include <random>

// 枚举值严格对应难度与游戏状态，禁止修改
enum MT_Difficulty { MT_EASY = 1, MT_MEDIUM = 2, MT_HARD = 3 };
enum MT_GameState { MT_MENU, MT_PLAYING, MT_COMBAT, MT_GAME_OVER, MT_WIN };

// 坐标结构体
struct MT_Position { int row; int col; };

// 统一日志打印宏（避免重复造轮子）
#define MT_LOG(msg) std::cout << "[MT] " << msg << std::endl

#endif // MT_COMMON_H