# C-Chess

## Overview

C-Chess is a simple chess game implemented in C, featuring a Win32 GUI. The project is divided into several components:


## Features

- Win32 GUI implementation
- Full chess piece movement and validation
- Sprite-based piece rendering
- Turn indicator
- Game state management

## File Structure

| File | Purpose |
|------|---------|
| `main.c` | Entry point, game loop, logic, and UI interactions |
| `board.c/board.h` | Board structure and state management |
| `piece.c/piece.h` | Piece structures and movement validation |
| `sprites.c/sprites.h` | Chess piece sprite loading and rendering |
| `paths.c/paths.h` | Asset and library file paths |
| `utility.c/utility.h` | Helper functions (image decompression, etc.) |

## Screenshots

**Starting Position**

Game at starting position with pieces represented by their respective sprites and turn indicator.

![Game at starting position](https://github.com/Paper-2/C-Chess/blob/main/images/image1.png?raw=true)

**Gameplay**

![Game after some moves](https://github.com/Paper-2/C-Chess/blob/main/images/image2.png?raw=true)
