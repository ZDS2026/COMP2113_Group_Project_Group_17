#include "player.h"
#include <algorithm>
#include <iostream>

void mt_player_init(MT_Player& p, int start_hp, int start_atk, int start_def) {
    p.hp = start_hp; p.max_hp = start_hp;
    p.atk = start_atk; p.def = start_def; p.keys = 0;
    p.inv_cap = 4; p.inv_count = 0;
    // Dynamic memory allocation (Course Requirement)
    p.inventory = new int[p.inv_cap]; 
}

void mt_player_destroy(MT_Player& p) {
    delete[] p.inventory;
    p.inventory = nullptr;
}

bool mt_player_expand_inventory(MT_Player& p) {
    int new_cap = p.inv_cap * 2;
    int* new_inv = new int[new_cap]; // Dynamic allocation
    for(int i=0; i<p.inv_count; ++i) new_inv[i] = p.inventory[i];
    delete[] p.inventory;
    p.inventory = new_inv;
    p.inv_cap = new_cap;
    return true;
}

void mt_player_use_item(MT_Player& p, int inv_index) {
    if(inv_index < 0 || inv_index >= p.inv_count) return;
    int item = p.inventory[inv_index];
    if(item == 1) { p.hp = std::min(p.max_hp, p.hp + 50); } // 血瓶
    else if(item == 2) { p.atk += 5; } // 攻击药
    // TODO: 移除已使用物品（后续元素前移，inv_count--）
}