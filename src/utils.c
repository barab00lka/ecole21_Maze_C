#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

bool
is_valid_position (int row, int col, int rows, int cols)
{
  return (row >= 0 && row < rows && col >= 0 && col < cols);
}

void *
safe_malloc (size_t size)
{
  void *ptr = malloc (size);
  if (!ptr)
    {
      fprintf (stderr, "Memory allocation failed\n");
      exit (EXIT_FAILURE);
    }
  return ptr;
}

void **
create_2d_array (int rows, int cols, size_t element_size)
{
  void **array = (void **)safe_malloc (rows * sizeof (void *));
  for (int i = 0; i < rows; i++)
    {
      array[i] = safe_malloc (cols * element_size);
    }
  return array;
}

void
destroy_2d_array (void **array, int rows)
{
  if (array)
    {
      for (int i = 0; i < rows; i++)
        {
          if (array[i])
            {
              free (array[i]);
            }
        }
      free (array);
    }
}

int
random_int (int min, int max)
{
  static bool seeded = false;
  if (!seeded)
    {
      srand (time (NULL));
      seeded = true;
    }
  return min + rand () % (max - min + 1);
}