#include "game_controller.h"
#include "renderer.h"
#include "map.h"
#include "event.h"
#include "file_io.h"
#include "combat.h"
#include <iostream>
#include <limits>

MT_GameController::MT_GameController(MT_Difficulty diff) 
    : difficulty(diff), config(mt_diff_get_config(diff)), state(MT_PLAYING), rng(std::random_device{}()) {
    // 1) Initialize player state
    mt_player_init(player, 100, 20, 5);
    player_pos = {1, 1};
    // 2) Allocate map dynamically
    grid = mt_map_create(config.map_width, config.map_height);
    mt_map_generate(grid, config.map_width, config.map_height, diff);
}

int MT_GameController::run_loop() {
    while (state != MT_GAME_OVER && state != MT_WIN) {
        mt_renderer_draw_frame((const char**)grid, config.map_width, config.map_height, player_pos, player);
        mt_renderer_draw_status(player);
        
        std::cout << "\nInput (w/a/s/d): move | v: save | l: load | q: quit | i: use item[0]: ";
        char cmd; std::cin >> cmd;

        if (cmd == 'v') {
            if (mt_io_save_game(player, (const char**)grid, config.map_width, config.map_height, player_pos, difficulty, "data/save.dat")) {
                MT_LOG("Game saved to data/save.dat");
            } else {
                MT_LOG("Failed to save game.");
            }
            continue;
        } else if (cmd == 'l') {
            int loaded_w = config.map_width;
            int loaded_h = config.map_height;
            MT_Difficulty loaded_diff = difficulty;
            if (mt_io_load_game(player, grid, loaded_w, loaded_h, player_pos, loaded_diff, "data/save.dat")) {
                difficulty = loaded_diff;
                config = mt_diff_get_config(difficulty);
                config.map_width = loaded_w;
                config.map_height = loaded_h;
                MT_LOG("Game loaded from data/save.dat");
            } else {
                MT_LOG("Failed to load game.");
            }
            continue;
        }
        else if (cmd == 'q') {
            state = MT_GAME_OVER;
            break;
        } else if (cmd == 'i') {
            mt_player_use_item(player, 0);
            continue;
        } else if (!handle_move(cmd)) {
            continue;
        }

        handle_event_after_move();
        if (player.hp <= 0) {
            state = MT_GAME_OVER;
        }
    }

    mt_player_destroy(player);
    mt_map_destroy(grid, config.map_height);
    return (state == MT_WIN) ? 0 : 1;
}

bool MT_GameController::handle_move(char cmd) {
    MT_Position next_pos = player_pos;
    if (cmd == 'w') next_pos.row--;
    else if (cmd == 'a') next_pos.col--;
    else if (cmd == 's') next_pos.row++;
    else if (cmd == 'd') next_pos.col++;
    else return false;

    if (mt_map_is_walkable((const char**)grid, config.map_width, config.map_height, next_pos.row, next_pos.col)) {
        player_pos = next_pos;
    }
    return true;
}

void MT_GameController::handle_event_after_move() {
    // Check whether player reached the exit '>'
    if (grid[player_pos.row][player_pos.col] == '>') {
        state = MT_WIN;
        return;
    }

    MT_EventType evt = mt_event_roll(config.event_trigger_prob, rng);
    if (evt == MT_EVT_NONE) return;

    MT_Enemy amb_enemy{};
    mt_event_apply(player, evt, difficulty, amb_enemy);
    if (evt == MT_EVT_AMBUSH) {
        handle_ambush_combat(amb_enemy);
    }
}

void MT_GameController::handle_ambush_combat(const MT_Enemy& enemy) {
    state = MT_COMBAT;
    MT_CombatResult result = mt_combat_resolve(player, enemy);
    player.hp -= result.damage_taken;
    if (player.hp <= 0 || !result.player_won) {
        player.hp = 0;
        state = MT_GAME_OVER;
        MT_LOG("You were defeated in ambush combat.");
        return;
    }
    MT_LOG("Ambush defeated. Damage taken: " << result.damage_taken);
    state = MT_PLAYING;
}