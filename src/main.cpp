#include <iostream>
#include "game_controller.h"
#include "common.h"

int main() {
    MT_LOG("Select Difficulty: 1-Easy 2-Medium 3-Hard");
    int d = 1;
    std::cin >> d;
    if (!std::cin) {
        MT_LOG("Invalid input, default to EASY.");
        d = 1;
        std::cin.clear();
    }
    MT_Difficulty diff = (d == 1) ? MT_EASY : (d == 2) ? MT_MEDIUM : MT_HARD;
    MT_GameController game(diff);
    return game.run_loop();
}