#include "cave.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "utils.h"

bool
simulate_cave (Cave *cave, int birth_limit, int death_limit)
{

  // Create a copy of the current state
  bool **new_cells
      = (bool **)create_2d_array (cave->rows, cave->cols, sizeof (bool));

  // For each cell, count live neighbors and apply rules
  for (int i = 0; i < cave->rows; i++)
    {
      for (int j = 0; j < cave->cols; j++)
        {
          // Count live neighbors
          int live_neighbors = 0;

          // Check all 8 neighboring cells
          for (int di = -1; di <= 1; di++)
            {
              for (int dj = -1; dj <= 1; dj++)
                {
                  if (di == 0 && dj == 0)
                    continue; // Skip the cell itself

                  int ni = i + di;
                  int nj = j + dj;

                  // Cells outside the cave are considered alive
                  if (ni < 0 || ni >= cave->rows || nj < 0 || nj >= cave->cols)
                    {
                      live_neighbors++;
                    }
                  else if (cave->cells[ni][nj])
                    {
                      live_neighbors++;
                    }
                }
            }

          // Apply cellular automaton rules
          if (cave->cells[i][j])
            {
              // Cell is alive
              new_cells[i][j] = (live_neighbors >= death_limit);
            }
          else
            {
              // Cell is dead
              new_cells[i][j] = (live_neighbors > birth_limit);
            }
        }
    }

  bool alive = false;
  // Update cave state
  for (int i = 0; i < cave->rows; i++)
    {
      for (int j = 0; j < cave->cols; j++)
        {
          alive |= (cave->cells[i][j] != new_cells[i][j]);
          cave->cells[i][j] = new_cells[i][j];
        }
    }

  // Clean up
  destroy_2d_array ((void **)new_cells, cave->rows);

  return alive; // if grid changed then cave is alive;
}
