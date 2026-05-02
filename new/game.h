#ifndef MT2_GAME_H
#define MT2_GAME_H

#include "common.h"
#include "renderer.h"
#include <deque>
#include <string>
#include <vector>

class Game {
public:
    Game();
    ~Game();

    /**
     * Initialize a new game session by selected difficulty.
     * Input: difficulty level.
     * Output: true if initialization succeeds.
     */
    bool start_new(DifficultyLevel level);

    /**
     * Load a saved session from slot 1..5 (saveN.dat). Rebuilds pathfinding cache.
     */
    bool load_saved(int slot);

    /**
     * Run interactive game loop until win/lose/quit.
     * Input: none.
     * Output: process exit code style result (0 win, 1 otherwise).
     */
    int run();

private:
    DifficultyLevel level;
    DifficultyConfig cfg;
    Player player;

    char** map;
    int width;
    int height;

    Monster* monsters;
    int monster_count;

    Item* items;
    int item_count;

    bool running;
    bool win;
    bool suppress_first_run_draw_;
    int invalid_input_streak;
    int last_route_count;
    int last_balance_winrate;
    std::deque<std::string> logs;
    RenderTargetInfo target_info;
    std::vector<Position> guaranteed_path;

    void cleanup_dynamic_memory();
    void paint_full_frame();
    void add_log(const std::string& msg);
    void refresh_target_panel();
    bool handle_input(char cmd);
    bool try_move(int dr, int dc);
    void place_entities();
    bool rebuild_guaranteed_path();
    int count_shortest_routes_cap(int cap) const;
    void ensure_boss_beatable();
    bool simulate_guaranteed_path_clear() const;
    int estimate_balance_winrate(int samples) const;
    void show_help_screen() const;
    std::string slot_path(int slot) const;
    int ask_slot_from_user(const std::string& action) const;
    void apply_item_effect(int item_idx);
    void resolve_combat(int monster_idx);
    int find_monster_at(int r, int c) const;
    int find_item_at(int r, int c) const;
    bool is_goal_defeated() const;

    bool cell_blocked_for_monster(int r, int c, int self_idx) const;
    void roll_skeleton_patrol(int idx);
    void ensure_skeleton_patrols();
    void tick_monsters();
};

#endif
