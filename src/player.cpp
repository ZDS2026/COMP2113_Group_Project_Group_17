#include "player.h"
#include <iostream>

using namespace std;

void mt_player_init(MT_Player& p, int start_hp, int start_atk, int start_def) {
    // Set the player's basic attributes
    p.hp = start_hp;
    p.max_hp = start_hp;
    p.atk = start_atk;
    p.def = start_def;
    p.keys = 0;

    // Set the initial inventory size
    p.inv_cap = 4;
    p.inv_count = 0;

    // Create the dynamic inventory array
    p.inventory = new int[p.inv_cap];
}

void mt_player_destroy(MT_Player& p) {
    // Free the inventory memory
    delete[] p.inventory;

    // Reset the pointer and counters
    p.inventory = nullptr;
    p.inv_count = 0;
    p.inv_cap = 0;
}

bool mt_player_expand_inventory(MT_Player& p) {
    // Decide the new capacity
    int new_cap;
    if (p.inv_cap <= 0) {
        new_cap = 4;
    } else {
        new_cap = p.inv_cap * 2;
    }

    // Create a new bigger array
    int* new_inventory = new int[new_cap];

    // Copy old items into the new array
    for (int i = 0; i < p.inv_count; i++) {
        new_inventory[i] = p.inventory[i];
    }

    // Delete the old array
    delete[] p.inventory;

    // Make the player use the new array
    p.inventory = new_inventory;
    p.inv_cap = new_cap;

    return true;
}

bool mt_player_add_item(MT_Player& p, int item_id) {
    // If the inventory is full, expand it first
    if (p.inv_count >= p.inv_cap) {
        bool success = mt_player_expand_inventory(p);
        if (!success) {
            return false;
        }
    }

    // Put the new item at the end
    p.inventory[p.inv_count] = item_id;
    p.inv_count++;

    return true;
}

bool mt_player_use_item(MT_Player& p, int inv_index) {
    // Check whether the index is valid
    if (inv_index < 0 || inv_index >= p.inv_count) {
        return false;
    }

    int item_id = p.inventory[inv_index];

    // Apply the item effect
    if (item_id == 1) {
        // Item 1: healing potion
        p.hp = p.hp + 50;
        if (p.hp > p.max_hp) {
            p.hp = p.max_hp;
        }
    }
    else if (item_id == 2) {
        // Item 2: attack potion
        p.atk = p.atk + 5;
    }
    else if (item_id == 3) {
        // Item 3: defense potion
        p.def = p.def + 5;
    }
    else if (item_id == 4) {
        // Item 4: key item
        p.keys = p.keys + 1;
    }
    else {
        // Unknown item ID
        return false;
    }

    // Remove the used item
    // Move all later items one step to the left
    for (int i = inv_index; i < p.inv_count - 1; i++) {
        p.inventory[i] = p.inventory[i + 1];
    }

    // Reduce inventory count
    p.inv_count--;

    return true;
}

bool mt_player_is_dead(const MT_Player& p) {
    if (p.hp <= 0) {
        return true;
    } else {
        return false;
    }
}

void mt_player_take_damage(MT_Player& p, int damage) {
    // Do not allow negative damage
    if (damage < 0) {
        damage = 0;
    }

    p.hp = p.hp - damage;

    // HP should not go below 0
    if (p.hp < 0) {
        p.hp = 0;
    }
}

void mt_player_heal(MT_Player& p, int amount) {
    // Do not allow negative healing
    if (amount < 0) {
        amount = 0;
    }

    p.hp = p.hp + amount;

    // HP should not go above max_hp
    if (p.hp > p.max_hp) {
        p.hp = p.max_hp;
    }
}
