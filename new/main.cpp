#include "difficulty.h"
#include "game.h"
#include "input.h"

#include <iostream>
#include <string>

namespace {
const char* PURP = "\033[95m";
const char* RST = "\033[0m";

// Inner width between '+' / '|' borders (must match title padding).
constexpr int kMenuInnerWidth = 32;

void draw_main_menu() {
    std::cout << "\033[2J\033[H";
    std::cout << "\n";
    const std::string title = "MINI MAZE TOWER";
    const int pad = kMenuInnerWidth - static_cast<int>(title.size());
    const int pad_left = pad / 2;
    const int pad_right = pad - pad_left;
    const std::string top = "+" + std::string(kMenuInnerWidth, '-') + "+";
    const std::string mid_inner = std::string(pad_left, ' ') + title + std::string(pad_right, ' ');

    std::cout << "        " << PURP << top << "\n";
    std::cout << "        " << PURP << "|" << RST << mid_inner << PURP << "|\n";
    std::cout << "        " << top << RST << "\n\n";
    std::cout << "          " << PURP << "-> " << RST << "1. Easy\n";
    std::cout << "             2. Medium\n";
    std::cout << "             3. Hard\n";
    std::cout << "             4. Continue Saved Game\n";
    std::cout << "             Q. Quit\n\n";
    std::cout << "          Press 1-4 or Q (no Enter)\n" << std::flush;
}

bool play_again_prompt() {
    std::cout << "\nPlay again? Y / N (single key)\n" << std::flush;
    for (;;) {
        int k = read_key_immediate();
        if (k == -2) continue;
        if (k == 'y' || k == 'Y') return true;
        if (k == 'n' || k == 'N') return false;
        if (k < 0) return false;
    }
}
} // namespace

int main() {
    std::cout << "\033[2J\033[H" << std::flush;
    while (true) {
        draw_main_menu();
        int k = read_key_immediate();
        while (k == -2) k = read_key_immediate();
        if (k == 'q' || k == 'Q') return 0;
        if (k != '1' && k != '2' && k != '3' && k != '4') continue;

        Game game;
        bool ok = false;
        if (k >= '1' && k <= '3') {
            ok = game.start_new(static_cast<DifficultyLevel>(k - '0'));
        } else {
            std::cout << "\n          Select save slot 1-5 (Esc = back to menu)\n" << std::flush;
            int sk = read_key_immediate();
            while (sk == -2) sk = read_key_immediate();
            if (sk == 27) continue;
            if (sk < '1' || sk > '5') {
                std::cout << "\n          Invalid slot. Press any key...\n" << std::flush;
                (void)read_key_immediate();
                continue;
            }
            ok = game.load_saved(sk - '0');
        }

        if (!ok) {
            std::cout << "\n          Cannot start (init or load failed). Press any key...\n" << std::flush;
            int w = read_key_immediate();
            while (w == -2) w = read_key_immediate();
            continue;
        }

        game.run();
        if (!play_again_prompt()) break;
    }
    return 0;
}
