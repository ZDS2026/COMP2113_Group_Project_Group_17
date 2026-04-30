#ifndef MT_RENDERER_H
#define MT_RENDERER_H
#include "common.h"
#include "player.h"

/** @brief 清屏并绘制当前帧（使用 ANSI 转义符） */
void mt_renderer_draw_frame(const char** grid, int w, int h, const MT_Position& pos, const MT_Player& p);
/** @brief 打印顶部状态栏 */
void mt_renderer_draw_status(const MT_Player& p);
#endif