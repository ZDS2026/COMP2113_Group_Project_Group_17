#include "renderer.h"

#include "common.h"

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
const char* CLR_RESET = "\033[0m";

// 256-color foreground (distinct per type; works on Windows Terminal / VS Code).
const char* monster_type_fg(MonsterType t) {
    switch (t) {
        case MON_SLIME: return "\033[38;5;82m";
        case MON_SKELETON: return "\033[38;5;145m";
        case MON_ORC: return "\033[38;5;64m";
        case MON_WARLOCK: return "\033[38;5;129m";
        case MON_BOSS: return "\033[38;5;196m";
        default: return "\033[38;5;250m";
    }
}

const char* item_type_fg(ItemType t) {
    switch (t) {
        case ITEM_POTION: return "\033[38;5;51m";
        case ITEM_SWORD: return "\033[38;5;220m";
        case ITEM_SHIELD: return "\033[38;5;39m";
        default: return "\033[38;5;250m";
    }
}

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
// Visible width between "| ... |" borders on the HUD panel.
constexpr int kPanelContentWidth = 39;

std::string center_colored_char_in_cell(char ch, int width, const char* fg) {
    if (width <= 0) return "";
    const std::string core = std::string(fg) + ch + CLR_RESET;
    if (width <= 1) return core;
    const int left = (width - 1) / 2;
    const int right = width - 1 - left;
    return std::string(left, ' ') + core + std::string(right, ' ');
}

std::string monster_plain_metrics(MonsterType t, int hp, int eff, int num) {
    std::ostringstream ss;
    ss << monster_symbol(t) << ' ' << pad_right(monster_name(t), 8)
       << " HP:" << std::setw(3) << (hp >= 0 ? std::to_string(hp) : std::string("-"))
       << " ATK:" << std::setw(3) << (eff >= 0 ? std::to_string(eff) : std::string("-"))
       << "  NUM:" << num;
    return ss.str();
}

std::string side_monster_row(MonsterType t, int hp, int eff, int num) {
    const char* fg = monster_type_fg(t);
    const std::string plain = monster_plain_metrics(t, hp, eff, num);
    std::ostringstream o;
    o << "| " << fg << static_cast<char>(monster_symbol(t)) << CLR_RESET << ' '
      << fg << pad_right(monster_name(t), 8) << CLR_RESET
      << " HP:" << std::setw(3) << (hp >= 0 ? std::to_string(hp) : std::string("-"))
      << " ATK:" << std::setw(3) << (eff >= 0 ? std::to_string(eff) : std::string("-"))
      << "  NUM:" << num;
    o << std::string(std::max(0, kPanelContentWidth - static_cast<int>(plain.size())), ' ');
    o << " |";
    return o.str();
}

std::string side_item_legend_row(ItemType it, char letter, const std::string& name_part, const std::string& tail) {
    const char* fg = item_type_fg(it);
    const std::string plain = std::string(1, letter) + ' ' + name_part + tail;
    std::ostringstream o;
    o << "| " << fg << letter << CLR_RESET << ' ' << fg << name_part << CLR_RESET << tail;
    o << std::string(std::max(0, kPanelContentWidth - static_cast<int>(plain.size())), ' ');
    o << " |";
    return o.str();
}

std::string side_items_count_row(int active_potion, int active_sword, int active_shield) {
    const char* fH = item_type_fg(ITEM_POTION);
    const char* fS = item_type_fg(ITEM_SWORD);
    const char* fD = item_type_fg(ITEM_SHIELD);
    std::ostringstream plain_ss;
    plain_ss << "Items H:" << active_potion << " S:" << active_sword << " D:" << active_shield;
    const std::string plain = plain_ss.str();
    std::ostringstream o;
    o << "| Items " << fH << 'H' << CLR_RESET << ':' << active_potion << ' '
      << fS << 'S' << CLR_RESET << ':' << active_sword << ' '
      << fD << 'D' << CLR_RESET << ':' << active_shield;
    o << std::string(std::max(0, kPanelContentWidth - static_cast<int>(plain.size())), ' ');
    o << " |";
    return o.str();
}

std::string side_boss_name_line(bool alive) {
    const char* fg = alive ? monster_type_fg(MON_BOSS) : "\033[38;5;240m";
    const std::string plain = std::string("Name: Ancient Boss");
    std::ostringstream o;
    o << "| Name: " << fg << "Ancient Boss" << CLR_RESET;
    o << std::string(std::max(0, kPanelContentWidth - static_cast<int>(plain.size())), ' ');
    o << " |";
    return o.str();
}

// After moving cursor home and overwriting, shorter new lines leave old text on the
// same row; EL clears from cursor to end of line (fixes "glued" log fragments).
constexpr const char* kClrEol = "\033[0m\033[K";

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
    // Full erase every frame causes visible flicker (blank flash). After the alternate
    // buffer exists, repaint from home only and erase tail once; one initial full clear.
    static bool s_did_initial_alt_clear = false;
    std::ostringstream out;
    if (!s_did_initial_alt_clear) {
        out << "\033[2J\033[3J\033[H";
        s_did_initial_alt_clear = true;
    } else {
        out << "\033[H";
    }

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
    side_lines.push_back(side_monster_row(MON_SLIME, slime_hp, slime_eff, alive_slime));
    side_lines.push_back(side_monster_row(MON_SKELETON, skeleton_hp, skeleton_eff, alive_skeleton));
    side_lines.push_back(side_monster_row(MON_ORC, orc_hp, orc_eff, alive_orc));
    side_lines.push_back(side_monster_row(MON_WARLOCK, warlock_hp, warlock_eff, alive_warlock));
    side_lines.push_back("| " + pad_right(std::string(panel_inner_width - 2, '-'), panel_inner_width - 2) + " |");
    side_lines.push_back("| " + pad_right("Boss Panel", panel_inner_width - 2) + " |");
    if (boss_ptr) {
        side_lines.push_back(side_boss_name_line(true));
        side_lines.push_back("| " + pad_right("HP:   " + std::to_string(boss_ptr->hp), panel_inner_width - 2) + " |");
        side_lines.push_back("| " + pad_right("ATK:  " + std::to_string(boss_ptr->atk), panel_inner_width - 2) + " |");
        side_lines.push_back("| " + pad_right("DEF:  " + std::to_string(boss_ptr->def), panel_inner_width - 2) + " |");
    } else {
        side_lines.push_back(side_boss_name_line(false));
        side_lines.push_back("| " + pad_right("HP:   0", panel_inner_width - 2) + " |");
        side_lines.push_back("| " + pad_right("ATK:  -", panel_inner_width - 2) + " |");
        side_lines.push_back("| " + pad_right("DEF:  -", panel_inner_width - 2) + " |");
    }
    side_lines.push_back("| " + pad_right(std::string(panel_inner_width - 2, '-'), panel_inner_width - 2) + " |");
    side_lines.push_back(side_items_count_row(active_potion, active_sword, active_shield));
    side_lines.push_back(side_item_legend_row(ITEM_POTION, 'H', "Potion", ": +35 HP (up to max HP)"));
    side_lines.push_back(side_item_legend_row(ITEM_SWORD, 'S', "Sword", " : +6 ATK"));
    side_lines.push_back(side_item_legend_row(ITEM_SHIELD, 'D', "Shield", ": +4 DEF"));
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
                    line << center_colored_char_in_cell(
                        monster_symbol(monsters[m_idx].type),
                        kCellCols,
                        monster_type_fg(monsters[m_idx].type));
                    continue;
                }
                int i_idx = find_active_item_at(items, item_count, map_r, c);
                if (i_idx >= 0) {
                    line << center_colored_char_in_cell(
                        item_symbol(items[i_idx].type),
                        kCellCols,
                        item_type_fg(items[i_idx].type));
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
        out << line.str() << kClrEol << "\n";
    }

    out << kClrEol << "\n";
    out << kClrEol << "\n";
    out << "Controls: WASD move | H help | Q quit" << kClrEol << "\n";
    out << "Save: P then slot 1-5 | Load: L then slot 1-5" << kClrEol << "\n";
    out << "Goal: Defeat boss (B) at bottom-right." << kClrEol << "\n";
    out << "Log:" << kClrEol << "\n";
    for (const std::string& msg : logs) {
        out << " - " << msg << kClrEol << "\n";
    }
    // Drop any lines left from a shorter previous frame (e.g. fewer log lines).
    out << "\033[J";
    std::cout << out.str() << std::flush;
}
