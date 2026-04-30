#ifndef MT_CONFIG_PARSER_H
#define MT_CONFIG_PARSER_H

#include "difficulty.h"
#include <string>

/**
 * @brief 从指定路径读取难度配置文件，解析为 MT_DifficultyConfig
 * @param filepath 配置文件路径（如 "data/difficulty.cfg"）
 * @return 解析后的难度配置结构体；若文件缺失或格式错误，返回 EASY 默认值
 * @note 仅负责“读取与解析”，不负责决定最终采用哪套配置
 */
MT_DifficultyConfig mt_config_parse(const std::string& filepath);

#endif // MT_CONFIG_PARSER_H