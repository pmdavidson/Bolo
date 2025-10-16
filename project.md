project.txt

CMPUT 350 - Project Assignment 1 Part 2
Game Title: Bolo
Team Members: Eric Jiang & Peter Davidson
Submission Date: October 15, 2025

Overview
This project is a C++ implementation of a simplified version of the classic Bolo game. The game includes player movement, shooting, walls, explosions, enemies, and enemy bases, as well as a fully featured game engine that supports advanced collision detection.

The game is built on a modular engine that separates gameplay logic (Bolo-specific code) from the rendering and core engine mechanics. It supports dynamic object management, GUI drawing, and accurate collision detection using bounding boxes and shape primitives (rectangles, circles, and lines).

Design Decisions & API Changes
Collision Detection

We fully implemented the upgraded collision system:

Each CollisionObject now returns a list of internal shapes using GetShapes() (e.g., rectangles for walls and bullets, circles for enemies). The engine first performs an AABB (bounding box) check and then tests specific shape combinations for accurate collision detection. The GetCollisionPoint function returns a collision point for applicable shape pairs (e.g., circle-circle, line-line).

Game Flow
The game starts in a waiting state, displaying a maze and awaiting player input. Upon starting, the player enters the gameplay state, where they move, shoot, and destroy enemies and bases. When all bases are destroyed, the game transitions into an end state that shows a congratulatory message before resetting the level. Player death transitions to a game over state, displaying a game-over screen.

Game Context & Notifications
GameContext now includes separate GUI and screen draw contexts, along with a pointer to the NotificationManager. NotificationManager is used to allow communication between objects without direct references. We used notifications minimally, as required by the assignment (e.g., score management and explosion chaining).

Bolo Class Game State Flow
The Bolo class manages the core state machine of the game:

State::WaitingToStart: Displays the maze and waits for the player to select a difficulty.

State::InitializingLevel: Builds the maze, places the player and bases, and sets up the GUI.

State::Gameplay: Main game loop, where the player can move, shoot, and destroy enemies/bases.

State::LevelComplete: All bases destroyed. Shows a congratulatory message before starting the next level.

State::GameOver: Triggered when the player dies. Displays a game-over message.

The Bolo class tracks game state transitions, player lives, remaining bases, score, and resets between levels.

File Overview
Engine Files (Peter)

GameEngine.h/.cpp: Core loop, collision checking, object management.

DrawContext.h/.cpp: Drawing API (unchanged from Part 1, with a new method added).

GameObject.h/.cpp: Base class for all game entities, extended with Kill and ReceiveNotification.

CollisionObject.h/.cpp: Extended to support shapes and collision points.

MathUtil.h: Geometry helpers (lines, circles, etc).

GameContext.h: Includes context for GUI and screen drawing.

NotificationManager.h/.cpp: Used to manage decoupled communication.

Bolo Game Files
Eric:
Base.h/.cpp: Spawns enemies, manages health, handles destruction.

Bolo.h/.cpp: Main game logic and state flow.

Enemy.h/.cpp: Movement and shooting logic for enemy ships.

Player.h/.cpp: Player controls, shooting, movement, GUI tracking.

MathUtil.h: Additional geometric logic (e.g., directions, circle math).


Peter:
Bullet.h/.cpp: Bullet movement, lifespan, collisions (with walls, enemies, explosions).

Explosion.h/.cpp: Handles rendering and lifespan of explosions.

Wall.h/.cpp: Static objects that block bullets, enemies, and players.


Extra Credit
None completed. All features implemented are required components of the base assignment.
