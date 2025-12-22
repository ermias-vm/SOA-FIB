# Dig Dug Clone - Game Architecture

## Overview

This is a Dig Dug clone implemented as a user-space application running on the ZeOS educational operating system. The game uses a multi-threaded architecture with separate threads for logic and rendering.

---

## Game Structure

### Rounds/Levels

- The game has **5 predefined levels** (rounds 1-5) defined in [`game_data.c`](../project/game_data.c)
- Maximum rounds: 5 (defined by `MAX_ROUNDS` in [`game_config.h`](../project/include/game_config.h))

Each round has:
- Player starting position
- Enemy spawn locations (Pooka and Fygar enemies)
- Rock positions
- Pre-dug tunnel layouts
- Ghost mode threshold

**Round Progression:**

| Round | Enemies | Rocks | Description |
|-------|---------|-------|-------------|
| 1 | 1 (Pooka) | 4 | Tutorial level |
| 2 | 2 (1 Pooka, 1 Fygar) | 4 | Introduction to Fygar |
| 3 | 3 (2 Pooka, 1 Fygar) | 4 | Increased difficulty |
| 4 | 4 (2 Pooka, 2 Fygar) | 4 | Multi-enemy challenge |
| 5 | 5 (3 Pooka, 2 Fygar) | 4 | Final challenge |

---

## Thread Architecture

The game uses **TWO threads**:

### 1. Logic Thread (Main Thread)

- **File:** [`game.c`](../project/game.c) - `logic_thread_func()`
- Runs in the main process thread
- **Responsibilities:**
  - Frame rate limiting (120 FPS)
  - Input processing
  - Game state updates
  - Scene management (menu, playing, paused, game over)
  - Player movement and actions
  - Enemy AI and movement
  - Collision detection
  - Scoring

### 2. Render Thread (Created Thread)

- **File:** [`game.c`](../project/game.c) - `render_thread_func()`
- Created via `ThreadCreate()` syscall
- **Responsibilities:**
  - Clearing the back buffer
  - Rendering the map
  - Rendering entities (player, enemies, rocks)
  - Rendering UI (HUD, menus)
  - Presenting to screen (double buffering via fd=10)

**Synchronization:**
- `g_frame_ready`: Flag set by logic thread when a frame is ready to render
- `g_running`: Flag to signal both threads to stop

---

## File Structure

### Main Game Files (`project/`)

| File | Description |
|------|-------------|
| [`game.c`](../project/game.c) | Main entry (`game_main`), game loop, thread functions |
| [`game_logic.c`](../project/game_logic.c) | All game logic: player, enemies, rocks, collisions |
| [`game_input.c`](../project/game_input.c) | Input system: keyboard handling, direction tracking |
| [`game_render.c`](../project/game_render.c) | Double-buffered rendering, screen drawing |
| [`game_map.c`](../project/game_map.c) | Map/tile system, terrain management |
| [`game_data.c`](../project/game_data.c) | Level definitions (5 predefined levels) |
| [`game_entities.c`](../project/game_entities.c) | Entity base functions and utilities |
| [`game_ui.c`](../project/game_ui.c) | UI drawing: menus, HUD, overlays |
| [`game_test.c`](../project/game_test.c) | Test functions for game subsystems |

### Header Files (`project/include/`)

| File | Description |
|------|-------------|
| [`game.h`](../project/include/game.h) | Main game interface, global state declarations |
| [`game_types.h`](../project/include/game_types.h) | All game types: Direction, GameScene, Entity, etc. |
| [`game_config.h`](../project/include/game_config.h) | Configuration constants (screen, colors, speeds) |
| [`game_logic.h`](../project/include/game_logic.h) | Logic function prototypes |
| [`game_input.h`](../project/include/game_input.h) | Input system interface |
| [`game_render.h`](../project/include/game_render.h) | Rendering interface |
| [`game_map.h`](../project/include/game_map.h) | Map system interface |
| [`game_data.h`](../project/include/game_data.h) | Level data structures |
| [`game_ui.h`](../project/include/game_ui.h) | UI drawing interface |
| [`times.h`](../project/include/times.h) | Time constants including FPS limiting |
| [`debug.h`](../project/include/debug.h) | Debug flags for enabling/disabling debug output |

---

## Game Flow

```
                         ┌─────────────┐
                         │  game_main  │
                         └──────┬──────┘
                                │
                         ┌──────▼──────┐
                         │  game_init  │
                         │  - render   │
                         │  - input    │
                         │  - logic    │
                         └──────┬──────┘
                                │
              ┌─────────────────┼─────────────────┐
              │                 │                 │
    ┌─────────▼─────────┐       │                 │
    │  ThreadCreate()   │       │                 │
    │  render_thread    │       │                 │
    └─────────┬─────────┘       │                 │
              │                 │                 │
              │         ┌───────▼───────┐         │
              │         │  game_run()   │         │
              │         │ (logic loop)  │         │
              │         └───────┬───────┘         │
              │                 │                 │
              ▼                 ▼                 │
    ┌───────────────┐   ┌───────────────┐         │
    │ RENDER THREAD │   │ LOGIC THREAD  │         │
    │               │   │               │         │
    │ while running │◄──┤ while running │         │
    │ - wait frame  │   │ - wait        │         │
    │ - clear       │   │ - new_frame   │         │
    │ - render      │   │ - input       │         │
    │ - present     │   │ - update      │         │
    └───────────────┘   └───────┬───────┘         │
                                │                 │
                         ┌──────▼──────┐          │
                         │ game_cleanup│          │
                         └─────────────┘          │
                                                  │
                         ┌────────────────────────┘
                         ▼
                    [EXIT GAME]
```

---

## Logic/Movement Calculation

### Who Does What

1. **INPUT PROCESSING** (`input_keyboard_handler` - runs in interrupt context)
   - Keyboard interrupt triggers handler
   - Sets `g_input.direction` and `g_input.held_dir`
   - Tracks key press/release for movement keys
   - Uses `hold_time` for movement detection

2. **FRAME TIMING** (`logic_thread_func` - main thread)
   - `wait_for_next_frame()` ensures 120 FPS
   - Uses `gettime()` and busy-wait to limit frame rate

3. **INPUT FRAME SETUP** (`input_new_frame` - main thread)
   - Resets `move_processed` flag
   - If key is held beyond `HOLD_THRESHOLD`, enables continuous movement

4. **PLAYER MOVEMENT** (`logic_update_player` - main thread)
   - Gets direction from input
   - Checks `speed_counter` for movement timing
   - Movement is cell-by-cell (one cell per `PLAYER_SPEED` ticks)
   - Validates new position against map
   - Digs tunnels if moving into dirt

5. **ENEMY AI** (`logic_update_enemies` - main thread)
   - Each enemy has `speed_counter` for movement timing
   - Pathfinding towards player
   - Ghost mode when stuck for too long
   - Fygar fire breathing with cooldown

6. **COLLISION DETECTION** (`logic_check_*_collision` - main thread)
   - Player vs Enemy collision
   - Player vs Rock collision
   - Enemy vs Rock collision
   - Pump vs Enemy collision

### Movement Timing

- **Player:** moves every `PLAYER_SPEED` ticks (6 ticks at 120 FPS)
- **Pooka:** moves every `POOKA_BASE_SPEED` ticks (16 ticks)
- **Fygar:** moves every `FYGAR_BASE_SPEED` ticks (12 ticks)
- **Ghost mode:** moves every `GHOST_SPEED` ticks (20 ticks)
- Cell-by-cell movement ensures consistent, predictable motion

---

## Key Constants

From [`game_config.h`](../project/include/game_config.h) and [`times.h`](../project/include/times.h):

### Screen

| Constant | Value | Description |
|----------|-------|-------------|
| `SCREEN_WIDTH` | 80 | Screen columns |
| `SCREEN_HEIGHT` | 25 | Screen rows |

### Speeds (ticks between moves)

| Constant | Value | Description |
|----------|-------|-------------|
| `PLAYER_SPEED` | 6 | Player moves every 6 ticks |
| `POOKA_BASE_SPEED` | 16 | Pooka moves every 16 ticks (slower) |
| `FYGAR_BASE_SPEED` | 12 | Fygar moves every 12 ticks (faster) |
| `GHOST_SPEED` | 20 | Ghost mode is slow |

### Frame Rate

| Constant | Value | Description |
|----------|-------|-------------|
| `TARGET_FPS` | 120 | Target frames per second |
| `TICKS_PER_SECOND` | 320 | Adjusted ticks per second |

### Lives and Rounds

| Constant | Value |
|----------|-------|
| `INITIAL_LIVES` | 5 |
| `MAX_LIVES` | 5 |
| `MAX_ROUNDS` | 5 |

### Scoring (by layer depth)

| Layer | Points | Fygar Points (2x) |
|-------|--------|-------------------|
| Layer 1 | 200 | 400 |
| Layer 2 | 300 | 600 |
| Layer 3 | 400 | 800 |
| Layer 4 | 500 | 1000|

### Fygar Attack

| Constant | Value | Description |
|----------|-------|-------------|
| `FYGAR_FIRE_RANGE` | 2 | Fire reaches 2 blocks horizontally |
| `FYGAR_FIRE_COOLDOWN` | 2 seconds | Cooldown between attacks |
| `FYGAR_FIRE_DURATION` | 1/4 second | Fire duration |

### Pump/Inflate

| Constant | Value | Description |
|----------|-------|-------------|
| `INFLATE_LEVELS` | 4 | Levels before enemy explodes |
| `INFLATE_DEFLATE_TIME` | 1 second | Time to deflate one level |
| `PUMP_RANGE` | 3 | Pump reaches 3 block ahead |

---

## Debug Output

Debug prints can be enabled/disabled in [`debug.h`](../project/include/debug.h):

| Flag | Description  |
|------|------------- |
| `DEBUG_GAME_ENABLED`| Master switch for all game debug |
| `DEBUG_GAME_INPUT`  | Input debugging |
| `DEBUG_GAME_PLAYER` | Player state debugging |
| `DEBUG_GAME_ENEMIES`| Enemy AI debugging |
| `DEBUG_GAME_STATE`  | Scene transition debugging |

Debug messages use `printd()` which writes to `FD_DEBUG` (fd=2).
This outputs ONLY to the Bochs terminal (port 0xe9), NOT to the game screen.

---

## Map Structure

### Layer Layout

```
Row 0:   ┌─────────────────────────────────────┐ STATUS BAR (TIME + FPS)
Row 1-2: │           SKY AREA                  │
Row 3:   ├─────────────────────────────────────┤ LAYER 1 START
Row 4-7: │        LAYER 1 (5 rows)             │
Row 8:   ├─────────────────────────────────────┤ LAYER 2 START
Row 8-12 │        LAYER 2 (5 rows)             │
Row 13:  ├─────────────────────────────────────┤ LAYER 3 START
Row 13-17│        LAYER 3 (5 rows)             │
Row 18:  ├─────────────────────────────────────┤ LAYER 4 START
Row 18-22│        LAYER 4 (5 rows)             │
Row 23:  │#####################################│ BORDER
Row 24:  └─────────────────────────────────────┘ STATUS BAR (LIVES + SCORE)
```

### Tile Types

| Type | Character | Description |
|------|-----------|-------------|
| `TILE_EMPTY` | ` ` | Empty tunnel |
| `TILE_DIRT` | `░` | Unexcavated dirt |
| `TILE_SKY` | ` ` | Sky area (black background) |
| `TILE_BORDER` | `#` | Bottom border |

---

## Entity Types

### Player

| Property | Value |
|----------|-------|
| Character | `P` |
| Speed | 6 ticks/move |
| Starting lives | 5 |

### Enemies

| Type | Character | Speed | Special |
|------|-----------|-------|---------|
| Pooka | `O` | 16 ticks/move | Basic chase AI |
| Fygar | `F` | 12 ticks/move | Fire breathing attack |

### Rocks

| Property | Value |
|----------|-------|
| Character | `#` |
| Behavior | Falls when ground below is empty |
| Effect | Crushes enemies and player |

### Bonus Items

Bonus items are **tiles** placed at fixed positions in each level. They are collected when the player walks over them.

| Property | Value |
|----------|-------|
| Character | `+` |
| Tile Type | `TILE_BONUS` |
| Points | 100 |
| Per Level | 3 bonuses |
| Behavior | Disappears when collected |

---

### Commands

| Command | Description |
|---------|-------------|
| `make` | Build the project |
| `make clean` | Clean build artifacts |
| `make emul` | Run in Bochs emulator |
| `make gdb` | Run with GDB debugging |

---

## Related Documentation

- [ZeOS Architecture](ZeOsArchitecture.md) - Operating system internals
- [Bochs Debugger Guide](guides/BochsInternalDebuggerGuide.md) - Internal debugger usage
- [GDB Debugger Guide](guides/GdbExternalDebuggerGuide.md) - External GDB debugging

---

