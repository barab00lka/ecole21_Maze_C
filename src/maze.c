#include "maze.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "utils.h"

Maze *
create_maze (int rows, int cols)
{
  if (rows <= 0 || cols <= 0 || rows > MAZE_SIZEMAX || cols > MAZE_SIZEMAX)
    {
      return NULL;
    }

  Maze *maze = (Maze *)safe_malloc (sizeof (Maze));
  maze->rows = rows;
  maze->cols = cols;

  maze->vertical_walls
      = (bool **)create_2d_array (rows, cols + 1, sizeof (bool));

  maze->horizontal_walls
      = (bool **)create_2d_array (rows + 1, cols, sizeof (bool));

  for (int i = 0; i < rows; i++)
    {
      for (int j = 0; j <= cols; j++)
        {
          maze->vertical_walls[i][j] = true;
        }
    }

  for (int i = 0; i <= rows; i++)
    {
      for (int j = 0; j < cols; j++)
        {
          maze->horizontal_walls[i][j] = true;
        }
    }

  return maze;
}

void
destroy_maze (Maze *maze)
{
  if (maze)
    {
      if (maze->vertical_walls)
        {
          destroy_2d_array ((void **)maze->vertical_walls, maze->rows);
        }
      if (maze->horizontal_walls)
        {
          destroy_2d_array ((void **)maze->horizontal_walls, maze->rows + 1);
        }
      free (maze);
    }
}

Maze *
load_maze (const char *content)
{ // maze only
  if (!content)
    return NULL;
  int chars_read = 0;
  int rows = 0, cols = 0;

  if (sscanf (content, "%d %d%n", &rows, &cols, &chars_read) != 2)
    {
      return NULL;
    }

  if (rows > MAZE_SIZEMAX || cols > MAZE_SIZEMAX)
    {
      LOG_DEBUG (fprintf (stderr, "invalid maze size: bigger than %dx%d\n",
                          MAZE_SIZEMAX, MAZE_SIZEMAX));
      return NULL;
    }
  content += chars_read; // move string pointer forward for next call

  Maze *maze = create_maze (rows, cols);
  if (!maze)
    return NULL;

  /* read vertical walls first */
  // для каждой строки создаём массив и добавляем числа 0/1
  int w;
  for (int i = 0; i < rows; i++)
    {
      for (int j = 0; j < cols; j++)
        {
          if (sscanf (content, "%d%n", &w, &chars_read) != 1
              || (w != 0 && w != 1))
            {
              destroy_maze (maze);
              return NULL;
            }
          maze->vertical_walls[i][j] = w;
          content += chars_read; // move string pointer forward for next call
        }
    }
  /* same for horizontal walls */
  for (int i = 0; i < rows; i++)
    {
      for (int j = 0; j < cols; j++)
        {
          if (sscanf (content, "%d%n", &w, &chars_read) != 1
              || (w != 0 && w != 1))
            {
              destroy_maze (maze);
              return NULL;
            }
          maze->horizontal_walls[i][j] = w;
          content += chars_read; // move string pointer forward for next call
        }
    }

  return maze;
}

Maze *
load_maze_from_file (const char *filename)
{
  FILE *file = fopen (filename, "r");
  if (!file)
    {
      return NULL;
    }

  const size_t len = 32 + 2 * 2 * MAZE_SIZEMAX * MAZE_SIZEMAX;
  char *buf = safe_malloc (len * sizeof (char));
  size_t read = fread (buf, sizeof (char), len, file);
  if (!read)
    {
      free (buf);
      fclose (file);
      return NULL;
    }
  buf[read] = '\0';
  Maze *maze = load_maze (buf);
  free (buf);
  fclose (file);

  return maze;
}

char *
get_maze_scheme (const Maze *maze)
{
  if (!maze)
    {
      fprintf (stderr, "Error: no maze!!\n");
      return NULL;
    }
  if (maze->rows < 1 || maze->rows > MAZE_SIZEMAX || maze->cols < 1
      || maze->cols > MAZE_SIZEMAX)
    {
      fprintf (stderr, "Error: invalid maze\n");
      return NULL;
    }

  const size_t len = 32 + 2 * 2 * 2 * maze->rows * maze->cols;
  char *buf = safe_malloc (len * sizeof (char));
  char *output = buf;
  size_t remaining = len;

  int written
      = snprintf (output, remaining, "%d %d\n", maze->rows, maze->cols);
  if (written < 0 || (size_t)written >= remaining)
    {
      free (buf);

      fprintf (stderr, "Error maze: buffer overflow\n");
      return NULL;
    }
  output += written;
  remaining -= written;

  // Write vertical walls
  for (int i = 0; i < maze->rows; i++)
    {
      for (int j = 0; j < maze->cols; j++)
        {
          written = snprintf (output, remaining, "%d ",
                              maze->vertical_walls[i][j] ? 1 : 0);
          if (written < 0 || (size_t)written >= remaining)
            {
              free (buf);
              fprintf (stderr, "Error vertical_walls: maze buffer overflow\n");
              return NULL;
            }
          output += written;
          remaining -= written;
        }
      written = snprintf (output, remaining, "\n");
      if (written < 0 || (size_t)written >= remaining)
        {
          free (buf);
          fprintf (stderr, "Error vertical_walls: maze buffer overflow\n");
          return NULL;
        }
      output += written;
      remaining -= written;
    }

  written = snprintf (output, remaining, "\n");
  if (written < 0 || (size_t)written >= remaining)
    {
      free (buf);
      fprintf (stderr, "Error vertical_walls: maze buffer overflow\n");

      return NULL;
    }
  output += written;
  remaining -= written;

  // Write horizontal walls
  for (int i = 0; i < maze->rows; i++)
    {
      for (int j = 0; j < maze->cols; j++)
        {
          written = snprintf (output, remaining, "%d ",
                              maze->horizontal_walls[i][j] ? 1 : 0);
          if (written < 0 || (size_t)written >= remaining)
            {
              free (buf);
              fprintf (stderr,
                       "Error horizontal_walls: maze buffer overflow\n");
              return NULL;
            }
          output += written;
          remaining -= written;
        }
      written = snprintf (output, remaining, "\n");
      if (written < 0 || (size_t)written >= remaining)
        {
          free (buf);
          fprintf (stderr, "Error horizontal_walls: maze buffer overflow\n");
          return NULL;
        }
      output += written;
      remaining -= written;
    }

  return buf;
}

bool
save_maze_to_file (const Maze *maze, const char *filename)
{
  if (!maze || !filename)
    {
      return false;
    }

  FILE *file = fopen (filename, "w");
  if (!file)
    {
      return false;
    }

  char *buf = get_maze_scheme (maze);
  if (!buf)
    {
      fclose (file);
      return false;
    }
  bool status = true;
  size_t written = fwrite (buf, sizeof (char), strlen (buf), file);
  if (written != strlen (buf))
    status = false;

  free (buf);
  fclose (file);
  return status;
}

void
render_maze (const Maze *maze)
{
  if (!maze)
    {
      return;
    }

  printf ("+");
  for (int j = 0; j < maze->cols; j++)
    {
      printf ("---+");
    }
  printf ("\n");

  for (int i = 0; i < maze->rows; i++)
    {
      printf ("|");
      for (int j = 0; j < maze->cols; j++)
        {
          printf ("   ");

          if (maze->vertical_walls[i][j])
            {
              printf ("|");
            }
          else
            {
              printf (" ");
            }
        }
      printf ("\n");

      printf ("+");
      for (int j = 0; j < maze->cols; j++)
        {
          if (maze->horizontal_walls[i][j])
            {
              printf ("---+");
            }
          else
            {
              printf ("   +");
            }
        }
      printf ("\n");
    }
}

void
print_2d_int_array (int **array, int rows, int cols, const char *name)
{
  if (name)
    {
      printf ("%s:\n", name);
    }
  for (int i = 0; i < rows; i++)
    {
      for (int j = 0; j < cols; j++)
        {
          printf ("%d ", array[i][j]);
        }
      printf ("\n");
    }
  printf ("\n");
}

void
print_2d_array (bool **array, int rows, int cols, const char *name)
{
  if (name)
    {
      printf ("%s:\n", name);
    }
  for (int i = 0; i < rows; i++)
    {
      for (int j = 0; j < cols; j++)
        {
          printf ("%d ", array[i][j] ? 1 : 0);
        }
      printf ("\n");
    }
  printf ("\n");
}

void
print_maze (const Maze *maze)
{
  printf ("=== Maze ===\n");
  print_2d_array (maze->vertical_walls, maze->rows, maze->cols,
                  "Vertical Walls");
  print_2d_array (maze->horizontal_walls, maze->rows, maze->cols,
                  "Horizontal Walls");
  printf ("===============\n");
}
