#include "maze.h"
#include "utils.h"
#include <stdlib.h>

int
dfs_count_cells (const Maze *maze, bool **visited, int row, int col)
{
  if (row < 0 || row >= maze->rows || col < 0 || col >= maze->cols
      || visited[row][col])
    {
      return 0;
    }

  visited[row][col] = true;
  int count = 1;

  if (col + 1 < maze->cols && !maze->vertical_walls[row][col])
    {
      count += dfs_count_cells (maze, visited, row, col + 1);
    }

  if (col > 0 && !maze->vertical_walls[row][col - 1])
    {
      count += dfs_count_cells (maze, visited, row, col - 1);
    }

  if (row + 1 < maze->rows && !maze->horizontal_walls[row][col])
    {
      count += dfs_count_cells (maze, visited, row + 1, col);
    }

  if (row > 0 && !maze->horizontal_walls[row - 1][col])
    {
      count += dfs_count_cells (maze, visited, row - 1, col);
    }

  return count;
}

bool
is_perfect_maze (const Maze *maze)
{
  if (!maze || maze->rows <= 0 || maze->cols <= 0)
    {
      return false;
    }

  bool **visited
      = (bool **)create_2d_array (maze->rows, maze->cols, sizeof (bool));

  for (int i = 0; i < maze->rows; i++)
    {
      for (int j = 0; j < maze->cols; j++)
        {
          visited[i][j] = false;
        }
    }

  int reachable_cells = dfs_count_cells (maze, visited, 0, 0);

  destroy_2d_array ((void **)visited, maze->rows);

  return reachable_cells == maze->rows * maze->cols;
}

Maze *
generate_perfect_maze (int rows, int cols)
{
  if (rows <= 0 || cols <= 0 || rows > MAZE_SIZEMAX || cols > MAZE_SIZEMAX)
    {
      return NULL;
    }

  Maze *maze = create_maze (rows, cols);
  if (!maze)
    {
      return NULL;
    }

  int *set_ids = (int *)safe_malloc (cols * sizeof (int));
  for (int j = 0; j < cols; j++)
    {
      set_ids[j] = j;
    }

  int next_set_id = cols;

  for (int i = 0; i < rows; i++)
    {
      for (int j = 0; j < cols - 1; j++)
        {
          if (set_ids[j] != set_ids[j + 1] && random_int (0, 1) == 1)
            {
              maze->vertical_walls[i][j] = false;

              int old_set = set_ids[j + 1];
              int new_set = set_ids[j];
              for (int k = 0; k < cols; k++)
                {
                  if (set_ids[k] == old_set)
                    {
                      set_ids[k] = new_set;
                    }
                }
            }
        }

      if (i == rows - 1)
        {
          for (int j = 0; j < cols - 1; j++)
            {
              if (set_ids[j] != set_ids[j + 1])
                {
                  maze->vertical_walls[i][j] = false;
                  int old_set = set_ids[j + 1];
                  int new_set = set_ids[j];
                  for (int k = 0; k < cols; k++)
                    {
                      if (set_ids[k] == old_set)
                        {
                          set_ids[k] = new_set;
                        }
                    }
                }
            }
          break;
        }

      bool *will_connect = (bool *)safe_malloc (cols * sizeof (bool));

      for (int j = 0; j < cols; j++)
        {
          will_connect[j] = (random_int (0, 1) == 1);
        }

      for (int j = 0; j < cols; j++)
        {
          int my_set = set_ids[j];
          bool set_has_connection = false;

          for (int k = 0; k < cols; k++)
            {
              if (set_ids[k] == my_set && will_connect[k])
                {
                  set_has_connection = true;
                  break;
                }
            }

          if (!set_has_connection)
            {
              will_connect[j] = true;
            }
        }

      for (int j = 0; j < cols; j++)
        {
          if (will_connect[j])
            {
              maze->horizontal_walls[i][j] = false;
            }
        }

      for (int j = 0; j < cols; j++)
        {
          if (!will_connect[j])
            {
              set_ids[j] = next_set_id++;
            }
        }

      free (will_connect);
    }

  free (set_ids);
  return maze;
}
