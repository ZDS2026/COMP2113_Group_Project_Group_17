#include "map.h"
#include <cstring>
#include <cstdlib>

char** mt_map_create(int width, int height) {
    char** grid = new char*[height]; // Dynamic allocation
    for(int i=0; i<height; ++i) grid[i] = new char[width];
    return grid;
}

void mt_map_destroy(char**& grid, int height) {
    for(int i=0; i<height; ++i) delete[] grid[i];
    delete[] grid;
    grid = nullptr;
}

void mt_map_generate(char** grid, int w, int h, MT_Difficulty diff) {
    for(int r=0; r<h; ++r)
        for(int c=0; c<w; ++c) grid[r][c] = '.';
    // 四周围墙
    for(int c=0; c<w; ++c) { grid[0][c] = '#'; grid[h-1][c] = '#'; }
    for(int r=0; r<h; ++r) { grid[r][0] = '#'; grid[r][w-1] = '#'; }
    // 随机内部障碍
    int wall_count = (diff==MT_EASY)?10:(diff==MT_MEDIUM)?25:40;
    while(wall_count-->0) {
        int r = rand()% (h-2)+1, c = rand()%(w-2)+1;
        grid[r][c] = '#';
    }
    grid[h-2][w-2] = '>'; // 出口
}

bool mt_map_is_walkable(const char** grid, int w, int h, int r, int c) {
    if(r<0||r>=h||c<0||c>=w) return false;
    return grid[r][c] != '#';
}