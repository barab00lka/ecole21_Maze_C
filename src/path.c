#include "path.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

void
render_path (const Maze *maze, const Path *path)
{
  if (!maze || !path || path->length == 0)
    {
      return;
    }

  printf ("Path: ");
  for (int i = 0; i < path->length; i++)
    {
      printf ("(%d,%d)", path->cells[i].row, path->cells[i].col);
      if (i < path->length - 1)
        {
          printf (" -> ");
        }
    }
  printf ("\n\n");

  render_maze_with_path (maze, path);
}

void
destroy_path (Path *path)
{
  if (path)
    {
      if (path->cells)
        {
          free (path->cells);
        }
      free (path);
    }
}

static const char *
get_path_symbol (int dirs, bool is_start, bool is_end)
{
  if (is_start)
    return "S";
  if (is_end)
    return "E";

  switch (dirs)
    {
    case DIR_UP | DIR_DOWN:
      return "║";
    case DIR_LEFT | DIR_RIGHT:
      return "═";
    case DIR_UP | DIR_RIGHT:
      return "╚";
    case DIR_UP | DIR_LEFT:
      return "╝";
    case DIR_DOWN | DIR_RIGHT:
      return "╔";
    case DIR_DOWN | DIR_LEFT:
      return "╗";
    case DIR_UP:
      return "║";
    case DIR_DOWN:
      return "║";
    case DIR_LEFT:
      return "═";
    case DIR_RIGHT:
      return "═";
    default:
      return "●";
    }
}

void
render_maze_with_path (const Maze *maze, const Path *path)
{
  if (!maze)
    return;

  int **path_dirs = NULL;
  if (path && path->length > 0)
    {
      path_dirs
          = (int **)create_2d_array (maze->rows, maze->cols, sizeof (int));
      for (int i = 0; i < maze->rows; i++)
        for (int j = 0; j < maze->cols; j++)
          path_dirs[i][j] = DIR_NONE;

      for (int i = 0; i < path->length - 1; i++)
        {
          Position curr = path->cells[i];
          Position next = path->cells[i + 1];
          int dr = next.row - curr.row;
          int dc = next.col - curr.col;

          if (dr == -1 && dc == 0)
            {
              path_dirs[curr.row][curr.col] |= DIR_UP;
              path_dirs[next.row][next.col] |= DIR_DOWN;
            }
          else if (dr == 1 && dc == 0)
            {
              path_dirs[curr.row][curr.col] |= DIR_DOWN;
              path_dirs[next.row][next.col] |= DIR_UP;
            }
          else if (dr == 0 && dc == -1)
            {
              path_dirs[curr.row][curr.col] |= DIR_LEFT;
              path_dirs[next.row][next.col] |= DIR_RIGHT;
            }
          else if (dr == 0 && dc == 1)
            {
              path_dirs[curr.row][curr.col] |= DIR_RIGHT;
              path_dirs[next.row][next.col] |= DIR_LEFT;
            }
        }
    }

  printf ("+");
  for (int j = 0; j < maze->cols; j++)
    printf ("---+");
  printf ("\n");

  for (int i = 0; i < maze->rows; i++)
    {
      printf ("|");
      for (int j = 0; j < maze->cols; j++)
        {
          if (path_dirs && path_dirs[i][j] != DIR_NONE)
            {
              bool is_start
                  = (i == path->cells[0].row && j == path->cells[0].col);
              bool is_end = (i == path->cells[path->length - 1].row
                             && j == path->cells[path->length - 1].col);
              const char *sym
                  = get_path_symbol (path_dirs[i][j], is_start, is_end);
              if (strcmp (sym, "═") == 0)
                {
                  printf (PATH_HIGHLIGHT "%s%s%s" COLOR_RESET, sym, sym, sym);
                }
              else if (strcmp (sym, "╚") == 0)
                {
                  printf (PATH_HIGHLIGHT " %s%s" COLOR_RESET, sym, "═");
                }
              else if (strcmp (sym, "╝") == 0)
                {
                  printf (PATH_HIGHLIGHT "%s%s " COLOR_RESET, "═", sym);
                }
              else if (strcmp (sym, "╔") == 0)
                {
                  printf (PATH_HIGHLIGHT " %s%s" COLOR_RESET, sym, "═");
                }
              else if (strcmp (sym, "╗") == 0)
                {
                  printf (PATH_HIGHLIGHT "%s%s " COLOR_RESET, "═", sym);
                }
              else
                {
                  printf (PATH_HIGHLIGHT " %s " COLOR_RESET, sym);
                }
            }
          else
            {
              printf ("   ");
            }

          if (maze->vertical_walls[i][j])
            {
              printf ("|");
            }
          else if (path_dirs && (path_dirs[i][j] & DIR_RIGHT))
            {
              printf (PATH_HIGHLIGHT "═" COLOR_RESET);
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
          else if (path_dirs && (path_dirs[i][j] & DIR_DOWN))
            {
              printf (PATH_HIGHLIGHT " ║ " COLOR_RESET "+" COLOR_RESET);
            }
          else
            {
              printf ("   +");
            }
        }
      printf ("\n");
    }

  if (path_dirs)
    {
      destroy_2d_array ((void **)path_dirs, maze->rows);
    }
}
