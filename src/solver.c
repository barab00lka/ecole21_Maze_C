#include <stdbool.h>
#include <stdio.h>

#include "debug.h"
#include "solver.h"
#include "utils.h"

#undef DEBUG
#define DEBUG 0

enum Directions
{
  UP,
  RIGHT,
  DOWN,
  LEFT,
  DIRECTION_COUNT
};

// Direction vectors for moving up, right, down, left
const int dr[] = { -1, 0, 1, 0 };
const int dc[] = { 0, 1, 0, -1 };

Path *
solve_maze (const Maze *maze, Position start, Position end)
{
  if (!maze
      || !is_valid_position (start.row, start.col, maze->rows, maze->cols)
      || !is_valid_position (end.row, end.col, maze->rows, maze->cols))
    {
      return NULL;
    }

  // debug
  LOG_DEBUG (print_maze (maze));

  // Check if start and end are the same
  if (start.row == end.row && start.col == end.col)
    {
      Path *path = (Path *)safe_malloc (sizeof (Path));
      path->cells = (Position *)safe_malloc (sizeof (Position));
      path->cells[0] = start;
      path->length = 1;
      return path;
    }

  // Arrays for BFS
  bool **visited
      = (bool **)create_2d_array (maze->rows, maze->cols, sizeof (bool));
  Position **prev = (Position **)create_2d_array (maze->rows, maze->cols,
                                                  sizeof (Position));

  // Initialize arrays
  for (int i = 0; i < maze->rows; i++)
    {
      for (int j = 0; j < maze->cols; j++)
        {
          visited[i][j] = false;
          prev[i][j].row = -1;
          prev[i][j].col = -1;
        }
    }

  // BFS
  Queue *queue = create_queue (maze->rows * maze->cols);
  enqueue (queue, start);
  visited[start.row][start.col] = true;

  bool found = false;

  while (!is_queue_empty (queue) && !found)
    {
      Position curr = dequeue (queue);

      // Check all four directions
      for (int i = 0; i < 4; i++)
        {
          int nr = curr.row + dr[i];
          int nc = curr.col + dc[i];

          // Check if the new position is valid
          if (!is_valid_position (nr, nc, maze->rows, maze->cols))
            {
              continue;
            }

          // Check if there is a wall between current cell and new cell
          bool can_move = false;
          int direction = i;
          if (direction == UP)
            {
              if (curr.row - 1 >= 0)
                {
                  can_move = !maze->horizontal_walls[curr.row - 1][curr.col];
                }
            }
          else if (direction == RIGHT)
            {
              if (curr.col + 1 < maze->cols)
                can_move = !maze->vertical_walls[curr.row][curr.col];
            }
          else if (direction == DOWN)
            {
              if (curr.row + 1 < maze->rows)
                can_move = !maze->horizontal_walls[curr.row][curr.col];
            }
          else if (direction == LEFT)
            {
              if (curr.col - 1 >= 0)
                {
                  can_move = !maze->vertical_walls[curr.row][curr.col - 1];
                }
            }

          if (can_move && !visited[nr][nc])
            {
              visited[nr][nc] = true;
              prev[nr][nc] = curr;
              Position next = { nr, nc };
              enqueue (queue, next);

              if (nr == end.row && nc == end.col)
                {
                  found = true;
                  break;
                }
            }
        }
    }

  destroy_queue (queue);

  // If no path found
  if (!found)
    {
      destroy_2d_array ((void **)visited, maze->rows);
      destroy_2d_array ((void **)prev, maze->rows);
      return NULL;
    }

  // Reconstruct path
  Path *path = (Path *)safe_malloc (sizeof (Path));

  // First, count the path length
  int length = 0;
  Position curr = end;
  while (!(curr.row == -1 && curr.col == -1))
    {
      length++;
      curr = prev[curr.row][curr.col];
    }

  // Allocate memory for path
  path->length = length;
  path->cells = (Position *)safe_malloc (length * sizeof (Position));

  // Fill path in reverse order
  curr = end;
  for (int i = length - 1; i >= 0; i--)
    {
      path->cells[i] = curr;
      curr = prev[curr.row][curr.col];
    }

  // Clean up
  destroy_2d_array ((void **)visited, maze->rows);
  destroy_2d_array ((void **)prev, maze->rows);

  return path;
}

// Backward compatible wrapper
void
render_maze_ (const Maze *maze)
{
  render_maze_with_path (maze, NULL);
}
