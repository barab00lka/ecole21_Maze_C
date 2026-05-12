#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "../cave.h"
#include "../utils.h"

/* Helper function to create a test cave with specific dimensions and initialize cells */
static Cave *create_test_cave(int rows, int cols, bool **cell_data) {
    Cave *cave = create_cave(rows, cols);
    if (cave && cell_data) {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                cave->cells[i][j] = cell_data[i][j];
            }
        }
    }
    return cave;
}

/* Helper to create a unique temp filename */
static char *get_unique_temp_filename(const char *prefix) {
    char *tmpl = malloc(strlen(prefix) + 20);
    if (!tmpl) return NULL;
    sprintf(tmpl, "/tmp/%s_XXXXXX", prefix);
    int fd = mkstemp(tmpl);
    if (fd < 0) {
        free(tmpl);
        return NULL;
    }
    close(fd);
    return tmpl;
}

/* ==================== create_cave / destroy_cave tests ==================== */

START_TEST(test_create_destroy_cave) {
    /* Test valid dimensions */
    Cave *cave = create_cave(10, 10);
    ck_assert_ptr_nonnull(cave);
    ck_assert_int_eq(cave->rows, 10);
    ck_assert_int_eq(cave->cols, 10);
    
    /* Check that all cells are initialized to false (dead) */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            ck_assert_int_eq(cave->cells[i][j], false);
        }
    }
    
    destroy_cave(cave);
    
    /* Test invalid dimensions (negative) */
    ck_assert_ptr_null(create_cave(-1, 10));
    ck_assert_ptr_null(create_cave(10, -1));
    
    /* Test zero dimensions */
    ck_assert_ptr_null(create_cave(0, 10));
    ck_assert_ptr_null(create_cave(10, 0));
    
    /* Test dimensions exceeding CAVE_SIZEMAX */
    ck_assert_ptr_null(create_cave(CAVE_SIZEMAX + 1, 10));
    ck_assert_ptr_null(create_cave(10, CAVE_SIZEMAX + 1));
    
    /* Test destroy with NULL - should not crash */
    destroy_cave(NULL);
}
END_TEST

/* ==================== get_cave_scheme tests ==================== */

START_TEST(test_get_cave_scheme) {
    /* Create a 2x2 cave for testing */
    bool **cell_data = (bool **)create_2d_array(2, 2, sizeof(bool));
    cell_data[0][0] = true;
    cell_data[0][1] = false;
    cell_data[1][0] = false;
    cell_data[1][1] = true;
    
    Cave *cave = create_test_cave(2, 2, cell_data);
    ck_assert_ptr_nonnull(cave);
    
    char *scheme = get_cave_scheme(cave);
    ck_assert_ptr_nonnull(scheme);
    
    /* Expected: "rows cols\n" followed by rows of "0/1 " and trailing newline */
    ck_assert_str_eq(scheme, "2 2\n1 0 \n0 1 \n\n");
    
    free(scheme);
    destroy_cave(cave);
    destroy_2d_array((void **)cell_data, 2);
    
    /* Test with NULL cave */
    ck_assert_ptr_null(get_cave_scheme(NULL));
    
    /* Test single cell cave */
    bool **single = (bool **)create_2d_array(1, 1, sizeof(bool));
    single[0][0] = true;
    Cave *c1 = create_test_cave(1, 1, single);
    scheme = get_cave_scheme(c1);
    ck_assert_str_eq(scheme, "1 1\n1 \n\n");
    free(scheme);
    destroy_cave(c1);
    destroy_2d_array((void **)single, 1);
}
END_TEST

/* ==================== load_cave_from_file tests ==================== */

START_TEST(test_load_cave_from_file) {
    /* Use unique temp filename to avoid conflicts */
    char *filename = get_unique_temp_filename("cave_test");
    ck_assert_ptr_nonnull(filename);
    
    /* Create a valid test file */
    FILE *temp_file = fopen(filename, "w");
    ck_assert_ptr_nonnull(temp_file);
    
    fprintf(temp_file, "3 3\n");
    fprintf(temp_file, "1 0 1 \n");
    fprintf(temp_file, "0 1 0 \n");
    fprintf(temp_file, "1 0 1 \n");
    fprintf(temp_file, "\n");
    fclose(temp_file);
    
    Cave *cave = load_cave_from_file(filename);
    ck_assert_ptr_nonnull(cave);
    ck_assert_int_eq(cave->rows, 3);
    ck_assert_int_eq(cave->cols, 3);
    
    /* Verify cell values */
    ck_assert_int_eq(cave->cells[0][0], true);
    ck_assert_int_eq(cave->cells[0][1], false);
    ck_assert_int_eq(cave->cells[0][2], true);
    ck_assert_int_eq(cave->cells[1][0], false);
    ck_assert_int_eq(cave->cells[1][1], true);
    ck_assert_int_eq(cave->cells[1][2], false);
    ck_assert_int_eq(cave->cells[2][0], true);
    ck_assert_int_eq(cave->cells[2][1], false);
    ck_assert_int_eq(cave->cells[2][2], true);
    
    destroy_cave(cave);
    
    /* Test with non-existent file */
    ck_assert_ptr_null(load_cave_from_file("/nonexistent/path/file.txt"));
    
    /* Test with invalid file format (missing dimensions) */
    char *bad_file = get_unique_temp_filename("cave_bad");
    ck_assert_ptr_nonnull(bad_file);
    temp_file = fopen(bad_file, "w");
    ck_assert_ptr_nonnull(temp_file);
    fprintf(temp_file, "invalid\n");
    fclose(temp_file);
    
    ck_assert_ptr_null(load_cave_from_file(bad_file));
    
    /* Test with truncated cell data */
    char *trunc_file = get_unique_temp_filename("cave_trunc");
    ck_assert_ptr_nonnull(trunc_file);
    temp_file = fopen(trunc_file, "w");
    ck_assert_ptr_nonnull(temp_file);
    fprintf(temp_file, "2 2\n1 0\n");  /* Missing one value */
    fclose(temp_file);
    
    ck_assert_ptr_null(load_cave_from_file(trunc_file));
    
    /* Clean up */
    unlink(filename);
    unlink(bad_file);
    unlink(trunc_file);
    free(filename);
    free(bad_file);
    free(trunc_file);
}
END_TEST

/* ==================== simulate_cave tests ==================== */

START_TEST(test_simulate_cave_birth_rule) {
    /* Test: dead cell becomes alive when live_neighbors > birth_limit
     * Note: out-of-bounds cells count as alive in this implementation */
    
    /* 1x1 cave: dead cell has 8 out-of-bounds neighbors = 8 live */
    bool **cells = (bool **)create_2d_array(1, 1, sizeof(bool));
    cells[0][0] = false;
    Cave *cave = create_test_cave(1, 1, cells);
    
    /* birth_limit=7: 8 > 7, so cell should become alive */
    ck_assert_int_eq(simulate_cave(cave, 7, 3), true);
    ck_assert_int_eq(cave->cells[0][0], true);
    
    destroy_cave(cave);
    destroy_2d_array((void **)cells, 1);
}
END_TEST

START_TEST(test_simulate_cave_death_rule) {
    /* Test: alive cell dies when live_neighbors < death_limit */
    
    /* 1x1 cave: alive cell has 8 out-of-bounds neighbors = 8 live */
    bool **cells = (bool **)create_2d_array(1, 1, sizeof(bool));
    cells[0][0] = true;
    Cave *cave = create_test_cave(1, 1, cells);
    
    /* death_limit=3: 8 >= 3, cell stays alive */
    ck_assert_int_eq(simulate_cave(cave, 4, 3), false);  /* no change */
    ck_assert_int_eq(cave->cells[0][0], true);
    
    destroy_cave(cave);
    destroy_2d_array((void **)cells, 1);
}
END_TEST

START_TEST(test_simulate_cave_no_change) {
    /* Test case where no cells change - all alive with low death_limit */
    bool **cells = (bool **)create_2d_array(3, 3, sizeof(bool));
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            cells[i][j] = true;
        }
    }
    
    Cave *cave = create_test_cave(3, 3, cells);
    
    /* All cells alive, all have many live neighbors (including out-of-bounds) */
    /* With death_limit=0, all should stay alive */
    ck_assert_int_eq(simulate_cave(cave, 0, 0), false);  /* no changes */
    
    /* Verify all still alive */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            ck_assert_int_eq(cave->cells[i][j], true);
        }
    }
    
    destroy_cave(cave);
    destroy_2d_array((void **)cells, 3);
}
END_TEST

/* ==================== initialize_random_cave tests ==================== */

START_TEST(test_initialize_random_cave) {
    Cave *cave = create_cave(10, 10);
    ck_assert_ptr_nonnull(cave);
    
    /* Test with invalid chance value (negative) - should not modify */
    /* First set all cells to known state */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            cave->cells[i][j] = false;
        }
    }
    initialize_random_cave(cave, -1);
    /* With invalid param, function returns early, cells should remain false */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            ck_assert_int_eq(cave->cells[i][j], false);
        }
    }
    
    /* Test with invalid chance value (>100) - should not modify */
    initialize_random_cave(cave, 101);
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            ck_assert_int_eq(cave->cells[i][j], false);
        }
    }
    
    /* Test with chance = 0 (all cells should be dead) */
    /* random_int(1,100) <= 0 is always false */
    initialize_random_cave(cave, 0);
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            ck_assert_int_eq(cave->cells[i][j], false);
        }
    }
    
    /* Test with chance = 100 (all cells should initialize alive) */
    initialize_random_cave(cave, 100);

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            ck_assert_int_eq(cave->cells[i][j], true);
        }
    }
			
    /* Test NULL cave - should not crash */
    initialize_random_cave(NULL, 50);
    
    destroy_cave(cave);
}
END_TEST

/* ==================== Suite creation ==================== */

Suite *cave_suite(void) {
    Suite *s = suite_create("Cave");
    TCase *tc_core = tcase_create("Core");
    
    tcase_add_test(tc_core, test_create_destroy_cave);
    tcase_add_test(tc_core, test_get_cave_scheme);
    tcase_add_test(tc_core, test_load_cave_from_file);
    tcase_add_test(tc_core, test_simulate_cave_birth_rule);
    tcase_add_test(tc_core, test_simulate_cave_death_rule);
    tcase_add_test(tc_core, test_simulate_cave_no_change);
    tcase_add_test(tc_core, test_initialize_random_cave);
    
    suite_add_tcase(s, tc_core);
    return s;
}

/* Export test function for external runner - NO main() here */
int test_cave(void) {
    int number_failed;
    Suite *s = cave_suite();
    SRunner *sr = srunner_create(s);
    
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
