#ifndef MT_CONTROLLER_H
#define MT_CONTROLLER_H
#include "common.h"
#include "difficulty.h"
#include "player.h"
#include "map.h"
#include "event.h"
#include "entity.h"

class MT_GameController {
public:
    /**
     * @brief Initialize the game controller.
     * @param diff Difficulty enum selected in main.
     * @note This constructor creates initial player and map resources.
     */
    MT_GameController(MT_Difficulty diff);
    /**
     * @brief Run the main game loop.
     * @return 0 for victory, 1 for quit or defeat.
     * @note Lifecycle contract: dynamic resources are released before return.
     */
    int run_loop();
private:
    MT_Difficulty difficulty;
    MT_DifficultyConfig config;
    MT_GameState state;
    MT_Position player_pos;
    MT_Player player;
    char** grid;          // 2D map array, created/destroyed by map module.
    std::mt19937 rng;     // Shared RNG owned by controller and passed to event.

    bool handle_move(char cmd);
    void handle_event_after_move();
    void handle_ambush_combat(const MT_Enemy& enemy);
};
#endif