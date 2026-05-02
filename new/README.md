# COMP2113 / ENGG1340 — Group Project: Mini Magic Tower

Console maze RPG (Magic Tower–style). The player starts at the top-left, moves on a randomly generated map, collects items, fights monsters automatically, and **wins by defeating the Boss** at the bottom-right.

> **Before submission:** replace all `<Name>` / `<UID>` placeholders below with **real names and HKU UIDs** (as required for Stage 2).

---

## 1. Team roster (required)

| Role | Name | UID |
|------|------|-----|
| Member A (Team Leader) | `<Name>` | `<UID>` |
| Member B | `<Name>` | `<UID>` |
| Member C | `<Name>` | `<UID>` |
| Member D | `<Name>` | `<UID>` |
| Member E | `<Name>` | `<UID>` |

---

## 2. Game description (required)

- **Goal:** Reach the bottom-right corner and **defeat the Boss** (`B`).
- **Movement:** `WASD` (single key, no Enter).
- **Combat:** Stepping onto a monster tile triggers **automatic turn-based combat**; results appear in the on-screen log.
- **Stats:** Player **HP / ATK / DEF**; regular monsters show **HP** and **effective damage vs current DEF** in the HUD; the Boss panel shows **HP / ATK / DEF**.
- **Items:** **H** Potion (+HP), **S** Sword (+ATK), **D** Shield (+DEF); picking them up applies effects immediately.
- **Difficulty:** **Easy / Medium / Hard** change map size, monster counts, item counts, and scaling.
- **Persistence:** **Save / load** to slot files `save1.dat` … `save5.dat`.
- **Fairness:** Map generation keeps a **reachable path** to the goal; an internal **simulation** rejects unfair seeds and may adjust balance so a viable route exists.

---

## 3. Course requirement compliance (required mapping)

This table maps the **official project requirements** (as summarized in the course brief: randomness, data structures, dynamic memory, file I/O, multi-file program, multiple difficulties) to **concrete features in this repository**.

| Requirement | How this project satisfies it | Primary files |
|-------------|-------------------------------|----------------|
| **Randomness / random events** | Random maze layout (walls/floors), random placement of monsters and items subject to reachability and playability checks | `map.cpp`, `game.cpp` |
| **Data structures** | `struct`/`enum` for player, monsters, items, positions, difficulty config; `std::vector`, `std::deque`, `std::string` for paths and logs | `common.h`, `game.cpp` |
| **Dynamic memory management** | `char**` map via `new[]`/`delete[]`; monster and item arrays dynamically allocated and freed on restart / load | `map.cpp`, `game.cpp`, `io.cpp` |
| **File input and output** | Full game state serialized to disk; **5 independent save slots** | `io.cpp`, `io.h`, `game.cpp` |
| **Multiple source files** | Modular `.h/.cpp` split by responsibility (see §4) | whole `new/` tree |
| **Multiple difficulty levels** | Three presets with distinct map size and entity parameters | `difficulty.cpp`, `game.cpp` |

**Third-party / non-standard libraries:** **None.** The project uses **ISO C++17** and the **C++ standard library** only (e.g. `<iostream>`, `<fstream>`, `<string>`, `<vector>`, `<deque>`, `<queue>`, `<algorithm>`).

---

## 4. Source file map (multi-file layout)

| File | Responsibility |
|------|------------------|
| `main.cpp` | Entry point, main menu, difficulty / continue / quit, replay loop |
| `game.h` / `game.cpp` | Core game loop, entity placement, combat, path checks, balancing hooks |
| `map.h` / `map.cpp` | Dynamic map allocation, maze-style generation, walkability |
| `renderer.h` / `renderer.cpp` | Terminal UI: map frame, HUD, logs |
| `io.h` / `io.cpp` | Save/load serialization |
| `difficulty.h` / `difficulty.cpp` | Per-difficulty numeric configuration |
| `common.h` / `common.cpp` | Shared types and display helpers |
| `input.h` / `input.cpp` | Immediate single-key input (platform-specific where needed) |
| `Makefile` | Build rule for Linux / academy environment |

---

## 5. Build and run

### Linux / HKU Academy Server (required verification target)

```bash
cd new
make
./mini_magic_tower
```

> **Teaching team expectation:** confirm `make` succeeds on the **course-provided Linux environment** before final submission (do not rely only on Windows).

### Windows (PowerShell, if `make` is unavailable)

```powershell
cd new
$src = (Get-ChildItem -Filter "*.cpp" | ForEach-Object { $_.FullName })
g++ -std=c++17 -Wall -Wextra -g $src -o mini_magic_tower.exe
.\mini_magic_tower.exe
```

**Clean build artifacts (optional):** `make clean` (Linux) or delete `*.o`, `*.exe`, `save*.dat` locally. The repository `.gitignore` excludes binaries and save files from Git.

---

## 6. Controls

### Main menu

| Key | Action |
|-----|--------|
| `1` / `2` / `3` | New game: Easy / Medium / Hard |
| `4` | Continue — then press `1`–`5` for save slot (`Esc` cancels) |
| `Q` | Quit program |

### In-game

| Key | Action |
|-----|--------|
| `W` `A` `S` `D` | Move (single key) |
| `P` then `1`–`5` | Save to slot (`Esc` cancels) |
| `L` then `1`–`5` | Load from slot (`Esc` cancels) |
| `H` | Help screen (any key to return) |
| `Q` | End current run |

---

## 7. Work distribution (for TA / group record)

### Member A (Team Leader) — integration & core systems

- Architecture, `main` / `game` flow, replay, difficulty wiring, map validity & simulation pipeline, integration testing.

### Member B — combat & progression

- Player stats, combat resolution, item effects, combat logs.

### Member C — map & rendering

- Map generation, walkability, HUD layout, terminal rendering.

### Member D — monsters & random content

- Monster types and placement rules, boss behaviour, balance with random maps.

### Member E — I/O & tooling

- Save format, multi-slot files, `Makefile`, `.gitignore`, README maintenance.

---

## 8. Demo video checklist (≤ 3 minutes, typical requirement)

1. **Intro (≈20–30 s):** game name + **all member names and UIDs** (must match README).
2. **Gameplay (≈60 s):** movement, combat, items, HUD (player + monsters + boss).
3. **Save/load (≈40 s):** save to a slot, load from another slot or after restart.
4. **Difficulty (≈20 s):** show at least two different levels or parameters.
5. **Wrap-up (≈20 s):** point to **§3 requirement table** — how the project meets each rubric line.

---

## 9. Submission hygiene

- Fill **§1 Team roster** with real data.
- Run **`make`** on the **academy Linux host** and fix any portability issues before the deadline.
- Do **not** commit `*.exe` or `save*.dat` if your course requires a source-only repo (already listed in `.gitignore`).
