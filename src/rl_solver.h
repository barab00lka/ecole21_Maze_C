#pragma once

#include "maze.h"
#include "path.h"
#include "solver.h"

typedef struct {
    double **Q;
    int n_states;
    Position goal;
} RLAgent;

RLAgent *rl_train(const Maze *maze, Position goal);

Path *rl_get_path(const RLAgent *agent, const Maze *maze, Position start);

void rl_agent_free(RLAgent *agent);
