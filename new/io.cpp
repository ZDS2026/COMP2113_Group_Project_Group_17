#include "io.h"
#include "map.h"

#include <fstream>
#include <string>

bool save_game(
    const std::string& path,
    DifficultyLevel level,
    const Player& player,
    char** map,
    int width,
    int height,
    const Monster* monsters,
    int monster_count,
    const Item* items,
    int item_count
) {
    std::ofstream out(path.c_str());
    if (!out.is_open()) return false;

    out << static_cast<int>(level) << "\n";
    out << width << " " << height << "\n";
    out << player.hp << " " << player.max_hp << " " << player.atk << " " << player.def
        << " " << player.pos.r << " " << player.pos.c << "\n";

    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) out << map[r][c];
        out << "\n";
    }

    out << monster_count << "\n";
    for (int i = 0; i < monster_count; ++i) {
        out << static_cast<int>(monsters[i].type) << " "
            << monsters[i].pos.r << " " << monsters[i].pos.c << " "
            << monsters[i].hp << " " << monsters[i].atk << " " << monsters[i].def << " "
            << (monsters[i].alive ? 1 : 0) << "\n";
    }

    out << item_count << "\n";
    for (int i = 0; i < item_count; ++i) {
        out << static_cast<int>(items[i].type) << " "
            << items[i].pos.r << " " << items[i].pos.c << " "
            << (items[i].active ? 1 : 0) << "\n";
    }
    return true;
}

bool load_game(
    const std::string& path,
    DifficultyLevel& level,
    Player& player,
    char**& map,
    int& width,
    int& height,
    Monster*& monsters,
    int& monster_count,
    Item*& items,
    int& item_count
) {
    std::ifstream in(path.c_str());
    if (!in.is_open()) return false;

    int level_raw = 1;
    in >> level_raw;
    level = static_cast<DifficultyLevel>(level_raw);

    int old_h = height;
    if (map && old_h > 0) destroy_map(map, old_h);
    if (monsters) {
        delete[] monsters;
        monsters = nullptr;
    }
    if (items) {
        delete[] items;
        items = nullptr;
    }

    in >> width >> height;
    in >> player.hp >> player.max_hp >> player.atk >> player.def >> player.pos.r >> player.pos.c;

    map = create_map(width, height);
    std::string line;
    std::getline(in, line);
    for (int r = 0; r < height; ++r) {
        if (!std::getline(in, line)) return false;
        for (int c = 0; c < width; ++c) {
            map[r][c] = (c < static_cast<int>(line.size())) ? line[c] : ' ';
        }
    }

    in >> monster_count;
    monsters = new Monster[monster_count];
    for (int i = 0; i < monster_count; ++i) {
        int type_raw, alive_raw;
        in >> type_raw
           >> monsters[i].pos.r >> monsters[i].pos.c
           >> monsters[i].hp >> monsters[i].atk >> monsters[i].def
           >> alive_raw;
        monsters[i].type = static_cast<MonsterType>(type_raw);
        monsters[i].alive = (alive_raw == 1);
    }

    in >> item_count;
    items = new Item[item_count];
    for (int i = 0; i < item_count; ++i) {
        int type_raw, active_raw;
        in >> type_raw >> items[i].pos.r >> items[i].pos.c >> active_raw;
        items[i].type = static_cast<ItemType>(type_raw);
        items[i].active = (active_raw == 1);
    }

    return true;
}
