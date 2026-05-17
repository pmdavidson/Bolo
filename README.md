# Bolo

Bolo is a top-down C++/SFML tank shooter built around a small custom game engine. The player drives a tank through a procedurally generated maze, destroys enemy bases, avoids enemies and bullets, and advances through increasingly dense levels.

This repository was originally developed as a two-person course project by **Peter Davidson** and **Eric Jiang**. The current CMake executable target is named `Project1`, but the game itself is **Bolo**.

## Table of Contents

- [Features](#features)
- [Controls](#controls)
- [Project Structure](#project-structure)
- [Architecture Overview](#architecture-overview)
- [Requirements](#requirements)
- [Build and Run](#build-and-run)
- [Debugging with Valgrind](#debugging-with-valgrind)
- [Known Notes](#known-notes)
- [Future Improvements](#future-improvements)
- [Credits](#credits)

## Features

- Top-down tank shooter gameplay built with **C++20** and **SFML 3**
- Procedural maze generation using depth-first search and density-based wall removal
- Level progression with increasing maze density
- Player tank movement, rotation, turret aiming, shooting, and cooldowns
- Enemy bases that spawn enemies and trigger level completion when destroyed
- Enemy movement, shooting, wall interaction, and death events
- Bullet and explosion entities with timed lifecycles
- Two-phase collision pipeline:
  - Broad-phase AABB checks
  - Shape-level collision checks for rectangles, circles, and lines
- Collision-point reporting so bullets and explosions can react at the actual impact location
- Static/dynamic collision partitioning to avoid unnecessary static-static checks
- Modular game engine with separate gameplay, rendering, collision, update, late-update, and notification concerns
- Weak-pointer-based notification system for decoupled events such as score updates and object destruction
- GUI/HUD rendering for score, remaining bases, radar, and direction finder

## Controls

When the game opens, select a starting density/level with the number keys:

| Key | Action |
|---|---|
| `1` - `5` | Select starting maze density / level |

During gameplay:

| Key | Action |
|---|---|
| `W` | Accelerate forward |
| `X` | Reverse / decrease speed |
| `S` | Stop |
| `A` | Rotate tank counterclockwise |
| `D` | Rotate tank clockwise |
| `1` | Rotate turret counterclockwise |
| `2` | Rotate turret clockwise |
| `Space` | Fire bullet |

## Project Structure

```text
.
├── CMakeLists.txt              # Root CMake configuration and executable target
├── main.cpp                    # Program entry point; creates the engine and Bolo game object
├── project.md                  # Original project write-up/design summary
├── parse_valgrind_suppressions.sh
├── engine/                     # Reusable engine layer
│   ├── GameEngine.*            # Main loop, object lifecycle, input dispatch, collision, render phases
│   ├── GameObject.*            # Base class for update/render/input/lifetime hooks
│   ├── CollisionObject.*       # Collision interface and shape-level collision logic
│   ├── DrawContext.*           # Rendering helper API and view/context management
│   ├── GameContext.h           # Shared per-frame context passed to game objects
│   ├── EngineView.h            # Engine iteration/add-object interface
│   ├── MathUtil.h              # Geometry primitives and math helpers
│   ├── NotificationManager.*   # Weak-pointer event notification system
│   └── FontData.h              # Embedded font data
└── bolo/                       # Bolo-specific gameplay layer
    ├── Bolo.*                  # Game state machine, maze generation, level flow, HUD
    ├── Player.*                # Player movement, turret control, shooting, collision
    ├── Bullet.*                # Bullet movement, lifespan, collision/explosion behavior
    ├── Explosion.*             # Explosion rendering and lifetime
    ├── Wall.*                  # Static maze/boundary walls
    ├── Base.*                  # Enemy base health, enemy spawning, destruction event
    └── Enemy.*                 # Enemy movement, shooting, collision, death event
```

## Architecture Overview

### Engine Layer

The `engine/` folder contains reusable systems that are not specific to Bolo:

- `GameEngine` owns the SFML window, update loop, pending object queue, active object list, rendering contexts, and notification manager.
- `GameObject` defines the common lifecycle hooks: `Update`, `LateUpdate`, `RenderBackground`, `RenderForeground`, `HandleKeyEvent`, `Kill`, and `IsAlive`.
- `CollisionObject` extends `GameObject` with bounds, static/dynamic classification, shape lists, collision callbacks, and shape intersection logic.
- `NotificationManager` stores listeners as `std::weak_ptr<GameObject>` so game objects can communicate without owning each other directly.
- `DrawContext` wraps SFML drawing calls and separates gameplay drawing from GUI/HUD drawing.

### Game Loop

Each frame follows this high-level order:

1. Remove dead objects.
2. Add pending objects that were spawned during the previous frame.
3. Poll SFML events and dispatch mapped key presses to game objects.
4. Call `Update` on each object.
5. Split collision objects into static and dynamic groups.
6. Check dynamic-vs-dynamic and dynamic-vs-static collisions.
7. Call `LateUpdate` on each object.
8. Render background objects, foreground objects, debug text, and display the frame.

New objects are added through a pending queue so objects can safely spawn bullets, enemies, and explosions without mutating the active object list mid-iteration.

### Collision System

Collision detection is split into two stages:

1. **Broad phase:** compare object AABBs using `GetBounds()`.
2. **Narrow phase:** compare internal shapes returned by `GetShapes()`.

Supported shape pairs include:

- Line vs. line
- Line vs. circle
- Line vs. rectangle
- Circle vs. circle
- Circle vs. rectangle
- Rectangle vs. rectangle

When possible, collision functions return an impact point. Bullets use this impact point to spawn explosions at the actual collision location rather than at the bullet center.

### Game Layer

The `bolo/` folder contains game-specific objects:

- `Bolo` manages the global game state, procedural maze, level transitions, scoring, and HUD.
- `Player` handles tank movement, turret direction, firing, and death behavior.
- `Base` represents the enemy base target and emits destruction events.
- `Enemy` handles simple enemy movement, collision response, shooting, and destruction events.
- `Bullet` handles projectile movement, lifespan, and collision-triggered explosions.
- `Explosion` renders temporary explosions and expires after a lifetime.
- `Wall` represents static maze and boundary walls.

## Requirements

- CMake 3.10+
- A C++20-compatible compiler
- SFML 3.x, especially the Graphics module

The project was developed with SFML 3. It uses SFML 3-style APIs, so SFML 2.x is not expected to compile without code changes.

## Build and Run

### 1. Clone the repository

```bash
git clone <repo-url>
cd project-2-bugs
```

### 2. Install or expose SFML 3

The CMake configuration first tries to find a system SFML 3 installation.

On macOS with Homebrew, this is usually:

```bash
brew install cmake sfml
```

If you have SFML 3 installed somewhere custom, pass its location to CMake:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/SFML-3.0.0
```

The submitted zip includes an `SFML-3.0.0/` folder at the repository root. The current `engine/CMakeLists.txt` fallback looks for `extern/SFML-3.0.0`, so if CMake cannot find SFML automatically, use one of these options:

```bash
# Option A: point CMake directly at the bundled folder
cmake -S . -B build -DCMAKE_PREFIX_PATH="$PWD/SFML-3.0.0"

# Option B: move/copy the bundled folder to the fallback path expected by CMake
mkdir -p extern
cp -R SFML-3.0.0 extern/SFML-3.0.0
cmake -S . -B build
```

### 3. Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

If you need the custom SFML path, combine it with the configure step:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$PWD/SFML-3.0.0"
cmake --build build
```

### 4. Run

The executable target is currently named `Project1`.

Depending on your CMake generator and platform, the binary may be placed in one of these locations:

```bash
./build/Project1
```

or:

```bash
./build/bin/Project1
```

On macOS/Linux, if the executable cannot find SFML dynamic libraries, set the library path before running:

```bash
# macOS
export DYLD_LIBRARY_PATH="$PWD/SFML-3.0.0/lib:$DYLD_LIBRARY_PATH"
./build/Project1

# Linux
export LD_LIBRARY_PATH="$PWD/SFML-3.0.0/lib:$LD_LIBRARY_PATH"
./build/Project1
```

## Debugging with Valgrind

The repository includes a helper script for generating a suppression file for common SFML/OpenGL/X11-related reports:

```bash
chmod +x parse_valgrind_suppressions.sh
./parse_valgrind_suppressions.sh
```

Then run the game under Valgrind:

```bash
valgrind --leak-check=full --show-leak-kinds=all --suppressions=valgrind_suppressions.txt ./build/Project1
```

or, if your binary is under `build/bin`:

```bash
valgrind --leak-check=full --show-leak-kinds=all --suppressions=valgrind_suppressions.txt ./build/bin/Project1
```

## Known Notes

- The executable target is named `Project1`; this can be renamed in `CMakeLists.txt` if desired.
- The current `SpawnBases` implementation spawns one base per level, even though an older comment mentions six bases.
- The code is tuned around a 30 FPS frame cap using `mWindow.setFramerateLimit(30)`.
- SFML 3 is required. SFML 2.x uses different APIs and may fail to compile.
- The project has no automated test suite in the current repository; validation was done through gameplay testing and memory/debugging tools.
- Some CMake include paths are macOS/Homebrew-oriented, such as `/opt/homebrew/include`. If you are building on Linux or Windows, you may need to adjust those paths or rely on `find_package(SFML 3 ...)`.

## Future Improvements

- Rename the CMake project and executable from `Project1` to `Bolo`.
- Move bundled SFML into `extern/SFML-3.0.0` or update the fallback path in `engine/CMakeLists.txt`.
- Add automated unit tests for geometry, collision detection, and maze generation.
- Add a formal game-over state and restart flow.
- Make base count configurable by level.
- Add sound effects, sprites, menus, and persistent high scores.
- Add CI for CMake configure/build checks.

## Credits

Developed by:

- **Peter Davidson**
- **Eric Jiang**

Original project context: CMPUT 350 Project Assignment 1 Part 2.
