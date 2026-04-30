#include "renderer.h"
#include <iostream>

void mt_renderer_draw_frame(const char** grid, int w, int h, const MT_Position& pos, const MT_Player& p) {
    (void)p;
    std::cout << "\033[H\033[2J"; // clear screen
    for (int r = 0; r < h; ++r) {
        for (int c = 0; c < w; ++c) {
            if (r == pos.row && c == pos.col) {
                std::cout << "\033[32m@\033[0m";
            } else {
                char ch = grid[r][c];
                if (ch == '#') std::cout << "\033[37m#\033[0m";
                else if (ch == '>') std::cout << "\033[33m>\033[0m";
                else std::cout << ch;
            }
        }
        std::cout << "\n";
    }
}

void mt_renderer_draw_status(const MT_Player& p) {
    std::cout << "HP:" << p.hp << "/" << p.max_hp
              << " ATK:" << p.atk << " DEF:" << p.def
              << " Keys:" << p.keys << "\n";
}
