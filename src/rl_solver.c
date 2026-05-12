#include "rl_solver.h"

#include <float.h>
#include <stdlib.h>

#define RL_EPISODES 100000
#define RL_ALPHA 0.1 // learning rate
#define RL_GAMMA 0.9 // discount factor
#define RL_EPS_START 0.9
#define RL_EPS_MIN 0.01
#define RL_EPS_DECAY 0.999

#define REWARD_GOAL 100.0
#define REWARD_STEP -1.0
#define REWARD_WALL -10.0

typedef enum
{
  ERROR = -1,
  UP,
  RIGHT,
  DOWN,
  LEFT
} Action;

// Direction vectors for moving up, right, down, left
const int ddr[] = { -1, 0, 1, 0 };
const int ddc[] = { 0, 1, 0, -1 };

static int
state_of (int row, int col, int cols)
{
  return row * cols + col;
}

static int
is_move_valid (const Maze *maze, int r, int c, Action action)
{
  if (action == UP)
    return r > 0 && !maze->horizontal_walls[r - 1][c];
  if (action == DOWN)
    return r < maze->rows - 1 && !maze->horizontal_walls[r][c];
  if (action == RIGHT)
    return c < maze->cols - 1 && !maze->vertical_walls[r][c];
  if (action == LEFT)
    return c > 0 && !maze->vertical_walls[r][c - 1];
  return 0;
}

static void
qtable_free (double **Q, int n_states)
{
  for (int i = 0; i < n_states; i++)
    {
      free (Q[i]);
    }
  free (Q);
}

static double **
qtable_create (int n_states)
{
  double **Q = calloc (n_states, sizeof (double *));
  if (!Q)
    return NULL;

  for (int i = 0; i < n_states; i++)
    {
      Q[i] = calloc (4, sizeof (double));
      if (!Q[i])
        {
          qtable_free (Q, i);
          return NULL;
        }
    }
  return Q;
}

static Action
qtable_best_action (double **Q, const Maze *maze, int r, int c)
{
  int best = -1;
  double max = -DBL_MAX;

  for (int a = 0; a < 4; a++)
    {
      if (is_move_valid (maze, r, c, a) && Q[r * maze->cols + c][a] > max)
        {
          max = Q[r * maze->cols + c][a];
          best = a;
        }
    }

  return best;
}

static void
train (double **Q, const Maze *maze, int goal_r, int goal_c, int episodes)
{
  double epsilon = RL_EPS_START;

  for (int ep = 0; ep < episodes; ep++)
    {
      int r = rand () % maze->rows;
      int c = rand () % maze->cols;

      int max_steps = maze->rows * maze->cols * 4;
      for (int step = 0; step < max_steps; step++)
        {
          Action a;
          if ((double)rand () / RAND_MAX < epsilon)
            {
              a = rand () % 4;
            }
          else
            {
              a = qtable_best_action (Q, maze, r, c);
            }

          if (!is_move_valid (maze, r, c, a))
            {
              continue;
            }

          int r2 = r + ddr[a], c2 = c + ddc[a];
          double reward
              = (r2 == goal_r && c2 == goal_c) ? REWARD_GOAL : REWARD_STEP;

          Action best_next = qtable_best_action (Q, maze, r2, c2);
          double max_q_next = Q[state_of (r2, c2, maze->cols)][best_next];

          int s = state_of (r, c, maze->cols);
          Q[s][a] += RL_ALPHA * (reward + RL_GAMMA * max_q_next - Q[s][a]);

          r = r2, c = c2;

          if (r == goal_r && c == goal_c)
            {
              break;
            }
        }

      if (epsilon > RL_EPS_MIN)
        {
          epsilon *= RL_EPS_DECAY;
        }
    }
}

static Path *
build_path (double **Q, const Maze *maze, Position start, Position end)
{
  int max_steps = maze->rows * maze->cols;

  Path *path = (Path *)malloc (sizeof (Path));
  path->length = 0;
  path->cells = malloc (max_steps * sizeof (Position));

  Position curr = start;
  while (curr.col != end.col || curr.row != end.row)
    {
      Action a = qtable_best_action (Q, maze, curr.row, curr.col);
      if (a == -1)
        {
          free (path->cells);
          free (path);
          return NULL;
        }

      Position next = { curr.row + ddr[a], curr.col + ddc[a] };
      path->cells[path->length++] = curr;
      curr = next;

      if (path->length >= max_steps)
        {
          free (path->cells);
          free (path);
          return NULL;
        }
    }
  path->cells[path->length++] = end;

  return path;
}

RLAgent *
rl_train(const Maze *maze, Position goal)
{
    if (!maze) {
        return NULL;
    }

    RLAgent *agent = (RLAgent *)malloc(sizeof(RLAgent));
    if (!agent) {
        return NULL;
    }

    agent->n_states = maze->rows * maze->cols;
    agent->goal = goal;
    agent->Q = qtable_create(agent->n_states);

    if (!agent->Q) {
        free(agent);
        return NULL;
    }

    train(agent->Q, maze, goal.row, goal.col, RL_EPISODES);

    return agent;
}

Path *
rl_get_path(const RLAgent *agent, const Maze *maze, Position start)
{
    if (!agent || !maze) {
        return NULL;
    }

    return build_path(agent->Q, maze, start, agent->goal);
}

void
rl_agent_free(RLAgent *agent)
{
    if (!agent) {
        return;
    }

    qtable_free(agent->Q, agent->n_states);
    free(agent);
}
