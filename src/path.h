#pragma once

#include "maze.h"
#include "position.h"

#define COLOR_GREEN "\033[32m"
#define COLOR_BOLD_GREEN "\033[1;32m"
#define COLOR_BG_LIGHTGREY "\033[48;5;240m"
#define COLOR_RESET "\033[0m"

#define PATH_HIGHLIGHT COLOR_BG_LIGHTGREY COLOR_BOLD_GREEN

typedef enum
{
  DIR_NONE = 0,
  DIR_UP = 1,
  DIR_DOWN = 2,
  DIR_LEFT = 4,
  DIR_RIGHT = 8
} Direction;

typedef struct
{
  Position *cells;
  int length;
} Path;

void render_path (const Maze *maze, const Path *path);
void destroy_path (Path *path);
void render_maze_with_path (const Maze *maze, const Path *path);
