#include "game_controller.h"
#include "renderer.h"
#include "map.h"
#include "event.h"
#include "file_io.h"
#include <iostream>
#include <limits>

MT_GameController::MT_GameController(MT_Difficulty diff) 
    : difficulty(diff), config(mt_diff_get_config(diff)), state(MT_PLAYING), rng(std::random_device{}()) {
    // 1. 初始化玩家
    mt_player_init(player, 100, 20, 5);
    player_pos = {1, 1};
    // 2. 动态分配地图
    grid = mt_map_create(config.map_width, config.map_height);
    mt_map_generate(grid, config.map_width, config.map_height, diff);
}

int MT_GameController::run_loop() {
    while (state != MT_GAME_OVER && state != MT_WIN) {
        mt_renderer_draw_frame((const char**)grid, config.map_width, config.map_height, player_pos, player);
        mt_renderer_draw_status(player);
        
        std::cout << "\nInput (w/a/s/d): move | v: save | q: quit | i: use item[0]: ";
        char cmd; std::cin >> cmd;
        
        MT_Position next_pos = player_pos;
        if (cmd == 'w') next_pos.row--;
        else if (cmd == 'a') next_pos.col--;
        else if (cmd == 's') next_pos.row++;
        else if (cmd == 'd') next_pos.col++;
        else if (cmd == 'v') {
            if (mt_io_save_game(player, (const char**)grid, config.map_width, config.map_height, player_pos, difficulty, "data/save.dat")) {
                MT_LOG("Game saved to data/save.dat");
            } else {
                MT_LOG("Failed to save game.");
            }
            continue;
        }
        else if (cmd == 'q') break;
        else if (cmd == 'i') { mt_player_use_item(player, 0); continue; }
        else continue;

        if (mt_map_is_walkable((const char**)grid, config.map_width, config.map_height, next_pos.row, next_pos.col)) {
            player_pos = next_pos;
            // 随机事件触发
            MT_EventType evt = mt_event_roll(config.event_trigger_prob, rng);
            if (evt != MT_EVT_NONE) {
                MT_Enemy amb_enemy;
                mt_event_apply(player, evt, difficulty, amb_enemy);
                // 若触发战斗，调用 combat 模块（此处简化为直接结算）
                // 实际可加 MT_COMBAT 状态循环
            }
            // 检查是否到达出口 '>'
            if (grid[player_pos.row][player_pos.col] == '>') state = MT_WIN;
        }
    }
    mt_player_destroy(player);
    mt_map_destroy(grid, config.map_height);
    return (state == MT_WIN) ? 0 : 1;
}