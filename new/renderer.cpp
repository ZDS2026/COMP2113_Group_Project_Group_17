#include "renderer.h"

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

namespace {
const char* CLR_BLUE = "\033[34m";
const char* CLR_RED = "\033[31m";
const char* CLR_RESET = "\033[0m";

// 256-color background (avoid SGR 100: on Windows / VS Code it often becomes bright white).
constexpr int kBgFrame = 237; // outer frame / border bar
constexpr int kBgWall = 238;  // maze wall cells
constexpr int kBgFloor = 234; // walkable floor (subtle dark grey, not default "paper white")

std::string bg_spaces(int visual_cols, int color256) {
    if (visual_cols <= 0) return "";
    std::ostringstream oss;
    oss << "\033[48;5;" << color256 << "m" << std::string(visual_cols, ' ') << "\033[49m\033[0m";
    return oss.str();
}

std::string grey_bar(int visual_cols) { return bg_spaces(visual_cols, kBgFrame); }

std::string grey_frame_row(int outer_visual_cols) { return grey_bar(outer_visual_cols); }

char item_symbol(ItemType t) {
    switch (t) {
        case ITEM_POTION: return 'H';
        case ITEM_SWORD: return 'S';
        case ITEM_SHIELD: return 'D';
        default: return '.';
    }
}

int find_alive_monster_at(const Monster* monsters, int monster_count, int r, int c) {
    for (int i = 0; i < monster_count; ++i) {
        if (monsters[i].alive && monsters[i].pos.r == r && monsters[i].pos.c == c) return i;
    }
    return -1;
}

int find_active_item_at(const Item* items, int item_count, int r, int c) {
    for (int i = 0; i < item_count; ++i) {
        if (items[i].active && items[i].pos.r == r && items[i].pos.c == c) return i;
    }
    return -1;
}

std::string pad_right(const std::string& s, int width) {
    if (static_cast<int>(s.size()) >= width) return s.substr(0, width);
    return s + std::string(width - static_cast<int>(s.size()), ' ');
}

// Terminal cells are taller than wide: use 3 columns per logical cell for a squarer look.
constexpr int kCellCols = 3;

std::string center_char_in_cell(char ch, int width) {
    if (width <= 0) return "";
    if (width == 1) return std::string(1, ch);
    const int left = (width - 1) / 2;
    const int right = width - 1 - left;
    return std::string(left, ' ') + ch + std::string(right, ' ');
}

void ensure_alternate_screen() {
    static std::once_flag once;
    std::call_once(once, []() {
        std::cout << "\033[?1049h" << std::flush;
        (void)std::atexit([]() { std::cout << "\033[?1049l\033[0m" << std::flush; });
    });
}

std::string wall_glyph(char** map, int width, int height, int r, int c) {
    (void)width;
    (void)height;
    (void)r;
    (void)c;
    if (map[r][c] == '#') return bg_spaces(kCellCols, kBgWall);
    if (map[r][c] == '.') return bg_spaces(kCellCols, kBgFloor);
    return center_char_in_cell(map[r][c], kCellCols);
}
}

void draw_frame(
    char** map,
    int width,
    int height,
    const Player& player,
    const Monster* monsters,
    int monster_count,
    const Item* items,
    int item_count,
    const RenderTargetInfo& target_info,
    const std::deque<std::string>& logs
) {
    (void)target_info;
    ensure_alternate_screen();
    // Clear scrollback + home (helps terminals that ignore alt-screen scroll).
    std::cout << "\033[2J\033[3J\033[H" << std::flush;

    int alive_slime = 0, alive_skeleton = 0, alive_orc = 0, alive_warlock = 0, alive_boss = 0;
    const Monster* boss_ptr = nullptr;
    int slime_hp = -1, slime_eff = -1;
    int skeleton_hp = -1, skeleton_eff = -1;
    int orc_hp = -1, orc_eff = -1;
    int warlock_hp = -1, warlock_eff = -1;
    for (int i = 0; i < monster_count; ++i) {
        if (!monsters[i].alive) continue;
        switch (monsters[i].type) {
            case MON_SLIME: ++alive_slime; break;
            case MON_SKELETON: ++alive_skeleton; break;
            case MON_ORC: ++alive_orc; break;
            case MON_WARLOCK: ++alive_warlock; break;
            case MON_BOSS: ++alive_boss; boss_ptr = &monsters[i]; break;
            default: break;
        }
        int eff = std::max(0, monsters[i].atk - player.def);
        if (monsters[i].type == MON_SLIME) { slime_hp = monsters[i].hp; slime_eff = eff; }
        if (monsters[i].type == MON_SKELETON) { skeleton_hp = monsters[i].hp; skeleton_eff = eff; }
        if (monsters[i].type == MON_ORC) { orc_hp = monsters[i].hp; orc_eff = eff; }
        if (monsters[i].type == MON_WARLOCK) { warlock_hp = monsters[i].hp; warlock_eff = eff; }
    }

    int active_potion = 0, active_sword = 0, active_shield = 0;
    for (int i = 0; i < item_count; ++i) {
        if (!items[i].active) continue;
        if (items[i].type == ITEM_POTION) ++active_potion;
        else if (items[i].type == ITEM_SWORD) ++active_sword;
        else if (items[i].type == ITEM_SHIELD) ++active_shield;
    }

    constexpr int panel_inner_width = 41;
    std::vector<std::string> side_lines;
    side_lines.push_back("+-----------------------------------------+");
    side_lines.push_back("| " + pad_right("HUD", panel_inner_width - 2) + " |");
    {
        std::stringstream ss;
        ss << "P HP " << player.hp << "/" << player.max_hp
           << " | ATK " << player.atk
           << " | DEF " << player.def;
        side_lines.push_back("| " + pad_right(ss.str(), panel_inner_width - 2) + " |");
    }
    side_lines.push_back("| " + pad_right(std::string(panel_inner_width - 2, '-'), panel_inner_width - 2) + " |");
    side_lines.push_back("| " + pad_right("Monsters (abbr / name / HP / ATK)", panel_inner_width - 2) + " |");
    {
        std::stringstream ss;
        ss << "s Slime    HP:" << std::setw(3) << (slime_hp >= 0 ? std::to_string(slime_hp) : "-")
           << " ATK:" << std::setw(3) << (slime_eff >= 0 ? std::to_string(slime_eff) : "-")
           << "  NUM:" << alive_slime;
        side_lines.push_back("| " + pad_right(ss.str(), panel_inner_width - 2) + " |");
    }
    {
        std::stringstream ss;
        ss << "k Skeleton HP:" << std::setw(3) << (skeleton_hp >= 0 ? std::to_string(skeleton_hp) : "-")
           << " ATK:" << std::setw(3) << (skeleton_eff >= 0 ? std::to_string(skeleton_eff) : "-")
           << "  NUM:" << alive_skeleton;
        side_lines.push_back("| " + pad_right(ss.str(), panel_inner_width - 2) + " |");
    }
    {
        std::stringstream ss;
        ss << "o Orc      HP:" << std::setw(3) << (orc_hp >= 0 ? std::to_string(orc_hp) : "-")
           << " ATK:" << std::setw(3) << (orc_eff >= 0 ? std::to_string(orc_eff) : "-")
           << "  NUM:" << alive_orc;
        side_lines.push_back("| " + pad_right(ss.str(), panel_inner_width - 2) + " |");
    }
    {
        std::stringstream ss;
        ss << "w Warlock  HP:" << std::setw(3) << (warlock_hp >= 0 ? std::to_string(warlock_hp) : "-")
           << " ATK:" << std::setw(3) << (warlock_eff >= 0 ? std::to_string(warlock_eff) : "-")
           << "  NUM:" << alive_warlock;
        side_lines.push_back("| " + pad_right(ss.str(), panel_inner_width - 2) + " |");
    }
    side_lines.push_back("| " + pad_right(std::string(panel_inner_width - 2, '-'), panel_inner_width - 2) + " |");
    side_lines.push_back("| " + pad_right("Boss Panel", panel_inner_width - 2) + " |");
    if (boss_ptr) {
        side_lines.push_back("| " + pad_right("Name: Ancient Boss", panel_inner_width - 2) + " |");
        side_lines.push_back("| " + pad_right("HP:   " + std::to_string(boss_ptr->hp), panel_inner_width - 2) + " |");
        side_lines.push_back("| " + pad_right("ATK:  " + std::to_string(boss_ptr->atk), panel_inner_width - 2) + " |");
        side_lines.push_back("| " + pad_right("DEF:  " + std::to_string(boss_ptr->def), panel_inner_width - 2) + " |");
    } else {
        side_lines.push_back("| " + pad_right("Name: Ancient Boss", panel_inner_width - 2) + " |");
        side_lines.push_back("| " + pad_right("HP:   0", panel_inner_width - 2) + " |");
        side_lines.push_back("| " + pad_right("ATK:  -", panel_inner_width - 2) + " |");
        side_lines.push_back("| " + pad_right("DEF:  -", panel_inner_width - 2) + " |");
    }
    side_lines.push_back("| " + pad_right(std::string(panel_inner_width - 2, '-'), panel_inner_width - 2) + " |");
    {
        std::stringstream ss;
        ss << "Items H:" << active_potion
           << " S:" << active_sword
           << " D:" << active_shield;
        side_lines.push_back("| " + pad_right(ss.str(), panel_inner_width - 2) + " |");
    }
    side_lines.push_back("| " + pad_right("H Potion: +35 HP (up to max HP)", panel_inner_width - 2) + " |");
    side_lines.push_back("| " + pad_right("S Sword : +6 ATK", panel_inner_width - 2) + " |");
    side_lines.push_back("| " + pad_right("D Shield: +4 DEF", panel_inner_width - 2) + " |");
    side_lines.push_back("+-----------------------------------------+");

    // Map: kCellCols terminal columns per grid cell; grey frame (2 cols each side).
    const int map_cell_cols = kCellCols * width;
    const int map_frame_width = map_cell_cols + 4;
    std::string map_top = grey_frame_row(map_frame_width);
    int map_block_rows = height + 2; // top border + map rows + bottom border
    int rows = std::max(map_block_rows, static_cast<int>(side_lines.size()));
    for (int r = 0; r < rows; ++r) {
        std::stringstream line;
        if (r == 0) {
            line << map_top;
        } else if (r >= 1 && r <= height) {
            line << grey_bar(2);
            int map_r = r - 1;
            for (int c = 0; c < width; ++c) {
                if (player.pos.r == map_r && player.pos.c == c) {
                    line << " " << CLR_BLUE << "@" << CLR_RESET << " ";
                    continue;
                }
                int m_idx = find_alive_monster_at(monsters, monster_count, map_r, c);
                if (m_idx >= 0) {
                    if (monsters[m_idx].type == MON_BOSS) {
                        line << " " << CLR_RED << monster_symbol(monsters[m_idx].type) << CLR_RESET << " ";
                    } else {
                        line << center_char_in_cell(monster_symbol(monsters[m_idx].type), kCellCols);
                    }
                    continue;
                }
                int i_idx = find_active_item_at(items, item_count, map_r, c);
                if (i_idx >= 0) {
                    line << center_char_in_cell(item_symbol(items[i_idx].type), kCellCols);
                    continue;
                }
                line << wall_glyph(map, width, height, map_r, c);
            }
            line << grey_bar(2);
        } else if (r == height + 1) {
            line << map_top;
        } else {
            line << grey_frame_row(map_frame_width);
        }
        line << "   ";
        if (r < static_cast<int>(side_lines.size())) {
            line << side_lines[r];
        }
        std::cout << line.str() << "\n";
    }

    std::cout << "\n\nControls: WASD (instant) | P then 1-5 save | L then 1-5 load | H help | Q quit\n";
    std::cout << "Goal: Defeat boss (B) at bottom-right.\n";
    std::cout << "Log:\n";
    for (const std::string& msg : logs) {
        std::cout << " - " << msg << "\n";
    }
}
