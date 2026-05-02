#include "map.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <vector>

char** create_map(int width, int height) {
    char** map = new char*[height];
    for (int r = 0; r < height; ++r) {
        map[r] = new char[width];
    }
    return map;
}

void destroy_map(char**& map, int height) {
    if (!map) return;
    for (int r = 0; r < height; ++r) {
        delete[] map[r];
    }
    delete[] map;
    map = nullptr;
}

void generate_map_with_path(char** map, int width, int height) {
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            map[r][c] = '.';
        }
    }

    int r = 0, c = 0;
    std::vector<std::pair<int, int>> route;
    route.push_back({0, 0});
    while (r != height - 1 || c != width - 1) {
        bool go_down = false;
        if (r == height - 1) go_down = false;
        else if (c == width - 1) go_down = true;
        else go_down = (std::rand() % 2 == 0);
        if (go_down) ++r;
        else ++c;
        route.push_back({r, c});
    }

    int wall_target = (width * height * 42) / 100;
    int wall_now = 0;
    while (wall_now < wall_target) {
        int wr = std::rand() % height;
        int wc = std::rand() % width;
        if ((wr == 0 && wc == 0) || (wr == height - 1 && wc == width - 1)) continue;
        bool on_route = false;
        for (const auto& p : route) {
            if (p.first == wr && p.second == wc) {
                on_route = true;
                break;
            }
        }
        if (on_route) continue;
        if (map[wr][wc] == '#') continue;
        map[wr][wc] = '#';
        ++wall_now;
    }

    int extra_open = (width * height) / 12;
    for (int i = 0; i < extra_open; ++i) {
        int rr = std::rand() % height;
        int cc = std::rand() % width;
        if (map[rr][cc] == '#') map[rr][cc] = '.';
    }

    map[0][0] = '.';
    if (width > 1) map[0][1] = '.';
    if (height > 1) map[1][0] = '.';
    map[height - 1][width - 1] = '.';
    if (width > 1) map[height - 1][width - 2] = '.';
    if (height > 1) map[height - 2][width - 1] = '.';
}

bool is_walkable(char** map, int width, int height, int r, int c) {
    if (r < 0 || c < 0 || r >= height || c >= width) return false;
    return map[r][c] != '#';
}
