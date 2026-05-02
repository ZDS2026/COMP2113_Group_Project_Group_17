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
- Non-boss monsters move after each WASD step (type-specific behaviour); Boss stays put; monsters do not enter tiles that still hold an item  
- Five save slots, single-key input, in-game help  

## Features and course coding requirements (1–5)

The project spec asks how **coding elements 1–5** support the features above. This is how they map in our implementation:

| # | Requirement | How it appears in the game | Supporting files / idea |
|---|----------------|---------------------------|-------------------------|
| **1** | **Randomness** (e.g. random events / procedural variation) | Each new run uses a **random seed** (`std::srand` / `std::rand`). The **maze** is carved with random choices; **monsters** and **items** are placed on random floor tiles; monster **types** are rolled by probability; optional **balance jitter** perturbs stats so runs differ. | `map.cpp` (`generate_map_with_path`), `game.cpp` (placement, `random_monster_type`, simulation) |
| **2** | **Data structures** | **Structs** (`Player`, `Monster`, `Item`, `DifficultyConfig`), **enums** (`MonsterType`, `ItemType`, `DifficultyLevel`), and a **2D grid** (`char**`) represent game state, entities, and the map for logic and rendering. | `common.h`, `game.cpp`, `map.cpp` |
| **3** | **Dynamic memory management** | The map is a **heap-allocated 2D array** (`new`/`delete` per row). **Monster** and **item** arrays are sized from difficulty (`new`/`delete` in setup and load). Memory is released when a run ends or the map is rebuilt. | `map.cpp` (`create_map`, `destroy_map`), `game.cpp`, `io.cpp` (load path) |
| **4** | **File input/output** | **Five save slots** (`save1.dat` … `save5.dat`). Save writes player, map, monsters, items, and metadata; load restores a session so the teaching team can use **Continue** after exit. | `io.cpp`, `io.h` (`<fstream>`) |
| **5** | **Multi-file program organization** | Logic is split into **translation units** (menu, game loop, map gen, render, persistence, difficulty, shared types, input) with headers for interfaces—easier to build, test, and assign work across the team. | See **Project layout** below |

Together, (1) gives varied dungeons each play; (2)–(3) hold that state safely at runtime; (4) persists it; (5) keeps the codebase maintainable.

## Non-standard C/C++ libraries

**None.** The game is built with **ISO C++11** and the **C++ standard library only** (for example `<iostream>`, `<fstream>`, `<string>`, `<vector>`, `<cstdlib>` for `rand`/`srand`, etc.). No third-party or course-“non-standard” libraries are linked or vendored in this repo, so **every listed feature** runs on that baseline.

## Requirements

- C++11 compiler (`g++` recommended)  

## Build (quick start)

Open a terminal **in the directory that contains this README and the `.cpp` files** (e.g. if you cloned the repo, `cd` into the project folder that holds `Makefile`). Then:

**Linux / macOS**

```bash
make
./mini_magic_tower
```

**Windows (PowerShell)**

```powershell
$src = (Get-ChildItem -Filter "*.cpp" | ForEach-Object { $_.FullName })
g++ -std=c++11 -Wall -Wextra -g $src -o mini_magic_tower.exe
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
