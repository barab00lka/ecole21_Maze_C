#ifndef HELPER_H_
#define HELPER_H_

#include "../cave.h"
#include "../maze.h"
#include "../solver.h"
#include "cJSON.h"
void maze_to_json (Maze *maze, cJSON *root);
void path_to_json (Path *solution, cJSON *root);
void cave_to_json (Cave *cave, cJSON *root);
#endif
