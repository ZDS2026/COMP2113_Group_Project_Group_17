#include "game.h"

#include "difficulty.h"
#include "input.h"
#include "io.h"
#include "map.h"

#include <algorithm>
#include <queue>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>

namespace {
int random_floor_row(int h) { return std::rand() % h; }
int random_floor_col(int w) { return std::rand() % w; }

MonsterType random_monster_type() {
    int roll = std::rand() % 100;
    if (roll < 40) return MON_SLIME;
    if (roll < 70) return MON_SKELETON;
    if (roll < 90) return MON_ORC;
    return MON_WARLOCK;
}

int base_monster_hp(MonsterType t) {
    switch (t) {
        case MON_SLIME: return 30;
        case MON_SKELETON: return 50;
        case MON_ORC: return 70;
        case MON_WARLOCK: return 45;
        case MON_BOSS: return 180;
        default: return 40;
    }
}

int base_monster_atk(MonsterType t) {
    switch (t) {
        case MON_SLIME: return 10;
        case MON_SKELETON: return 16;
        case MON_ORC: return 20;
        case MON_WARLOCK: return 24;
        case MON_BOSS: return 32;
        default: return 12;
    }
}
}

Game::Game()
    : level(DIFF_EASY), cfg{}, player{}, map(nullptr), width(0), height(0),
      monsters(nullptr), monster_count(0), items(nullptr), item_count(0),
      running(false), win(false), suppress_first_run_draw_(false),
      invalid_input_streak(0),
      last_route_count(0), last_balance_winrate(0), logs(), target_info{} {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

Game::~Game() {
    cleanup_dynamic_memory();
}

void Game::cleanup_dynamic_memory() {
    if (map && height > 0) destroy_map(map, height);
    if (monsters) delete[] monsters;
    if (items) delete[] items;
    map = nullptr;
    monsters = nullptr;
    items = nullptr;
    monster_count = 0;
    item_count = 0;
    width = 0;
    height = 0;
}

void Game::paint_full_frame() {
    refresh_target_panel();
    draw_frame(map, width, height, player, monsters, monster_count, items, item_count, target_info, logs);
}

void Game::add_log(const std::string& msg) {
    logs.push_front(msg);
    while (logs.size() > 6) logs.pop_back();
}

bool Game::load_saved(int slot) {
    if (slot < 1 || slot > 5) return false;
    cleanup_dynamic_memory();
    logs.clear();
    target_info = {};
    guaranteed_path.clear();
    running = true;
    win = false;
    invalid_input_streak = 0;
    last_route_count = 0;
    last_balance_winrate = 0;

    DifficultyLevel loaded_level = DIFF_EASY;
    std::string path = slot_path(slot);
    if (!load_game(path, loaded_level, player, map, width, height, monsters, monster_count, items, item_count)) {
        cleanup_dynamic_memory();
        return false;
    }
    ensure_skeleton_patrols();
    level = loaded_level;
    cfg = get_difficulty_config(level);
    if (!rebuild_guaranteed_path()) {
        cleanup_dynamic_memory();
        return false;
    }
    add_log("Game loaded from " + path);
    paint_full_frame();
    suppress_first_run_draw_ = true;
    return true;
}

bool Game::start_new(DifficultyLevel lv) {
    cleanup_dynamic_memory();
    logs.clear();
    target_info = {};
    running = true;
    win = false;
    invalid_input_streak = 0;
    last_route_count = 0;
    last_balance_winrate = 0;

    level = lv;
    cfg = get_difficulty_config(level);
    width = cfg.width;
    height = cfg.height;

    player.hp = cfg.player_start_hp;
    player.max_hp = cfg.player_start_hp;
    player.atk = cfg.player_start_atk;
    player.def = cfg.player_start_def;
    player.pos = {0, 0};

    bool ok = false;
    for (int attempt = 0; attempt < 160; ++attempt) {
        if (map && height > 0) destroy_map(map, height);
        map = create_map(width, height);
        if (!map) return false;
        generate_map_with_path(map, width, height);
        if (!rebuild_guaranteed_path()) continue;

        if (monsters) { delete[] monsters; monsters = nullptr; }
        if (items) { delete[] items; items = nullptr; }
        monster_count = 0;
        item_count = 0;
        place_entities();

        // Final guard: reject maps where guaranteed route still cannot beat boss.
        if (!simulate_guaranteed_path_clear()) continue;

        int route_count = count_shortest_routes_cap(9);
        int winrate = estimate_balance_winrate(40);

        // Prefer maps with multiple shortest choices. If not, keep trying.
        if (route_count < 2 && attempt < 120) continue;

        ok = true;
        last_route_count = route_count;
        last_balance_winrate = winrate;
        break;
    }
    if (!ok) return false;
    add_log("New game started.");
    add_log("Map simulation check: PASS");
    {
        std::stringstream ss;
        ss << "Route check: shortest-path choices = " << last_route_count;
        add_log(ss.str());
    }
    {
        std::stringstream ss;
        ss << "Balance sim (40 runs): winrate " << last_balance_winrate << "%";
        add_log(ss.str());
    }
    add_log("Defeat the boss at bottom-right!");
    paint_full_frame();
    suppress_first_run_draw_ = true;
    return true;
}

bool Game::rebuild_guaranteed_path() {
    guaranteed_path.clear();
    if (!map) return false;

    const int total = width * height;
    std::vector<int> parent(total, -1);
    std::vector<char> vis(total, 0);
    auto idx = [&](int r, int c) { return r * width + c; };

    std::queue<Position> q;
    q.push({0, 0});
    vis[idx(0, 0)] = 1;
    parent[idx(0, 0)] = idx(0, 0);

    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};
    while (!q.empty()) {
        Position cur = q.front();
        q.pop();
        if (cur.r == height - 1 && cur.c == width - 1) break;
        for (int i = 0; i < 4; ++i) {
            int nr = cur.r + dr[i];
            int nc = cur.c + dc[i];
            if (nr < 0 || nc < 0 || nr >= height || nc >= width) continue;
            if (map[nr][nc] == '#') continue;
            int ni = idx(nr, nc);
            if (vis[ni]) continue;
            vis[ni] = 1;
            parent[ni] = idx(cur.r, cur.c);
            q.push({nr, nc});
        }
    }

    int goal = idx(height - 1, width - 1);
    if (!vis[goal]) return false;

    int cur = goal;
    while (true) {
        int r = cur / width;
        int c = cur % width;
        guaranteed_path.push_back({r, c});
        if (cur == parent[cur]) break;
        cur = parent[cur];
    }
    std::reverse(guaranteed_path.begin(), guaranteed_path.end());
    return !guaranteed_path.empty();
}

int Game::count_shortest_routes_cap(int cap) const {
    if (!map || width <= 0 || height <= 0) return 0;
    auto idx = [&](int r, int c) { return r * width + c; };
    const int total = width * height;
    std::vector<int> dist(total, -1);
    std::vector<int> ways(total, 0);
    std::queue<Position> q;

    int start = idx(0, 0);
    int goal = idx(height - 1, width - 1);
    dist[start] = 0;
    ways[start] = 1;
    q.push({0, 0});

    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};
    while (!q.empty()) {
        Position cur = q.front();
        q.pop();
        int cur_id = idx(cur.r, cur.c);
        for (int k = 0; k < 4; ++k) {
            int nr = cur.r + dr[k];
            int nc = cur.c + dc[k];
            if (nr < 0 || nc < 0 || nr >= height || nc >= width) continue;
            if (map[nr][nc] == '#') continue;
            int nid = idx(nr, nc);
            if (dist[nid] == -1) {
                dist[nid] = dist[cur_id] + 1;
                ways[nid] = ways[cur_id];
                if (ways[nid] > cap) ways[nid] = cap;
                q.push({nr, nc});
            } else if (dist[nid] == dist[cur_id] + 1) {
                ways[nid] += ways[cur_id];
                if (ways[nid] > cap) ways[nid] = cap;
            }
        }
    }
    if (dist[goal] < 0) return 0;
    return ways[goal];
}

void Game::place_entities() {
    monster_count = cfg.monster_count + 1; // +1 boss
    monsters = new Monster[monster_count];
    for (int i = 0; i < monster_count; ++i) {
        monsters[i].alive = false;
        monsters[i].hp = 0;
        monsters[i].atk = 0;
        monsters[i].def = 0;
        monsters[i].pos = {0, 0};
        monsters[i].type = MON_SLIME;
        monsters[i].patrol_dr = 0;
        monsters[i].patrol_dc = 0;
    }

    auto is_on_path = [&](int r, int c) {
        for (const Position& p : guaranteed_path) {
            if (p.r == r && p.c == c) return true;
        }
        return false;
    };

    // Reachability map from start to avoid placing important content in isolated pockets.
    std::vector<char> reachable(width * height, 0);
    auto cell_id = [&](int r, int c) { return r * width + c; };
    std::queue<Position> rq;
    rq.push({0, 0});
    reachable[cell_id(0, 0)] = 1;
    const int dr4[4] = {-1, 1, 0, 0};
    const int dc4[4] = {0, 0, -1, 1};
    while (!rq.empty()) {
        Position cur = rq.front();
        rq.pop();
        for (int i = 0; i < 4; ++i) {
            int nr = cur.r + dr4[i];
            int nc = cur.c + dc4[i];
            if (nr < 0 || nc < 0 || nr >= height || nc >= width) continue;
            if (map[nr][nc] == '#') continue;
            int id = cell_id(nr, nc);
            if (reachable[id]) continue;
            reachable[id] = 1;
            rq.push({nr, nc});
        }
    }

    int path_blockers_target = std::min(2, std::max(0, static_cast<int>(guaranteed_path.size() / 30)));
    int non_boss_total = cfg.monster_count;
    int off_path_target = std::max(0, non_boss_total - path_blockers_target);

    int idx = 0;
    while (idx < off_path_target) {
        int r = random_floor_row(height);
        int c = random_floor_col(width);
        if (map[r][c] == '#') continue;
        if (!reachable[cell_id(r, c)]) continue;
        if ((r == 0 && c == 0) || (r == height - 1 && c == width - 1)) continue;
        if (is_on_path(r, c)) continue;
        if (find_monster_at(r, c) >= 0) continue;

        MonsterType t = random_monster_type();
        monsters[idx].type = t;
        monsters[idx].pos = {r, c};
        monsters[idx].hp = static_cast<int>(base_monster_hp(t) * cfg.monster_hp_scale);
        monsters[idx].atk = static_cast<int>(base_monster_atk(t) * cfg.monster_atk_scale);
        monsters[idx].def = 0;
        monsters[idx].alive = true;
        if (monsters[idx].type == MON_SKELETON) roll_skeleton_patrol(idx);
        ++idx;
    }

    // Place a few weak monsters on the guaranteed path to create tactical pressure,
    // while keeping the route still passable.
    int path_n = static_cast<int>(guaranteed_path.size());
    for (int placed = 0; placed < path_blockers_target && idx < non_boss_total; ++placed) {
        int base_slot = ((placed + 2) * path_n) / (path_blockers_target + 4);
        base_slot = std::max(2, std::min(path_n - 3, base_slot));
        bool done = false;
        for (int shift = 0; shift < path_n; ++shift) {
            Position p = guaranteed_path[(base_slot + shift) % path_n];
            if ((p.r == 0 && p.c == 0) || (p.r == height - 1 && p.c == width - 1)) continue;
            if (find_monster_at(p.r, p.c) >= 0) continue;
            MonsterType t = MON_SLIME;
            monsters[idx].type = t;
            monsters[idx].pos = p;
            monsters[idx].hp = static_cast<int>(base_monster_hp(t) * cfg.monster_hp_scale);
            monsters[idx].atk = static_cast<int>(base_monster_atk(t) * cfg.monster_atk_scale);
            monsters[idx].def = 0;
            monsters[idx].alive = true;
            if (monsters[idx].type == MON_SKELETON) roll_skeleton_patrol(idx);
            ++idx;
            done = true;
            break;
        }
        if (!done) break;
    }

    // Fill any remaining non-boss slots anywhere walkable.
    while (idx < non_boss_total) {
        int r = random_floor_row(height);
        int c = random_floor_col(width);
        if (map[r][c] == '#') continue;
        if (!reachable[cell_id(r, c)]) continue;
        if ((r == 0 && c == 0) || (r == height - 1 && c == width - 1)) continue;
        if (find_monster_at(r, c) >= 0) continue;
        MonsterType t = random_monster_type();
        monsters[idx].type = t;
        monsters[idx].pos = {r, c};
        monsters[idx].hp = static_cast<int>(base_monster_hp(t) * cfg.monster_hp_scale);
        monsters[idx].atk = static_cast<int>(base_monster_atk(t) * cfg.monster_atk_scale);
        monsters[idx].def = 0;
        monsters[idx].alive = true;
        if (monsters[idx].type == MON_SKELETON) roll_skeleton_patrol(idx);
        ++idx;
    }

    monsters[monster_count - 1].type = MON_BOSS;
    monsters[monster_count - 1].pos = {height - 1, width - 1};
    monsters[monster_count - 1].hp = static_cast<int>(base_monster_hp(MON_BOSS) * cfg.monster_hp_scale);
    monsters[monster_count - 1].atk = static_cast<int>(base_monster_atk(MON_BOSS) * cfg.monster_atk_scale);
    monsters[monster_count - 1].def = (level == DIFF_HARD) ? 12 : (level == DIFF_MEDIUM ? 8 : 6);
    monsters[monster_count - 1].alive = true;
    monsters[monster_count - 1].patrol_dr = 0;
    monsters[monster_count - 1].patrol_dc = 0;

    item_count = cfg.potion_count + cfg.sword_count + cfg.shield_count;
    items = new Item[item_count];
    for (int i = 0; i < item_count; ++i) {
        items[i].type = ITEM_NONE;
        items[i].pos = {0, 0};
        items[i].active = false;
    }
    idx = 0;

    int rem_potion = cfg.potion_count;
    int rem_sword = cfg.sword_count;
    int rem_shield = cfg.shield_count;

    auto place_guaranteed_item_on_path = [&](ItemType type, int count) -> int {
        int placed_count = 0;
        if (guaranteed_path.size() < 4) return placed_count;
        int path_n = static_cast<int>(guaranteed_path.size());
        for (int i = 0; i < count; ++i) {
            int slot = ((i + 1) * path_n) / (count + 2);
            slot = std::max(1, std::min(path_n - 2, slot));
            int tries = 0;
            while (tries < path_n) {
                Position p = guaranteed_path[(slot + tries) % path_n];
                if (!(p.r == 0 && p.c == 0) &&
                    !(p.r == height - 1 && p.c == width - 1) &&
                    find_item_at(p.r, p.c) < 0 &&
                    find_monster_at(p.r, p.c) < 0) {
                    items[idx].type = type;
                    items[idx].pos = p;
                    items[idx].active = true;
                    ++idx;
                    ++placed_count;
                    break;
                }
                ++tries;
            }
        }
        return placed_count;
    };

    int guaranteed_potion = std::min(3, rem_potion);
    int guaranteed_sword = std::min(2, rem_sword);
    int guaranteed_shield = std::min(2, rem_shield);
    rem_potion -= place_guaranteed_item_on_path(ITEM_POTION, guaranteed_potion);
    rem_sword -= place_guaranteed_item_on_path(ITEM_SWORD, guaranteed_sword);
    rem_shield -= place_guaranteed_item_on_path(ITEM_SHIELD, guaranteed_shield);

    auto place_item = [&](ItemType type, int count) {
        for (int k = 0; k < count; ++k) {
            while (true) {
                int r = random_floor_row(height);
                int c = random_floor_col(width);
                if (map[r][c] == '#') continue;
                if (!reachable[cell_id(r, c)]) continue;
                if ((r == 0 && c == 0) || (r == height - 1 && c == width - 1)) continue;
                if (find_item_at(r, c) >= 0) continue;
                if (find_monster_at(r, c) >= 0) continue;
                items[idx].type = type;
                items[idx].pos = {r, c};
                items[idx].active = true;
                ++idx;
                break;
            }
        }
    };
    place_item(ITEM_POTION, rem_potion);
    place_item(ITEM_SWORD, rem_sword);
    place_item(ITEM_SHIELD, rem_shield);

    ensure_boss_beatable();
}

void Game::ensure_boss_beatable() {
    if (monster_count <= 0) return;
    Monster& boss = monsters[monster_count - 1];
    if (boss.type != MON_BOSS) return;

    int expected_atk = player.atk;
    int expected_def = player.def;
    int expected_hp = player.max_hp;
    for (int i = 0; i < item_count; ++i) {
        if (!items[i].active) continue;
        bool on_path = false;
        for (const Position& p : guaranteed_path) {
            if (p.r == items[i].pos.r && p.c == items[i].pos.c) {
                on_path = true;
                break;
            }
        }
        if (!on_path) continue;
        if (items[i].type == ITEM_POTION) expected_hp += 25;
        else if (items[i].type == ITEM_SWORD) expected_atk += 6;
        else if (items[i].type == ITEM_SHIELD) expected_def += 4;
    }
    expected_hp = std::min(expected_hp, player.max_hp + 120);

    auto can_win = [&](int b_hp, int b_atk, int b_def) {
        int p_dmg = std::max(1, expected_atk - b_def);
        int b_dmg = std::max(0, b_atk - expected_def);
        int rounds = (b_hp + p_dmg - 1) / p_dmg;
        int total_taken = (rounds - 1) * b_dmg;
        return expected_hp > total_taken;
    };

    int safe_guard = 0;
    while (!can_win(boss.hp, boss.atk, boss.def) && safe_guard < 200) {
        if (boss.atk > 18) boss.atk -= 1;
        else if (boss.def > 3) boss.def -= 1;
        else if (boss.hp > 80) boss.hp -= 4;
        else break;
        ++safe_guard;
    }

    int sim_guard = 0;
    while (!simulate_guaranteed_path_clear() && sim_guard < 250) {
        bool changed = false;
        if (boss.atk > 16) { boss.atk -= 1; changed = true; }
        if (boss.def > 2) { boss.def -= 1; changed = true; }
        if (boss.hp > 70) { boss.hp -= 2; changed = true; }

        if (!changed) {
            for (int i = 0; i < monster_count; ++i) {
                if (monsters[i].type == MON_BOSS || !monsters[i].alive) continue;
                bool on_path = false;
                for (size_t k = 1; k < guaranteed_path.size(); ++k) {
                    if (monsters[i].pos.r == guaranteed_path[k].r &&
                        monsters[i].pos.c == guaranteed_path[k].c) {
                        on_path = true;
                        break;
                    }
                }
                if (!on_path) continue;
                if (monsters[i].atk > 1) { monsters[i].atk -= 1; changed = true; }
                if (monsters[i].hp > 8) { monsters[i].hp -= 2; changed = true; }
            }
        }

        if (!changed) break;
        ++sim_guard;
    }

    // Monte-Carlo balancing: nudge boss if hard map still trends too punishing.
    int target_winrate = (level == DIFF_EASY) ? 75 : ((level == DIFF_MEDIUM) ? 58 : 42);
    int guard = 0;
    int wr = estimate_balance_winrate(30);
    while (wr < target_winrate && guard < 80) {
        if (boss.atk > 14) boss.atk -= 1;
        else if (boss.def > 1) boss.def -= 1;
        else if (boss.hp > 65) boss.hp -= 2;
        else break;
        wr = estimate_balance_winrate(30);
        ++guard;
    }
}

bool Game::simulate_guaranteed_path_clear() const {
    if (!monsters || !items || guaranteed_path.empty()) return false;
    int p_hp = player.hp;
    int p_max_hp = player.max_hp;
    int p_atk = player.atk;
    int p_def = player.def;

    std::vector<int> mhp(monster_count, 0);
    std::vector<char> malive(monster_count, 0);
    for (int i = 0; i < monster_count; ++i) {
        mhp[i] = monsters[i].hp;
        malive[i] = monsters[i].alive ? 1 : 0;
    }

    std::vector<char> iactive(item_count, 0);
    for (int i = 0; i < item_count; ++i) iactive[i] = items[i].active ? 1 : 0;

    auto find_monster_idx = [&](int r, int c) {
        for (int i = 0; i < monster_count; ++i) {
            if (malive[i] && monsters[i].pos.r == r && monsters[i].pos.c == c) return i;
        }
        return -1;
    };
    auto find_item_idx = [&](int r, int c) {
        for (int i = 0; i < item_count; ++i) {
            if (iactive[i] && items[i].pos.r == r && items[i].pos.c == c) return i;
        }
        return -1;
    };

    for (size_t step = 1; step < guaranteed_path.size(); ++step) {
        int r = guaranteed_path[step].r;
        int c = guaranteed_path[step].c;

        int item_idx = find_item_idx(r, c);
        if (item_idx >= 0) {
            iactive[item_idx] = 0;
            if (items[item_idx].type == ITEM_POTION) p_hp = std::min(p_max_hp, p_hp + 35);
            else if (items[item_idx].type == ITEM_SWORD) p_atk += 6;
            else if (items[item_idx].type == ITEM_SHIELD) p_def += 4;
        }

        int mon_idx = find_monster_idx(r, c);
        if (mon_idx >= 0) {
            while (p_hp > 0 && mhp[mon_idx] > 0) {
                mhp[mon_idx] -= std::max(1, p_atk - monsters[mon_idx].def);
                if (mhp[mon_idx] <= 0) break;
                p_hp -= std::max(0, monsters[mon_idx].atk - p_def);
            }
            if (p_hp <= 0) return false;
            malive[mon_idx] = 0;
        }
    }

    for (int i = 0; i < monster_count; ++i) {
        if (monsters[i].type == MON_BOSS) return !malive[i];
    }
    return false;
}

int Game::estimate_balance_winrate(int samples) const {
    if (samples <= 0 || !monsters || !items || guaranteed_path.empty()) return 0;
    int wins = 0;
    for (int s = 0; s < samples; ++s) {
        int p_hp = player.hp;
        int p_max_hp = player.max_hp;
        int p_atk = player.atk;
        int p_def = player.def;

        std::vector<int> mhp(monster_count, 0);
        std::vector<char> malive(monster_count, 0);
        for (int i = 0; i < monster_count; ++i) {
            mhp[i] = monsters[i].hp;
            malive[i] = monsters[i].alive ? 1 : 0;
        }
        std::vector<char> iactive(item_count, 0);
        for (int i = 0; i < item_count; ++i) iactive[i] = items[i].active ? 1 : 0;

        auto find_monster_idx = [&](int r, int c) {
            for (int i = 0; i < monster_count; ++i) {
                if (malive[i] && monsters[i].pos.r == r && monsters[i].pos.c == c) return i;
            }
            return -1;
        };
        auto find_item_idx = [&](int r, int c) {
            for (int i = 0; i < item_count; ++i) {
                if (iactive[i] && items[i].pos.r == r && items[i].pos.c == c) return i;
            }
            return -1;
        };

        for (size_t step = 1; step < guaranteed_path.size() && p_hp > 0; ++step) {
            int r = guaranteed_path[step].r;
            int c = guaranteed_path[step].c;

            int item_idx = find_item_idx(r, c);
            if (item_idx >= 0) {
                iactive[item_idx] = 0;
                if (items[item_idx].type == ITEM_POTION) p_hp = std::min(p_max_hp, p_hp + 35);
                else if (items[item_idx].type == ITEM_SWORD) p_atk += 6;
                else if (items[item_idx].type == ITEM_SHIELD) p_def += 4;
            }

            int mon_idx = find_monster_idx(r, c);
            if (mon_idx >= 0) {
                while (p_hp > 0 && mhp[mon_idx] > 0) {
                    int p_base = std::max(1, p_atk - monsters[mon_idx].def);
                    int p_jitter = 90 + (std::rand() % 21); // 90%~110%
                    int p_dmg = std::max(1, (p_base * p_jitter) / 100);
                    mhp[mon_idx] -= p_dmg;
                    if (mhp[mon_idx] <= 0) break;

                    int m_base = std::max(0, monsters[mon_idx].atk - p_def);
                    int m_jitter = 90 + (std::rand() % 21); // 90%~110%
                    int m_dmg = (m_base * m_jitter) / 100;
                    p_hp -= m_dmg;
                }
                malive[mon_idx] = 0;
            }
        }

        bool boss_dead = false;
        for (int i = 0; i < monster_count; ++i) {
            if (monsters[i].type == MON_BOSS) {
                boss_dead = !malive[i];
                break;
            }
        }
        if (p_hp > 0 && boss_dead) ++wins;
    }
    return (wins * 100) / samples;
}

void Game::show_help_screen() const {
    std::cout << "\033[2J\033[H";
    std::cout << "==== Help / Rules ====\n\n";
    std::cout << "Goal:\n";
    std::cout << "  Defeat boss B at bottom-right.\n\n";
    std::cout << "Controls:\n";
    std::cout << "  W A S D : move\n";
    std::cout << "  P       : save game (then press 1-5 for slot, no Enter)\n";
    std::cout << "  L       : load game (then press 1-5 for slot, no Enter)\n";
    std::cout << "  H       : open this help page\n";
    std::cout << "  Q       : quit\n\n";
    std::cout << "Combat:\n";
    std::cout << "  Player damage = max(1, PlayerATK - MonsterDEF)\n";
    std::cout << "  Monster damage = max(0, MonsterATK - PlayerDEF)\n\n";
    std::cout << "Items:\n";
    std::cout << "  H Potion: +35 HP (up to max HP)\n";
    std::cout << "  S Sword : +6 ATK\n";
    std::cout << "  D Shield: +4 DEF\n\n";
    std::cout << "Monsters (after each WASD step; Boss never moves):\n";
    std::cout << "  Slime    : random wander\n";
    std::cout << "  Skeleton : patrol in a line, turns when blocked\n";
    std::cout << "  Orc      : closes in within Manhattan 4, else wanders\n";
    std::cout << "  Warlock  : backs off if too close; keeps distance mid-range\n\n";
    std::cout << "Press any key to return...\n" << std::flush;
    for (;;) {
        int k = read_key_immediate();
        if (k >= 0 && k != -2) break;
    }
}

std::string Game::slot_path(int slot) const {
    if (slot < 1 || slot > 5) return "";
    return "save" + std::to_string(slot) + ".dat";
}

int Game::ask_slot_from_user(const std::string& action) const {
    std::cout << "\n" << action << " slot: press 1-5 (Esc = cancel)\n" << std::flush;
    for (;;) {
        int ch = read_key_immediate();
        if (ch == -2 || ch < 0) continue;
        if (ch == 27) return 0;
        if (ch >= '1' && ch <= '5') return ch - '0';
        return 0;
    }
}

int Game::find_monster_at(int r, int c) const {
    for (int i = 0; i < monster_count; ++i) {
        if (monsters[i].alive && monsters[i].pos.r == r && monsters[i].pos.c == c) return i;
    }
    return -1;
}

int Game::find_item_at(int r, int c) const {
    for (int i = 0; i < item_count; ++i) {
        if (items[i].active && items[i].pos.r == r && items[i].pos.c == c) return i;
    }
    return -1;
}

bool Game::cell_blocked_for_monster(int r, int c, int self_idx) const {
    if (!is_walkable(map, width, height, r, c)) return true;
    const int j = find_monster_at(r, c);
    if (j >= 0 && j != self_idx) return true;
    return false;
}

void Game::roll_skeleton_patrol(int idx) {
    if (idx < 0 || idx >= monster_count) return;
    Monster& m = monsters[idx];
    const int dr4[4] = {-1, 1, 0, 0};
    const int dc4[4] = {0, 0, -1, 1};
    int cand[4][2];
    int n = 0;
    for (int k = 0; k < 4; ++k) {
        const int tr = m.pos.r + dr4[k];
        const int tc = m.pos.c + dc4[k];
        if (!cell_blocked_for_monster(tr, tc, idx)) {
            cand[n][0] = dr4[k];
            cand[n][1] = dc4[k];
            ++n;
        }
    }
    if (n == 0) {
        m.patrol_dr = 0;
        m.patrol_dc = 0;
        return;
    }
    const int pick = std::rand() % n;
    m.patrol_dr = cand[pick][0];
    m.patrol_dc = cand[pick][1];
}

void Game::ensure_skeleton_patrols() {
    for (int i = 0; i < monster_count; ++i) {
        if (!monsters[i].alive || monsters[i].type != MON_SKELETON) continue;
        if (monsters[i].patrol_dr == 0 && monsters[i].patrol_dc == 0) roll_skeleton_patrol(i);
    }
}

void Game::tick_monsters() {
    if (!running) return;
    const int dr4[4] = {-1, 1, 0, 0};
    const int dc4[4] = {0, 0, -1, 1};
    auto manhattan = [&](int r, int c) {
        return std::abs(r - player.pos.r) + std::abs(c - player.pos.c);
    };

    for (int i = 0; i < monster_count && running; ++i) {
        Monster& m = monsters[i];
        if (!m.alive || m.type == MON_BOSS) continue;

        const int r = m.pos.r;
        const int c = m.pos.c;
        int nr = r;
        int nc = c;

        if (m.type == MON_SLIME) {
            int moves[5][2];
            int nm = 0;
            moves[nm][0] = 0;
            moves[nm][1] = 0;
            ++nm;
            for (int k = 0; k < 4; ++k) {
                const int tr = r + dr4[k];
                const int tc = c + dc4[k];
                if (!cell_blocked_for_monster(tr, tc, i)) {
                    moves[nm][0] = dr4[k];
                    moves[nm][1] = dc4[k];
                    ++nm;
                }
            }
            const int pick = std::rand() % nm;
            nr = r + moves[pick][0];
            nc = c + moves[pick][1];
        } else if (m.type == MON_SKELETON) {
            if (m.patrol_dr == 0 && m.patrol_dc == 0) roll_skeleton_patrol(i);
            if (m.patrol_dr != 0 || m.patrol_dc != 0) {
                const int tr = r + m.patrol_dr;
                const int tc = c + m.patrol_dc;
                if (!cell_blocked_for_monster(tr, tc, i)) {
                    nr = tr;
                    nc = tc;
                } else {
                    roll_skeleton_patrol(i);
                }
            }
        } else if (m.type == MON_ORC) {
            const int d0 = manhattan(r, c);
            if (d0 > 0 && d0 <= 4) {
                int best = d0;
                int cand[4][2];
                int ncand = 0;
                for (int k = 0; k < 4; ++k) {
                    const int tr = r + dr4[k];
                    const int tc = c + dc4[k];
                    if (cell_blocked_for_monster(tr, tc, i)) continue;
                    const int d1 = manhattan(tr, tc);
                    if (d1 < best) {
                        best = d1;
                        ncand = 0;
                        cand[ncand][0] = dr4[k];
                        cand[ncand][1] = dc4[k];
                        ++ncand;
                    } else if (d1 == best) {
                        cand[ncand][0] = dr4[k];
                        cand[ncand][1] = dc4[k];
                        ++ncand;
                    }
                }
                if (ncand > 0) {
                    const int pick = std::rand() % ncand;
                    nr = r + cand[pick][0];
                    nc = c + cand[pick][1];
                }
            } else {
                int moves[5][2];
                int nm = 0;
                moves[nm][0] = 0;
                moves[nm][1] = 0;
                ++nm;
                for (int k = 0; k < 4; ++k) {
                    const int tr = r + dr4[k];
                    const int tc = c + dc4[k];
                    if (!cell_blocked_for_monster(tr, tc, i)) {
                        moves[nm][0] = dr4[k];
                        moves[nm][1] = dc4[k];
                        ++nm;
                    }
                }
                const int pick = std::rand() % nm;
                nr = r + moves[pick][0];
                nc = c + moves[pick][1];
            }
        } else if (m.type == MON_WARLOCK) {
            const int d0 = manhattan(r, c);
            if (d0 <= 2 && d0 > 0) {
                int cand[4][2];
                int ncand = 0;
                for (int k = 0; k < 4; ++k) {
                    const int tr = r + dr4[k];
                    const int tc = c + dc4[k];
                    if (cell_blocked_for_monster(tr, tc, i)) continue;
                    if (manhattan(tr, tc) > d0) {
                        cand[ncand][0] = dr4[k];
                        cand[ncand][1] = dc4[k];
                        ++ncand;
                    }
                }
                if (ncand > 0) {
                    const int pick = std::rand() % ncand;
                    nr = r + cand[pick][0];
                    nc = c + cand[pick][1];
                }
            } else if (d0 >= 3 && d0 <= 5) {
                int cand[4][2];
                int ncand = 0;
                for (int k = 0; k < 4; ++k) {
                    const int tr = r + dr4[k];
                    const int tc = c + dc4[k];
                    if (cell_blocked_for_monster(tr, tc, i)) continue;
                    if (manhattan(tr, tc) >= d0) {
                        cand[ncand][0] = dr4[k];
                        cand[ncand][1] = dc4[k];
                        ++ncand;
                    }
                }
                if (ncand > 0) {
                    const int pick = std::rand() % ncand;
                    nr = r + cand[pick][0];
                    nc = c + cand[pick][1];
                }
            } else {
                if ((std::rand() % 2) == 0) {
                    nr = r;
                    nc = c;
                } else {
                    int moves[4][2];
                    int nm = 0;
                    for (int k = 0; k < 4; ++k) {
                        const int tr = r + dr4[k];
                        const int tc = c + dc4[k];
                        if (!cell_blocked_for_monster(tr, tc, i)) {
                            moves[nm][0] = dr4[k];
                            moves[nm][1] = dc4[k];
                            ++nm;
                        }
                    }
                    if (nm > 0) {
                        const int pick = std::rand() % nm;
                        nr = r + moves[pick][0];
                        nc = c + moves[pick][1];
                    }
                }
            }
        }

        if (nr == r && nc == c) continue;
        if (nr == player.pos.r && nc == player.pos.c) {
            m.pos.r = nr;
            m.pos.c = nc;
            resolve_combat(i);
        } else {
            m.pos.r = nr;
            m.pos.c = nc;
        }
    }
}

void Game::apply_item_effect(int item_idx) {
    if (item_idx < 0 || item_idx >= item_count) return;
    Item& it = items[item_idx];
    if (!it.active) return;
    it.active = false;

    if (it.type == ITEM_POTION) {
        int before = player.hp;
        player.hp = std::min(player.max_hp, player.hp + 35);
        std::stringstream ss;
        ss << "Picked Potion: HP " << before << " -> " << player.hp;
        add_log(ss.str());
    } else if (it.type == ITEM_SWORD) {
        player.atk += 6;
        add_log("Picked Sword: ATK +6");
    } else if (it.type == ITEM_SHIELD) {
        player.def += 4;
        add_log("Picked Shield: DEF +4");
    }
}

void Game::resolve_combat(int monster_idx) {
    Monster& m = monsters[monster_idx];
    if (!m.alive) return;

    int start_hp = player.hp;
    int rounds = 0;
    while (player.hp > 0 && m.hp > 0) {
        ++rounds;
        m.hp -= std::max(1, player.atk - m.def);
        if (m.hp <= 0) break;
        int dmg = std::max(0, m.atk - player.def);
        player.hp -= dmg;
    }

    if (m.hp <= 0) {
        m.alive = false;
        std::stringstream ss;
        ss << "Defeated " << monster_name(m.type)
           << " in " << rounds << " rounds, HP -" << (start_hp - std::max(0, player.hp));
        add_log(ss.str());
    }

    if (player.hp <= 0) {
        player.hp = 0;
        running = false;
        win = false;
        add_log("You are defeated.");
    } else if (m.type == MON_BOSS && !m.alive) {
        running = false;
        win = true;
        add_log("Boss defeated. You win!");
    }
}

bool Game::try_move(int dr, int dc) {
    int nr = player.pos.r + dr;
    int nc = player.pos.c + dc;
    if (!is_walkable(map, width, height, nr, nc)) {
        add_log("Blocked by wall.");
        return false;
    }
    player.pos = {nr, nc};
    int item_idx = find_item_at(nr, nc);
    if (item_idx >= 0) apply_item_effect(item_idx);
    int mon_idx = find_monster_at(nr, nc);
    if (mon_idx >= 0) resolve_combat(mon_idx);
    return true;
}

bool Game::handle_input(char cmd) {
    if (cmd == 'w' || cmd == 'W') {
        invalid_input_streak = 0;
        const bool step = try_move(-1, 0);
        if (running) tick_monsters();
        return step;
    }
    if (cmd == 's' || cmd == 'S') {
        invalid_input_streak = 0;
        const bool step = try_move(1, 0);
        if (running) tick_monsters();
        return step;
    }
    if (cmd == 'a' || cmd == 'A') {
        invalid_input_streak = 0;
        const bool step = try_move(0, -1);
        if (running) tick_monsters();
        return step;
    }
    if (cmd == 'd' || cmd == 'D') {
        invalid_input_streak = 0;
        const bool step = try_move(0, 1);
        if (running) tick_monsters();
        return step;
    }
    if (cmd == 'q' || cmd == 'Q') {
        invalid_input_streak = 0;
        running = false;
        win = false;
        add_log("Quit game.");
        return true;
    }
    if (cmd == 'p' || cmd == 'P') {
        invalid_input_streak = 0;
        int slot = ask_slot_from_user("Save");
        if (slot <= 0) {
            add_log("Save cancelled: invalid slot.");
            return true;
        }
        std::string path = slot_path(slot);
        if (save_game(path, level, player, map, width, height, monsters, monster_count, items, item_count)) {
            add_log("Game saved to " + path);
        } else {
            add_log("Save failed.");
        }
        return true;
    }
    if (cmd == 'l' || cmd == 'L') {
        invalid_input_streak = 0;
        int slot = ask_slot_from_user("Load");
        if (slot <= 0) {
            add_log("Load cancelled: invalid slot.");
            return true;
        }
        std::string path = slot_path(slot);
        DifficultyLevel loaded_level = level;
        if (load_game(path, loaded_level, player, map, width, height, monsters, monster_count, items, item_count)) {
            level = loaded_level;
            cfg = get_difficulty_config(level);
            add_log("Game loaded from " + path);
        } else {
            add_log("Load failed.");
        }
        return true;
    }
    if (cmd == 'h' || cmd == 'H') {
        invalid_input_streak = 0;
        show_help_screen();
        add_log("Help page closed.");
        return true;
    }
    ++invalid_input_streak;
    std::stringstream ss;
    ss << "Invalid input '" << cmd << "'. Use WASD/P/L/H/Q"
       << " (x" << invalid_input_streak << ")";
    add_log(ss.str());
    return false;
}

void Game::refresh_target_panel() {
    target_info = {};
    int best_idx = -1;
    int best_dist = 1e9;
    for (int i = 0; i < monster_count; ++i) {
        if (!monsters[i].alive) continue;
        int dist = std::abs(monsters[i].pos.r - player.pos.r) + std::abs(monsters[i].pos.c - player.pos.c);
        if (dist < best_dist) {
            best_dist = dist;
            best_idx = i;
        }
    }
    if (best_idx >= 0) {
        target_info.has_target = true;
        target_info.name = monster_name(monsters[best_idx].type);
        target_info.hp = monsters[best_idx].hp;
        target_info.atk = monsters[best_idx].atk;
        target_info.effective_atk = std::max(0, monsters[best_idx].atk - player.def);
    }
}

bool Game::is_goal_defeated() const {
    for (int i = 0; i < monster_count; ++i) {
        if (monsters[i].type == MON_BOSS) return !monsters[i].alive;
    }
    return false;
}

int Game::run() {
    while (running) {
        if (suppress_first_run_draw_) {
            suppress_first_run_draw_ = false;
        } else {
            paint_full_frame();
        }
        if (!running) break;
        int k = -1;
        while (running && (k = read_key_immediate()) == -2) {
        }
        if (!running) break;
        if (k < 0) break;
        char cmd = static_cast<char>(k);
        handle_input(cmd);
    }
    paint_full_frame();
    std::cout << "\nResult: " << (win ? "Victory!" : "Game Over or Quit.") << "\n";
    return win ? 0 : 1;
}
