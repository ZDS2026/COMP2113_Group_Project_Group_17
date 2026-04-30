#include "config_parser.h"
#include <fstream>
#include <sstream>
#include <iostream>

MT_DifficultyConfig mt_config_parse(const std::string& filepath) {
    // 默认回退值（对应 EASY 难度，防崩溃保底）
    MT_DifficultyConfig cfg = {10, 10, 1.0f, 0.1f};
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        MT_LOG("Config file not found, using default EASY settings.");
        return cfg;
    }

    std::string line;
    while (std::getline(file, line)) {
        // 跳过注释与空行
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string key;
        if (!(iss >> key)) continue;

        // 键值匹配解析（忽略大小写容错可后续加，此处保持简洁）
        if (key == "WIDTH")      iss >> cfg.map_width;
        else if (key == "HEIGHT") iss >> cfg.map_height;
        else if (key == "MULT")   iss >> cfg.enemy_stat_mult;
        else if (key == "PROB")   iss >> cfg.event_trigger_prob;
    }
    file.close();
    return cfg;
}