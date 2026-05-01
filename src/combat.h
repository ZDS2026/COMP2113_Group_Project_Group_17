#ifndef MT_COMBAT_H
#define MT_COMBAT_H

#include "player.h"
#include "entity.h"

// This structure stores the result of one combat.
struct MT_CombatResult {
    bool player_won;     // true if the player wins
    int damage_taken;    // total damage taken by the player
    int rounds;          // total number of rounds
};

/**
 * @brief Resolve one turn-based combat.
 *
 * @param p A copy of the player data (read-only).
 * @param e A copy of the enemy data (read-only).
 * @return The combat result.
 *
 * @note This function does not directly change the real player object.
 * @note The controller can use the result to decide:
 *       - whether to reduce player HP
 *       - whether the player gets rewards
 *       - whether the game state should change
 */
MT_CombatResult mt_combat_resolve(const MT_Player& p, const MT_Enemy& e);

#endif
