#include <check.h>
#include <stdlib.h>
#include "../maze.h"
#include "../solver.h"
#include "../utils.h"
#include "tests.h"

// Helper function to create a simple 3x3 maze
Maze* create_simple_maze() {
    Maze* maze = create_maze(3, 3);
    
    // Create a simple path from (0,0) to (2,2)
    // Clear walls to create a path: (0,0) -> (0,1) -> (1,1) -> (2,1) -> (2,2)
    
    // Remove wall between (0,0) and (0,1) - vertical wall at (0,0)
    maze->vertical_walls[0][0] = false;
    
    // Remove wall between (0,1) and (1,1) - horizontal wall at (0,1)
    maze->horizontal_walls[0][1] = false;
    
    // Remove wall between (1,1) and (2,1) - horizontal wall at (1,1)
    maze->horizontal_walls[1][1] = false;
    
    // Remove wall between (2,1) and (2,2) - vertical wall at (2,1)
    maze->vertical_walls[2][1] = false;
    
    return maze;
}

// Helper function to create a maze with no path
Maze* create_unsolvable_maze() {
    Maze* maze = create_maze(2, 2);
    
    // All walls are present, so no movement is possible
    // This creates a maze where each cell is isolated
    
    return maze; // All walls are true by default
}

// Test case: solve a simple maze
START_TEST(test_solve_simple_maze) {
    Maze* maze = create_simple_maze();
    Position start = {0, 0};
    Position end = {2, 2};
    
    Path* path = solve_maze(maze, start, end);
    
    // Check that a path was found
    ck_assert_ptr_nonnull(path);
    
    // Check path length (should be 5: (0,0) -> (0,1) -> (1,1) -> (2,1) -> (2,2))
    ck_assert_int_eq(path->length, 5);
    
    // Check start position
    ck_assert_int_eq(path->cells[0].row, 0);
    ck_assert_int_eq(path->cells[0].col, 0);
    
    // Check end position
    ck_assert_int_eq(path->cells[4].row, 2);
    ck_assert_int_eq(path->cells[4].col, 2);
    
    // Verify the path is valid
    for (int i = 0; i < path->length - 1; i++) {
        int row_diff = abs(path->cells[i+1].row - path->cells[i].row);
        int col_diff = abs(path->cells[i+1].col - path->cells[i].col);
        
        // Adjacent cells should differ by exactly 1 in one dimension
        ck_assert_int_eq(row_diff + col_diff, 1);
        
        // Check that there is no wall between consecutive cells
        int curr_row = path->cells[i].row;
        int curr_col = path->cells[i].col;
        int next_row = path->cells[i+1].row;
        int next_col = path->cells[i+1].col;
        
        if (next_row == curr_row + 1) { // Moving down
            ck_assert(!maze->horizontal_walls[curr_row][curr_col]);
        } else if (next_row == curr_row - 1) { // Moving up
            ck_assert(!maze->horizontal_walls[next_row][next_col]);
        } else if (next_col == curr_col + 1) { // Moving right
            ck_assert(!maze->vertical_walls[curr_row][curr_col]);
        } else if (next_col == curr_col - 1) { // Moving left
            ck_assert(!maze->vertical_walls[next_row][next_col]);
        }
    }
    
    // Clean up
    destroy_path(path);
    destroy_maze(maze);
}
END_TEST

// Test case: start and end positions are the same
START_TEST(test_solve_same_start_end) {
    Maze* maze = create_maze(3, 3);
    Position start = {1, 1};
    Position end = {1, 1};
    
    Path* path = solve_maze(maze, start, end);
    
    // Check that a path was found
    ck_assert_ptr_nonnull(path);
    
    // Check path length (should be 1)
    ck_assert_int_eq(path->length, 1);
    
    // Check the position
    ck_assert_int_eq(path->cells[0].row, 1);
    ck_assert_int_eq(path->cells[0].col, 1);
    
    // Clean up
    destroy_path(path);
    destroy_maze(maze);
}
END_TEST

// Test case: invalid maze (null maze)
START_TEST(test_solve_null_maze) {
    Position start = {0, 0};
    Position end = {1, 1};
    
    Path* path = solve_maze(NULL, start, end);
    
    // Should return NULL for invalid maze
    ck_assert_ptr_null(path);
}
END_TEST

// Test case: invalid start position
START_TEST(test_solve_invalid_start) {
    Maze* maze = create_maze(3, 3);
    Position start = {-1, 0};  // Invalid row
    Position end = {1, 1};
    
    Path* path = solve_maze(maze, start, end);
    
    // Should return NULL for invalid start
    ck_assert_ptr_null(path);
    
    // Clean up
    destroy_maze(maze);
}
END_TEST

// Test case: invalid end position
START_TEST(test_solve_invalid_end) {
    Maze* maze = create_maze(3, 3);
    Position start = {0, 0};
    Position end = {5, 5};  // Invalid position
    
    Path* path = solve_maze(maze, start, end);
    
    // Should return NULL for invalid end
    ck_assert_ptr_null(path);
    
    // Clean up
    destroy_maze(maze);
}
END_TEST

// Test case: no path exists
START_TEST(test_solve_no_path) {
    Maze* maze = create_unsolvable_maze();
    Position start = {0, 0};
    Position end = {1, 1};
    
    Path* path = solve_maze(maze, start, end);
    
    // Should return NULL when no path exists
    ck_assert_ptr_null(path);
    
    // Clean up
    destroy_maze(maze);
}
END_TEST

// Test case: path reconstruction correctness
START_TEST(test_solve_path_reconstruction) {
    // Create a maze with a single path
    Maze* maze = create_maze(1, 3);
    
    // Create a path from (0,0) to (0,2)
    maze->vertical_walls[0][0] = false;  // Open wall between (0,0) and (0,1)
    maze->vertical_walls[0][1] = false;  // Open wall between (0,1) and (0,2)
    
    Position start = {0, 0};
    Position end = {0, 2};
    
    Path* path = solve_maze(maze, start, end);
    
    // Check path validity
    ck_assert_ptr_nonnull(path);
    ck_assert_int_eq(path->length, 3);
    
    // Check the sequence of positions
    ck_assert_int_eq(path->cells[0].row, 0); ck_assert_int_eq(path->cells[0].col, 0);
    ck_assert_int_eq(path->cells[1].row, 0); ck_assert_int_eq(path->cells[1].col, 1);
    ck_assert_int_eq(path->cells[2].row, 0); ck_assert_int_eq(path->cells[2].col, 2);
    
    // Clean up
    destroy_path(path);
    destroy_maze(maze);
}
END_TEST

int
test_solve ()
{
  Suite *s = suite_create ("solve");
  TCase *tc_core = tcase_create ("Core");

    tcase_add_test(tc_core, test_solve_simple_maze);
    tcase_add_test(tc_core, test_solve_same_start_end);
    tcase_add_test(tc_core, test_solve_null_maze);
    tcase_add_test(tc_core, test_solve_invalid_start);
    tcase_add_test(tc_core, test_solve_invalid_end);
    tcase_add_test(tc_core, test_solve_no_path);
    tcase_add_test(tc_core, test_solve_path_reconstruction);
    
    suite_add_tcase(s, tc_core);

  SRunner *sr = srunner_create (s);
  srunner_run_all (sr, CK_NORMAL);
  int number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);

    return number_failed;
}
