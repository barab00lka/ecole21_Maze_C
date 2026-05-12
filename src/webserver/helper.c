#include "helper.h"

void
maze_to_json (Maze *maze, cJSON *root)
{
  cJSON_AddNumberToObject (root, "rows", maze->rows);
  cJSON_AddNumberToObject (root, "cols", maze->cols);

  cJSON *hor_ws = cJSON_AddArrayToObject (root, "horizontal_walls"),
        *vert_ws = cJSON_AddArrayToObject (root, "vertical_walls");
  /* read vertical walls first */
  // для каждой строки создаём массив и добавляем числа 0/1
  for (int i = 0; i < maze->rows; i++)
    {
      cJSON *row_arr = cJSON_CreateArray ();
      for (int j = 0; j < maze->cols; j++)
        {
          cJSON_AddItemToArray (
              row_arr, cJSON_CreateNumber (maze->vertical_walls[i][j]));
        }
      cJSON_AddItemToArray (vert_ws, row_arr);
    }
  /* same for horizontal walls */
  for (int i = 0; i < maze->rows; i++)
    {
      cJSON *col_arr = cJSON_CreateArray ();
      for (int j = 0; j < maze->cols; j++)
        {
          cJSON_AddItemToArray (
              col_arr, cJSON_CreateNumber (maze->horizontal_walls[i][j]));
        }
      cJSON_AddItemToArray (hor_ws, col_arr);
    }
}

void
path_to_json (Path *solution, cJSON *root)
{
  cJSON *path_arr = cJSON_AddArrayToObject (root, "path");
  for (int n = 0; n < solution->length; ++n)
    {
      cJSON *step = cJSON_CreateIntArray (
          (int[]){ solution->cells[n].row, solution->cells[n].col }, 2);
      cJSON_AddItemToArray (path_arr, step);
    }
}

void
cave_to_json (Cave *cave, cJSON *root)
{
  cJSON_AddNumberToObject (root, "rows", cave->rows);
  cJSON_AddNumberToObject (root, "cols", cave->cols);

  cJSON *cells = cJSON_AddArrayToObject (root, "cells");

  // для каждой строки создаём массив и добавляем числа 0/1
  for (int i = 0; i < cave->rows; i++)
    {
      cJSON *arr = cJSON_CreateArray ();
      for (int j = 0; j < cave->cols; j++)
        {
          cJSON_AddItemToArray (arr, cJSON_CreateNumber (cave->cells[i][j]));
        }
      cJSON_AddItemToArray (cells, arr);
    }
}
