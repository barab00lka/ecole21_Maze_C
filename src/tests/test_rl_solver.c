#include <check.h>
#include <stdlib.h>
#include "../maze.h"
#include "../rl_solver.h"
#include "tests.h"

static Maze *create_simple_maze()
{
    Maze *maze = create_maze(3, 3);
    maze->vertical_walls[0][0]   = false;
    maze->horizontal_walls[0][1] = false;
    maze->horizontal_walls[1][1] = false;
    maze->vertical_walls[2][1]   = false;
    return maze;
}

static Maze *create_isolated_maze()
{
    return create_maze(2, 2);
}

static Maze *create_corridor_maze()
{
    Maze *maze = create_maze(1, 3);
    maze->vertical_walls[0][0] = false;
    maze->vertical_walls[0][1] = false;
    return maze;
}

static Maze *create_vertical_corridor()
{
    Maze *maze = create_maze(3, 1);
    maze->horizontal_walls[0][0] = false;
    maze->horizontal_walls[1][0] = false;
    return maze;
}

static Maze *create_multi_path_maze()
{
    Maze *maze = create_maze(4, 4);
    maze->vertical_walls[0][0] = false;
    maze->vertical_walls[0][1] = false;
    maze->vertical_walls[0][2] = false;
    maze->horizontal_walls[0][3] = false;
    maze->horizontal_walls[1][3] = false;
    maze->horizontal_walls[2][3] = false;
    return maze;
}

START_TEST(test_rl_train_null_maze)
{
    Position goal = {1, 1};
    RLAgent *agent = rl_train(NULL, goal);
    ck_assert_ptr_null(agent);
}
END_TEST

START_TEST(test_rl_train_creates_agent)
{
    Maze *maze = create_simple_maze();
    Position goal = {2, 2};
    RLAgent *agent = rl_train(maze, goal);
    ck_assert_ptr_nonnull(agent);
    ck_assert_ptr_nonnull(agent->Q);
    ck_assert_int_eq(agent->n_states, maze->rows * maze->cols);
    ck_assert_int_eq(agent->goal.row, goal.row);
    ck_assert_int_eq(agent->goal.col, goal.col);
    rl_agent_free(agent);
    destroy_maze(maze);
}
END_TEST

START_TEST(test_rl_train_isolated_maze)
{
    Maze *maze = create_isolated_maze();
    Position goal = {1, 1};
    RLAgent *agent = rl_train(maze, goal);
    ck_assert_ptr_nonnull(agent);
    rl_agent_free(agent);
    destroy_maze(maze);
}
END_TEST

START_TEST(test_rl_get_path_null_agent)
{
    Maze *maze = create_simple_maze();
    Position start = {0, 0};
    Path *path = rl_get_path(NULL, maze, start);
    ck_assert_ptr_null(path);
    destroy_maze(maze);
}
END_TEST

START_TEST(test_rl_get_path_null_maze)
{
    Maze *maze = create_simple_maze();
    Position goal = {2, 2};
    RLAgent *agent = rl_train(maze, goal);
    Position start = {0, 0};
    Path *path = rl_get_path(agent, NULL, start);
    ck_assert_ptr_null(path);
    rl_agent_free(agent);
    destroy_maze(maze);
}
END_TEST

START_TEST(test_rl_get_path_same_start_end)
{
    Maze *maze = create_simple_maze();
    Position goal = {1, 1};
    RLAgent *agent = rl_train(maze, goal);
    Path *path = rl_get_path(agent, maze, goal);
    ck_assert_ptr_nonnull(path);
    ck_assert_int_eq(path->length, 1);
    ck_assert_int_eq(path->cells[0].row, goal.row);
    ck_assert_int_eq(path->cells[0].col, goal.col);
    destroy_path(path);
    rl_agent_free(agent);
    destroy_maze(maze);
}
END_TEST

START_TEST(test_rl_get_path_simple)
{
    Maze *maze = create_simple_maze();
    Position goal = {2, 2};
    RLAgent *agent = rl_train(maze, goal);
    Position start = {0, 0};
    Path *path = rl_get_path(agent, maze, start);
    ck_assert_ptr_nonnull(path);
    ck_assert_int_gt(path->length, 0);
    ck_assert_int_eq(path->cells[0].row, 0);
    ck_assert_int_eq(path->cells[0].col, 0);
    ck_assert_int_eq(path->cells[path->length-1].row, 2);
    ck_assert_int_eq(path->cells[path->length-1].col, 2);
    destroy_path(path);
    rl_agent_free(agent);
    destroy_maze(maze);
}
END_TEST

START_TEST(test_rl_get_path_valid_steps)
{
    Maze *maze = create_simple_maze();
    Position goal = {2, 2};
    RLAgent *agent = rl_train(maze, goal);
    Position start = {0, 0};
    Path *path = rl_get_path(agent, maze, start);
    ck_assert_ptr_nonnull(path);
    for (int i = 0; i < path->length - 1; i++) {
        int r1 = path->cells[i].row,   c1 = path->cells[i].col;
        int r2 = path->cells[i+1].row, c2 = path->cells[i+1].col;
        ck_assert_int_eq(abs(r2 - r1) + abs(c2 - c1), 1);
        if      (r2 == r1 + 1) ck_assert(!maze->horizontal_walls[r1][c1]);
        else if (r2 == r1 - 1) ck_assert(!maze->horizontal_walls[r2][c2]);
        else if (c2 == c1 + 1) ck_assert(!maze->vertical_walls[r1][c1]);
        else if (c2 == c1 - 1) ck_assert(!maze->vertical_walls[r2][c2]);
    }
    destroy_path(path);
    rl_agent_free(agent);
    destroy_maze(maze);
}
END_TEST

START_TEST(test_rl_get_path_no_path)
{
    Maze *maze = create_isolated_maze();
    Position goal = {1, 1};
    RLAgent *agent = rl_train(maze, goal);
    Position start = {0, 0};
    Path *path = rl_get_path(agent, maze, start);
    ck_assert_ptr_null(path);
    rl_agent_free(agent);
    destroy_maze(maze);
}
END_TEST

START_TEST(test_rl_get_path_corridor)
{
    Maze *maze = create_corridor_maze();
    Position goal = {0, 2};
    RLAgent *agent = rl_train(maze, goal);
    Position start = {0, 0};
    Path *path = rl_get_path(agent, maze, start);
    ck_assert_ptr_nonnull(path);
    ck_assert_int_eq(path->cells[path->length-1].row, 0);
    ck_assert_int_eq(path->cells[path->length-1].col, 2);
    destroy_path(path);
    rl_agent_free(agent);
    destroy_maze(maze);
}
END_TEST

START_TEST(test_rl_get_path_vertical_corridor)
{
    Maze *maze = create_vertical_corridor();
    Position goal = {2, 0};
    RLAgent *agent = rl_train(maze, goal);
    Position start = {0, 0};
    Path *path = rl_get_path(agent, maze, start);
    ck_assert_ptr_nonnull(path);
    ck_assert_int_eq(path->cells[path->length-1].row, 2);
    ck_assert_int_eq(path->cells[path->length-1].col, 0);
    destroy_path(path);
    rl_agent_free(agent);
    destroy_maze(maze);
}
END_TEST

START_TEST(test_rl_get_path_in_bounds)
{
    Maze *maze = create_simple_maze();
    Position goal = {2, 2};
    RLAgent *agent = rl_train(maze, goal);
    Position start = {0, 0};
    Path *path = rl_get_path(agent, maze, start);
    ck_assert_ptr_nonnull(path);
    for (int i = 0; i < path->length; i++) {
        ck_assert_int_ge(path->cells[i].row, 0);
        ck_assert_int_ge(path->cells[i].col, 0);
        ck_assert_int_lt(path->cells[i].row, maze->rows);
        ck_assert_int_lt(path->cells[i].col, maze->cols);
    }
    destroy_path(path);
    rl_agent_free(agent);
    destroy_maze(maze);
}
END_TEST

START_TEST(test_rl_get_path_multi_path)
{
    Maze *maze = create_multi_path_maze();
    Position goal = {0, 3};
    RLAgent *agent = rl_train(maze, goal);
    Position start = {0, 0};
    Path *path = rl_get_path(agent, maze, start);
    ck_assert_ptr_nonnull(path);
    ck_assert_int_eq(path->cells[path->length-1].row, 0);
    ck_assert_int_eq(path->cells[path->length-1].col, 3);
    destroy_path(path);
    rl_agent_free(agent);
    destroy_maze(maze);
}
END_TEST

START_TEST(test_rl_agent_multiple_starts)
{
    Maze *maze = create_simple_maze();
    Position goal = {2, 2};
    RLAgent *agent = rl_train(maze, goal);

    Position start1 = {0, 0};
    Path *p1 = rl_get_path(agent, maze, start1);
    ck_assert_ptr_nonnull(p1);
    ck_assert_int_eq(p1->cells[p1->length-1].row, 2);
    ck_assert_int_eq(p1->cells[p1->length-1].col, 2);
    destroy_path(p1);

    Position start2 = {1, 1};
    Path *p2 = rl_get_path(agent, maze, start2);
    ck_assert_ptr_nonnull(p2);
    ck_assert_int_eq(p2->cells[p2->length-1].row, 2);
    ck_assert_int_eq(p2->cells[p2->length-1].col, 2);
    destroy_path(p2);

    rl_agent_free(agent);
    destroy_maze(maze);
}
END_TEST

int test_rl_solver()
{
    Suite *s = suite_create("rl_solver");
    TCase *tc = tcase_create("Core");

    tcase_set_timeout(tc, 120);

    tcase_add_test(tc, test_rl_train_null_maze);
    tcase_add_test(tc, test_rl_train_creates_agent);
    tcase_add_test(tc, test_rl_train_isolated_maze);

    tcase_add_test(tc, test_rl_get_path_null_agent);
    tcase_add_test(tc, test_rl_get_path_null_maze);
    tcase_add_test(tc, test_rl_get_path_same_start_end);
    tcase_add_test(tc, test_rl_get_path_simple);
    tcase_add_test(tc, test_rl_get_path_valid_steps);
    tcase_add_test(tc, test_rl_get_path_no_path);
    tcase_add_test(tc, test_rl_get_path_corridor);
    tcase_add_test(tc, test_rl_get_path_vertical_corridor);
    tcase_add_test(tc, test_rl_get_path_in_bounds);
    tcase_add_test(tc, test_rl_get_path_multi_path);
    tcase_add_test(tc, test_rl_agent_multiple_starts);

    suite_add_tcase(s, tc);

    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    int failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return failed;
}
