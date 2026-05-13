
# Double Player Pong Game

A modern, fast-paced 2D Pong-like game built with C++17 and Qt 6.

[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![Qt 6](https://img.shields.io/badge/Qt-6.0+-green.svg)](https://www.qt.io/)
[![CMake](https://img.shields.io/badge/CMake-3.23+-orange.svg)](https://cmake.org/)

## Overview

This project is a graphical 2D Pong game that introduces modern mechanics like **multi-ball**, **ball fusion**, and an **energy accumulation system**. Players control paddles to deflect glowing orbs, competing in either local multiplayer or against an AI.

Originally developed as a C++ course design project at College, it serves as a practical implementation of Object-Oriented Programming, state machine management, and C++/QML integration.

## Quick Start

### Requirements

* A C++17-capable compiler
* CMake 3.23 or newer
* Qt 6 (Core, Gui, Qml, Quick modules)
* `vcpkg` (optional but recommended for dependency management)

### Build the Game

```bash
git clone [https://github.com/XingHE-YX/double-player-playing-ball-game.git](https://github.com/XingHE-YX/double-player-playing-ball-game.git)
cd double-player-playing-ball-game

cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug

./build/debug/DualBallShooter

```


## Core Mechanics

### Game Modes

* **PVP (Player vs Player):** Local dual-player competitive mode.
* **PVE (Player vs Environment):** Play against an AI with three adjustable difficulty levels (Easy, Normal, Hard) based on predictive trajectory algorithms.

### Advanced Physics

* **Multi-Ball System:** The arena can host up to 5 active orbs simultaneously, dynamically increasing the game's pace over time.
* **Fusion System:** When two orbs collide, they fuse into a larger, faster orb, dramatically changing the tactical landscape.
* **Energy System:** Missing a ball grants the opponent energy. The amount of energy scales with the orb's size. The first player to max out their energy bar wins the match.

## Architecture

The project utilizes a modern **C++ Core + QML Presentation Layer** architecture:

* **C++ Logic Layer:** Acts as the game's brain. It handles the state machine, AABB collision detection, physics calculations, AI prediction, and data persistence via `QSettings`.
* **QML UI Layer:** Handles the responsive, dark-themed user interface, including the start menu, dynamic game canvas, pause overlay, and leaderboard rendering.
* The core game loop is driven by a high-precision timer targeting **60 FPS** for smooth physical simulation.

## Controls

Keybindings are fully customizable in the start menu and persist across sessions. Default controls:

| Action | Player 1 (Left) | Player 2 (Right) | Global |
| --- | --- | --- | --- |
| **Move Up** | `W` | `I` | - |
| **Move Down** | `S` | `K` | - |
| **Pause / Resume** | - | - | `Esc` |

## Build Notes & Persistence

* Local user data, including player names, AI difficulty, custom keybinds, and the **Leaderboard**, are automatically saved and loaded using Qt's local storage mechanisms.
* Text input fields automatically suppress game controls to prevent keybinding conflicts.
* Automatic orb spawn delays are implemented to ensure a natural pacing after a player misses a ball.

## Contributors

* **IX (@XingHE-YX)**
* Xie
* Jiang
