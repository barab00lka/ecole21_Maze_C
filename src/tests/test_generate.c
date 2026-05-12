#include <check.h>
#include "../maze.h"
#include "tests.h"

/* Helper: count open (removed) walls - passages between internal cells.
 * In a perfect maze (spanning tree) there must be exactly rows*cols - 1
 * passages regardless of maze shape.
 */
static int
count_passages (const Maze *maze)
{
  int count = 0;

  /* Vertical walls that are open (between columns j and j+1) */
  for (int i = 0; i < maze->rows; i++)
    {
      for (int j = 0; j < maze->cols - 1; j++)
        { /* exclude right boundary */
          if (!maze->vertical_walls[i][j])
            {
              count++;
            }
        }
    }

  /* Horizontal walls that are open (between rows i and i+1) */
  for (int i = 0; i < maze->rows - 1; i++)
    { /* exclude bottom boundary */
      for (int j = 0; j < maze->cols; j++)
        {
          if (!maze->horizontal_walls[i][j])
            {
              count++;
            }
        }
    }

    return count;
}

/* Validity / NULL-safety */

START_TEST (invalid_zero_rows_returns_null)
{
  Maze *maze = generate_perfect_maze (0, 5);
  ck_assert_ptr_null (maze);
}
END_TEST

START_TEST (invalid_zero_cols_returns_null)
{
  Maze *maze = generate_perfect_maze (5, 0);
  ck_assert_ptr_null (maze);
}
END_TEST

START_TEST (invalid_negative_size_returns_null)
{
  Maze *maze = generate_perfect_maze (-1, -1);
  ck_assert_ptr_null (maze);
}
END_TEST

START_TEST (invalid_oversized_returns_null)
{
  Maze *maze = generate_perfect_maze (MAZE_SIZEMAX + 1, MAZE_SIZEMAX + 1);
  ck_assert_ptr_null (maze);
}
END_TEST

/* Dimensions stored correctly */

START_TEST (dimensions_are_stored_correctly)
{
  Maze *maze = generate_perfect_maze (7, 11);
  ck_assert_ptr_nonnull (maze);
  ck_assert_int_eq (maze->rows, 7);
  ck_assert_int_eq (maze->cols, 11);
  destroy_maze (maze);
}
END_TEST

/* is_perfect_maze: connectivity */

START_TEST (generated_maze_is_perfect_16x16)
{
  Maze *maze = generate_perfect_maze (16, 16);
  ck_assert_ptr_nonnull (maze);
  ck_assert (is_perfect_maze (maze));
  destroy_maze (maze);
}
END_TEST

START_TEST (generated_maze_is_perfect_1x1)
{
  Maze *maze = generate_perfect_maze (1, 1);
  ck_assert_ptr_nonnull (maze);
  ck_assert (is_perfect_maze (maze));
  destroy_maze (maze);
}
END_TEST

START_TEST (generated_maze_is_perfect_1xN)
{
  Maze *maze = generate_perfect_maze (1, 10);
  ck_assert_ptr_nonnull (maze);
  ck_assert (is_perfect_maze (maze));
  destroy_maze (maze);
}
END_TEST

START_TEST (generated_maze_is_perfect_Nx1)
{
  Maze *maze = generate_perfect_maze (10, 1);
  ck_assert_ptr_nonnull (maze);
  ck_assert (is_perfect_maze (maze));
  destroy_maze (maze);
}
END_TEST

START_TEST (generated_maze_is_perfect_max_size)
{
  Maze *maze = generate_perfect_maze (MAZE_SIZEMAX, MAZE_SIZEMAX);
  ck_assert_ptr_nonnull (maze);
  ck_assert (is_perfect_maze (maze));
  destroy_maze (maze);
}
END_TEST

/* Spanning-tree edge count: exactly rows*cols - 1 passages */
/* is_perfect_maze checks only connectivity; this test catches loops */

START_TEST (passage_count_equals_spanning_tree)
{
  int rows = 8, cols = 12;
  Maze *maze = generate_perfect_maze (rows, cols);
  ck_assert_ptr_nonnull (maze);

  int passages = count_passages (maze);
  ck_assert_int_eq (passages, rows * cols - 1);

  destroy_maze (maze);
}
END_TEST

START_TEST (passage_count_1x1_is_zero)
{
  Maze *maze = generate_perfect_maze (1, 1);
  ck_assert_ptr_nonnull (maze);
  ck_assert_int_eq (count_passages (maze), 0);
  destroy_maze (maze);
}
END_TEST

/* Boundary walls must always be present */

START_TEST (boundary_walls_are_intact)
{
  int rows = 5, cols = 7;
  Maze *maze = generate_perfect_maze (rows, cols);
  ck_assert_ptr_nonnull (maze);

  /* Right boundary: vertical_walls[i][cols] must be true for every row */
  for (int i = 0; i < rows; i++)
    {
      ck_assert_msg (maze->vertical_walls[i][cols],
                     "Right boundary wall missing at row %d", i);
    }

  /* Bottom boundary: horizontal_walls[rows][j] must be true for every col */
  for (int j = 0; j < cols; j++)
    {
      ck_assert_msg (maze->horizontal_walls[rows][j],
                     "Bottom boundary wall missing at col %d", j);
    }

  destroy_maze (maze);
}
END_TEST

/* Multiple independent calls produce valid (different) mazes */
/* Checks the generator is not returning cached/static state */

START_TEST (multiple_calls_all_valid)
{
  for (int attempt = 0; attempt < 10; attempt++)
    {
      Maze *maze = generate_perfect_maze (10, 10);
      ck_assert_ptr_nonnull (maze);
      ck_assert (is_perfect_maze (maze));
      ck_assert_int_eq (count_passages (maze), 10 * 10 - 1);
      destroy_maze (maze);
    }
}
END_TEST

int
test_generate ()
{
  Suite *s = suite_create ("generate");

  TCase *tc_null = tcase_create ("NullChecks");
  tcase_add_test (tc_null, invalid_zero_rows_returns_null);
  tcase_add_test (tc_null, invalid_zero_cols_returns_null);
  tcase_add_test (tc_null, invalid_negative_size_returns_null);
  tcase_add_test (tc_null, invalid_oversized_returns_null);
  suite_add_tcase (s, tc_null);

  TCase *tc_dims = tcase_create ("Dimensions");
  tcase_add_test (tc_dims, dimensions_are_stored_correctly);
  suite_add_tcase (s, tc_dims);

  TCase *tc_perfect = tcase_create ("Perfectness");
  tcase_add_test (tc_perfect, generated_maze_is_perfect_16x16);
  tcase_add_test (tc_perfect, generated_maze_is_perfect_1x1);
  tcase_add_test (tc_perfect, generated_maze_is_perfect_1xN);
  tcase_add_test (tc_perfect, generated_maze_is_perfect_Nx1);
  tcase_add_test (tc_perfect, generated_maze_is_perfect_max_size);
  tcase_add_test (tc_perfect, multiple_calls_all_valid);
  suite_add_tcase (s, tc_perfect);

  TCase *tc_tree = tcase_create ("SpanningTree");
  tcase_add_test (tc_tree, passage_count_equals_spanning_tree);
  tcase_add_test (tc_tree, passage_count_1x1_is_zero);
  suite_add_tcase (s, tc_tree);

  TCase *tc_walls = tcase_create ("BoundaryWalls");
  tcase_add_test (tc_walls, boundary_walls_are_intact);
  suite_add_tcase (s, tc_walls);

  SRunner *sr = srunner_create (s);
  srunner_run_all (sr, CK_NORMAL);
  int number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);

    return number_failed;
}
