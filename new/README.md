# Mini Magic Tower

Text-mode maze RPG inspired by Magic Tower. Move from the top-left, fight monsters, collect items, and defeat the **Boss** at the bottom-right.

## Team

Replace placeholders before you submit.

| | Name | UID |
|--|------|-----|
| A (Leader) | `<Name>` | `<UID>` |
| B | `<Name>` | `<UID>` |
| C | `<Name>` | `<UID>` |
| D | `<Name>` | `<UID>` |
| E | `<Name>` | `<UID>` |

## Work distribution

### A — Team leader (heavier load)

- Overall structure: `main`, `game`, wiring between modules  
- Game loop, replay, difficulty selection, map validation / simulation / reroll  
- Integration, testing, fixing cross-module issues  

### B — Combat & player

- Player HP / ATK / DEF, combat rules, win/lose  
- Items (potion / sword / shield) and combat log text  

### C — Map & UI

- Map generation, walkability  
- Terminal rendering: map frame, HUD, help / hints  

### D — Monsters & balance

- Monster types, stats, placement  
- Boss tuning and random-map balance  

### E — Files & build

- Save/load format, slots `save1.dat` … `save5.dat`  
- `Makefile`, `.gitignore`, README upkeep  

## Features

- Three difficulties (Easy / Medium / Hard)  
- Random map with a viable path; optional simulation / balance passes  
- Auto-combat, items, HUD + boss panel  
- Five save slots, single-key input, in-game help  

## Requirements

- C++17 compiler (`g++` recommended)  
- Standard library only  

## Build

**Linux / macOS**

```bash
make
./mini_magic_tower
```

**Windows (PowerShell)**

```powershell
$src = (Get-ChildItem -Filter "*.cpp" | ForEach-Object { $_.FullName })
g++ -std=c++17 -Wall -Wextra -g $src -o mini_magic_tower.exe
.\mini_magic_tower.exe
```

`make clean` removes objects, binary, and `save*.dat` (see `Makefile`).

## Controls

**Menu:** `1`–`3` new game (easy/medium/hard), `4` continue then slot `1`–`5`, `Q` quit.

**In game:** `WASD` move; `P` / `L` then slot `1`–`5` to save/load; `H` help; `Q` quit run.

## Project layout

| Module | Files |
|--------|--------|
| Entry & menu | `main.cpp` |
| Core logic | `game.cpp`, `game.h` |
| Map | `map.cpp`, `map.h` |
| Draw | `renderer.cpp`, `renderer.h` |
| Save/load | `io.cpp`, `io.h` |
| Difficulty | `difficulty.cpp`, `difficulty.h` |
| Shared types | `common.cpp`, `common.h` |
| Keyboard | `input.cpp`, `input.h` |
