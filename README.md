# COMP2113_Group_Project_Group_17
# Magic Tower - COMP2113/ENGG1340 Project
## Team Members
- Name1 (ID) - Role A
- Name2 (ID) - Role B
...

## Module Ownership (suggested)
- Role A: `main`, `game_controller`, `difficulty`
- Role B: `player`, `combat`
- Role C: `map`, `renderer`
- Role D: `entity`, `event`
- Role E: `file_io`, `config_parser`

## Interface Rules (for team + AI)
- Keep function names and parameter order unchanged unless the team agrees first.
- If a function has `[in,out]` comments, it is allowed to modify that argument.
- `controller` is the only module that orchestrates cross-module calls.
- `renderer` should read state only, and should not change gameplay data.
- For dynamic memory (`new[]`), ensure there is exactly one matching destroy path.
- Add new TODO comments with "owner + expected output" format, e.g. `TODO(Role B): return final damage`.

## Coding Requirements Mapping
1. **Random Events**: Implemented in `src/event.cpp`. Probability scales with difficulty.
2. **Data Structures**: `MT_Player`, `MT_Enemy`, `MT_DifficultyConfig` structs used throughout.
3. **Dynamic Memory**: `mt_map_create/destroy` (2D array), `mt_player_expand_inventory` (1D array resize).
4. **File I/O**: `mt_io_save_game/load_game` serialize state to `data/save.dat`.
5. **Multiple Files**: 10+ `.h/.cpp` files, strictly separated by responsibility.
6. **Multiple Difficulty**: 3 levels in `difficulty.h`, scaling map size, enemy stats, event rates.

## Compilation & Execution
```bash
make clean && make
./magictower
```

On Windows without `make`, you can compile directly:
```bash
g++ -std=c++17 -Wall -Wextra -g src/*.cpp -o magictower.exe
./magictower.exe
```