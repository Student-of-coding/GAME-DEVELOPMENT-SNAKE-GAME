# GAME-DEVELOPMENT-SNAKE-GAME

*COMPANY*: CODTECH IT SOLUTIONS

*NAME*: VAIBHAV SINGH

*INTERN ID*: CT06DK432

*DOMAIN*: C++ Programming

*DURATION*: 6 WEEKS

*MENTOR*: NEELA SANTOSH


## Description

This is a classic Snake game implemented in C++ using SFML 3. The grid-based gameplay features:

* A growing snake that the player steers with the arrow keys
* Food that appears at random free cells with eating sound effect
* Occasional bonus items that spawn every 15 seconds and expire after 5 seconds
* Procedurally generated obstacles that increase in number each level
* Multiple levels of increasing speed and difficulty
* Life system: you start with 3 lives and lose one on collision with walls, obstacles, or yourself
* Main menu, level selection, pause, and game-over screens with sound effects

It demonstrates real-time rendering, event-driven input handling, and simple game-state management with SFML 3.

## Features

* **Grid & Checkerboard Background**
* **Dynamic Speed**: Snake movement delay scales down by 10% each level
* **Obstacles**: Up to 40 obstacles grow in number per level, avoiding the snake’s spawn area
* **Bonus Items**: Yellow bonus circles worth 50 points, spawning periodically
* **Lives System**: 3 lives, reset to center on collision until lives run out
* **Score & Level Display**: Shown in real time at the top-left
* **Menus & UI**:

  * Main Menu with high-score display
  * Level Select (Levels 1–5)
  * Pause screen with **P** and **M** controls
  * Game Over screen with retry, main-menu, and exit

## Requirements

* **OS**: Windows 10/11
* **IDE**: Visual Studio 2022
* **Compiler**: MSVC (C++17)
* **Libraries**:

  * SFML 3 (Graphics, Window, Audio modules)
  * Standard C++ library

## Build Instructions (Windows)

1. **Clone this repository**

   ```bash
   git clone https://github.com/Student-of-coding/GAME-DEVELOPMENT-SNAKE-GAME.git
   cd GAME-DEVELOPMENT-SNAKE-GAME
   ```

2. **Install SFML 3**

   * Download the SFML SDK for C++17 and extract it, e.g., to `C:\SFML_Snake`.
   * Ensure you use the **x64** version of the libraries for a 64-bit build.

3. **Compile with MSVC (Visual Studio 2022)**

   ```bat
   cl.exe /EHsc /std:c++17 /I"C:\SFML_Snake\include" Source.cpp ^
       /link /LIBPATH:"C:\SFML_Snake\lib\x64" sfml-graphics.lib sfml-window.lib sfml-system.lib sfml-audio.lib
   ```

4. **Resources**
   Ensure these files are in the same folder as the compiled executable:

   * `arial.ttf` (font)
   * `eat.wav` (food/bonus sound)
   * `gameover.wav` (game-over sound)

## Usage

1. Run the generated `SFML_Snake.exe` executable.
2. In **Main Menu**, click **Play** to go to Level Select or **Exit** to quit.
3. Choose a starting level (1–5) or click **Back** to return.
4. Control the snake with **W/A/S/D** or **Arrow Keys**.
5. Press **P** to pause/resume, **M** to return to the menu.
6. Collect white food (+10 points) and yellow bonus (+50 points).
7. Avoid walls, obstacles, and your own tail. You have 3 lives.
8. On **Game Over**, choose **Retry**, **Main Menu**, or **Exit**.

## Code Overview

* **`Source.cpp`**

  * Entry point: loads resources, sets up window and UI, and runs the main loop.
  * **`GameState`** enum: `MainMenu`, `LevelSelect`, `Playing`, `Paused`, `GameOver`.
  * **UI Elements**: `Button` struct, `centerText` helper.
  * **Game Logic**:

    * `isCellFree` lambda checks free cells for food/bonus/obstacles/snake.
    * `spawnFood`, `spawnObstacles`, and bonus-spawn logic.
    * Movement & collision driven by a fixed **moveDelay**, decreasing per level.
    * Score, level progression, and life handling.
  * **Rendering**:

    * Checkerboard grid via nested loops.
    * Draw food, bonus, obstacles, snake segments, borders, and HUD text.
  * **Menus & Screens** drawn with SFML shapes and text.

## Limitations & Future Enhancements

### Known Limitations

* No persistent high-score storage between sessions
* Single fixed window size (no DPI/scaling support)
* No customizable controls or settings menu
* All assets hard-coded (font, sounds)

### Possible Enhancements

* **High-Score File**: Save/load best score to a file
* **Responsive UI**: Support window resizing and high-DPI displays
* **Settings Menu**: Adjust volume, key bindings, or grid size
* **Visual Effects**: Particle trails, animated bonuses, or level transitions
* **Mobile/Touch Controls**: Port to touchscreen devices
* **Multiplayer Mode**: Two-player snake competition

## Screenshots

### Main Menu

![Image](https://github.com/user-attachments/assets/f18bd3cb-1d41-4b70-a53a-16e3f115d183)

### In-Game View (Level 5, Score: 150, Lives: 1)

![Image](https://github.com/user-attachments/assets/43ea8fda-18e9-4780-903a-15ff553b2535)

### Game Over Screen

![Image](https://github.com/user-attachments/assets/6bc918a8-6057-43b6-a8fb-721df9ae2ada)



---

Enjoy the game! 🐍 Feel free to raise issues or submit pull requests for any improvements.
