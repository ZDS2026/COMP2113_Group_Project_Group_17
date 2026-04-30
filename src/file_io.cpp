#include "file_io.h"
#include <fstream>

bool mt_io_save_game(const MT_Player& p, const char** grid, int w, int h, const MT_Position& pos, MT_Difficulty diff, const std::string& path) {
    std::ofstream out(path);
    if(!out.is_open()) return false;
    out << static_cast<int>(diff) << " " << w << " " << h << " "
        << p.hp << " " << p.atk << " " << p.def << " " << p.keys << " " << p.inv_count << "\n";
    for(int i=0; i<p.inv_count; ++i) out << p.inventory[i] << " ";
    out << "\n" << pos.row << " " << pos.col << "\n";
    for(int r=0; r<h; ++r) {
        for(int c=0; c<w; ++c) out << grid[r][c];
        out << "\n";
    }
    out.close();
    return true;
}

bool mt_io_load_game(MT_Player& p, char**& grid, int& w, int& h, MT_Position& pos, MT_Difficulty& diff, const std::string& path) {
    std::ifstream in(path);
    if(!in.is_open()) return false;
    int old_h = h;
    int diff_raw = static_cast<int>(MT_EASY);
    int inv_cnt = 0;
    in >> diff_raw >> w >> h >> p.hp >> p.atk >> p.def >> p.keys >> inv_cnt;
    diff = static_cast<MT_Difficulty>(diff_raw);
    p.max_hp = p.hp; p.inv_count = 0; p.inv_cap = 4;
    if (p.inventory != nullptr) {
        delete[] p.inventory;
    }
    p.inventory = new int[p.inv_cap];
    for(int i=0; i<inv_cnt; ++i) {
        int item; in >> item;
        if(p.inv_count == p.inv_cap) mt_player_expand_inventory(p);
        p.inventory[p.inv_count++] = item;
    }
    in >> pos.row >> pos.col;
    if (grid != nullptr && old_h > 0) {
        mt_map_destroy(grid, old_h);
    }
    grid = mt_map_create(w, h);
    for(int r=0; r<h; ++r) {
        std::string row_str; in >> row_str;
        for(int c=0; c<w; ++c) {
            grid[r][c] = (c < static_cast<int>(row_str.size())) ? row_str[c] : '.';
        }
    }
    in.close();
    return true;
}