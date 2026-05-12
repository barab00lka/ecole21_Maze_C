#pragma once

#include "maze.h"
#include "path.h"
#include "queue.h"

Path *solve_maze (const Maze *maze, Position start, Position end);
