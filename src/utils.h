#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

// Utility functions
bool is_valid_position (int row, int col, int rows, int cols);

// Memory allocation with error checking
void *safe_malloc (size_t size);
void **create_2d_array (int rows, int cols, size_t element_size);
void destroy_2d_array (void **array, int rows);

// Random number generation
int random_int (int min, int max);

#endif