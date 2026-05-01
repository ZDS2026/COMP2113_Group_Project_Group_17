#ifndef MT_PLAYER_H
#define MT_PLAYER_H

#include "common.h"

struct MT_Player {
    int hp;
    int max_hp;
    int atk;
    int def;
    int keys;

    // Dynamic array storing item IDs
    int* inventory;

    // Current number of items in the inventory
    int inv_count;

    // Current inventory capacity
    int inv_cap;
};

/**
 * @brief Initialize player stats and allocate dynamic inventory memory.
 *
 * @param p The player object to initialize.
 * @param start_hp Initial maximum HP and current HP.
 * @param start_atk Initial attack value.
 * @param start_def Initial defense value.
 *
 * @note This function allocates dynamic memory for inventory.
 *       The caller must later call mt_player_destroy().
 */
void mt_player_init(MT_Player& p, int start_hp, int start_atk, int start_def);

/**
 * @brief Release the dynamic memory used by the inventory.
 *
 * @param p The player object whose inventory memory will be released.
 *
 * @note This function is safe to call multiple times.
 *       If inventory is already null, it has no side effects.
 */
void mt_player_destroy(MT_Player& p);

/**
 * @brief Expand the inventory capacity to twice its current size.
 *
 * @param p The player whose inventory will be expanded.
 * @return true if expansion succeeds, false otherwise.
 */
bool mt_player_expand_inventory(MT_Player& p);

/**
 * @brief Add an item ID to the player's inventory.
 *
 * @param p The player receiving the item.
 * @param item_id The ID of the item to add.
 * @return true if the item is added successfully, false otherwise.
 *
 * @note This function expands the inventory automatically if needed.
 */
bool mt_player_add_item(MT_Player& p, int item_id);

/**
 * @brief Use the item at the given inventory index.
 *
 * @param p The player using the item.
 * @param inv_index The inventory index (0-based).
 * @return true if the item is used successfully, false otherwise.
 *
 * @note This function should validate the index before use.
 * @note After applying the item effect, the used item should be removed
 *       and later items should be shifted left.
 */
bool mt_player_use_item(MT_Player& p, int inv_index);

/**
 * @brief Check whether the player is dead.
 *
 * @param p The player to check.
 * @return true if hp <= 0, false otherwise.
 */
bool mt_player_is_dead(const MT_Player& p);

/**
 * @brief Apply damage to the player.
 *
 * @param p The player taking damage.
 * @param damage The raw damage value.
 *
 * @note HP will never become negative.
 * @note Negative damage should be treated as zero.
 */
void mt_player_take_damage(MT_Player& p, int damage);

/**
 * @brief Heal the player.
 *
 * @param p The player being healed.
 * @param amount The healing amount.
 *
 * @note HP will never exceed max_hp.
 * @note Negative healing should be treated as zero.
 */
void mt_player_heal(MT_Player& p, int amount);

#endif
