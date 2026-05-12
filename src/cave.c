#include "cave.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debug.h"
#include "utils.h"

Cave *
create_cave (int rows, int cols)
{
  if (rows <= 0 || cols <= 0 || rows > CAVE_SIZEMAX || cols > CAVE_SIZEMAX)
    {
      return NULL;
    }

  Cave *cave = (Cave *)safe_malloc (sizeof (Cave));
  cave->rows = rows;
  cave->cols = cols;
  cave->cells = (bool **)create_2d_array (rows, cols, sizeof (bool));

  // Initialize all cells as dead
  for (int i = 0; i < rows; i++)
    {
      for (int j = 0; j < cols; j++)
        {
          cave->cells[i][j] = false;
        }
    }

  return cave;
}

void
destroy_cave (Cave *cave)
{
  if (cave)
    {
      if (cave->cells)
        {
          destroy_2d_array ((void **)cave->cells, cave->rows);
        }
      free (cave);
    }
}

char *
get_cave_scheme (const Cave *cave)
{
  if (!cave)
    {
      fprintf (stderr, "Error: invalid cave\n");
      return NULL;
    }
  if (cave->rows < 1 || cave->rows > CAVE_SIZEMAX || cave->cols < 1
      || cave->cols > CAVE_SIZEMAX)
    {
      fprintf (stderr, "Error cave: invalid dimensions\n");
      return NULL;
    }

  const size_t len = 32 + 2 * 2 * cave->rows * cave->cols;
  char *buf = safe_malloc (len * sizeof (char));
  char *output = buf;
  size_t remaining = len;

  int written
      = snprintf (output, remaining, "%d %d\n", cave->rows, cave->cols);
  if (written < 0 || (size_t)written >= remaining)
    {
      free (buf);
      return NULL;
    }
  output += written;
  remaining -= written;

  for (int i = 0; i < cave->rows; i++)
    {
      for (int j = 0; j < cave->cols; j++)
        {
          written
              = snprintf (output, remaining, "%d ", cave->cells[i][j] ? 1 : 0);
          if (written < 0 || (size_t)written >= remaining)
            {
              free (buf);

              fprintf (stderr, "Error cave cells: cave buffer overflow\n");
              return NULL;
            }
          output += written;
          remaining -= written;
        }
      written = snprintf (output, remaining, "\n");
      if (written < 0 || (size_t)written >= remaining)
        {
          free (buf);

          fprintf (stderr, "Error cave cells: cave buffer overflow\n");
          return NULL;
        }
      output += written;
      remaining -= written;
    }

  written = snprintf (output, remaining, "\n");
  if (written < 0 || (size_t)written >= remaining)
    {
      free (buf);
      fprintf (stderr, "Error cave cells: cave buffer overflow\n");

      return NULL;
    }
  output += written;
  remaining -= written;

  return buf;
}

Cave *
load_cave (const char *content)
{ // cave only
  if (!content)
    return NULL;
  int chars_read = 0;
  int rows = 0, cols = 0;

  if (sscanf (content, "%d %d%n", &rows, &cols, &chars_read) != 2)
    {
      return NULL;
    }

  content += chars_read; // move string pointer forward for next call

  if (rows > CAVE_SIZEMAX || cols > CAVE_SIZEMAX)
    {
      LOG_DEBUG (fprintf (stderr, "invalid cave size: bigger than %dx%d\n",
                          CAVE_SIZEMAX, CAVE_SIZEMAX));
      return NULL;
    }

  Cave *cave = create_cave (rows, cols);
  if (!cave)
    return NULL;

  // для каждой строки создаём массив и добавляем числа 0/1
  int w;
  for (int i = 0; i < rows; i++)
    {
      for (int j = 0; j < cols; j++)
        {
          if (sscanf (content, "%d%n", &w, &chars_read) != 1
              || (w != 0 && w != 1))
            {
              destroy_cave (cave);
              return NULL;
            }
          cave->cells[i][j] = w;
          content += chars_read; // move string pointer forward for next call
        }
    }
  return cave;
}

Cave *
load_cave_from_file (const char *filename)
{
  FILE *file = fopen (filename, "r");
  if (!file)
    {
      return NULL;
    }

  const size_t len = 32 + 2 * CAVE_SIZEMAX * CAVE_SIZEMAX;
  char *buf = safe_malloc (len * sizeof (char));
  size_t read = fread (buf, sizeof (char), len, file);
  if (!read)
    {
      free (buf);
      fclose (file);
      return NULL;
    }
  buf[read] = '\0';
  Cave *cave = load_cave (buf);
  free (buf);
  fclose (file);

  return cave;
}

bool
save_cave_to_file (const Cave *cave, const char *filename)
{
  if (!cave || !filename)
    {
      return false;
    }

  FILE *file = fopen (filename, "w");
  if (!file)
    {
      return false;
    }

  char *buf = get_cave_scheme (cave);
  if (!buf)
    {
      fclose (file);
      return false;
    }
  bool status = true;
  size_t written = fwrite (buf, 1, strlen (buf), file);
  if (written != strlen (buf))
    status = false;

  free (buf);
  fclose (file);
  return status;
}

void
clear_screen ()
{
  printf ("\033[2J\033[H"); // clear screen, move cursor to top-left
  fflush (stdout);
}

void
draw_square_pixel (int color)
{
  // set background color (using 256‑color mode for flexibility)
  printf ("\033[%dm", 40 + color); // or standard 16‑color:
  // printf("\033[48;5;%dm", color);
  // print two spaces to stretch the pixel
  printf ("  ");
  // reset
  printf ("\033[0m");
}

void
render_cave (const Cave *cave)
{
  if (!cave)
    {
      return;
    }

  // Print rows
  for (int i = 0; i < cave->rows; i++)
    {
      for (int j = 0; j < cave->cols; j++)
        {
          if (cave->cells[i][j])
            {
              draw_square_pixel (7);
            }
          else
            {
              draw_square_pixel (4);
            }
        }
      printf ("\n"); // Right wall and newline
    }
  printf ("\n");
}

bool
render_lifecycles (Cave *cave, int birth_limit, int death_limit, int delay,
                   int total_ticks)
{
  bool alive = true;
  for (; total_ticks > 0 && alive; --total_ticks)
    {
      clear_screen ();
      alive = simulate_cave (cave, birth_limit, death_limit);
      render_cave (cave);
      usleep (delay * 1000);
    }

  return alive;
}

int
render_state_interactive (Cave *cave, int birth_limit, int death_limit)
{
  int total_ticks = 0;
  bool alive = true;

  while (getchar () != 'q' && alive)
    {
      alive = render_lifecycles (cave, birth_limit, death_limit, 0, 1);
      ++total_ticks;
    }

  return total_ticks;
}

void
initialize_random_cave (Cave *cave, int chance)
{
  if (!cave || chance < 0 || chance > 100)
    {
      return;
    }

  for (int i = 0; i < cave->rows; i++)
    {
      for (int j = 0; j < cave->cols; j++)
        {
          cave->cells[i][j] = (random_int (1, 100) <= chance);
        }
    }
}
