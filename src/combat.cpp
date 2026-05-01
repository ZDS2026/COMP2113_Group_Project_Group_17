#include "combat.h"

MT_CombatResult mt_combat_resolve(const MT_Player& p, const MT_Enemy& e) {
    MT_CombatResult res;

    // Set default result values
    res.player_won = false;
    res.damage_taken = 0;
    res.rounds = 0;

    // Create local copies of HP
    // This function should not change the real player or enemy object
    int player_hp = p.hp;
    int enemy_hp = e.hp;

    // Calculate player damage
    int player_damage = p.atk - e.def;
    if (player_damage < 1) {
        player_damage = 1;
    }

    // Calculate enemy damage
    int enemy_damage = e.atk - p.def;
    if (enemy_damage < 1) {
        enemy_damage = 1;
    }

    // Run the battle round by round
    while (player_hp > 0 && enemy_hp > 0) {
        // Count one round
        res.rounds = res.rounds + 1;

        // Player attacks first
        enemy_hp = enemy_hp - player_damage;

        // If the enemy is defeated, the battle ends
        if (enemy_hp <= 0) {
            res.player_won = true;
            break;
        }

        // Enemy attacks back
        player_hp = player_hp - enemy_damage;
        res.damage_taken = res.damage_taken + enemy_damage;

        // If the player is defeated, the battle ends
        if (player_hp <= 0) {
            res.player_won = false;
            break;
        }
    }

    return res;
}
