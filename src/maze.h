#ifndef MAZE_H
#define MAZE_H

#include <stdbool.h>

#define MAZE_SIZEMAX 50

// Structure to represent a maze
typedef struct
{
  int rows, cols;
  bool **vertical_walls;   // Vertical walls: [row][col] - wall to the right of
                           // cell (row,col)
  bool **horizontal_walls; // Horizontal walls: [row][col] - wall below cell
                           // (row,col)
} Maze;

// Maze creation and destruction
Maze *create_maze (int rows, int cols);
void destroy_maze (Maze *maze);

/* universal maze loading from char string */
Maze *load_maze (const char *content);

// Maze loading from file wrapper
Maze *load_maze_from_file (const char *filename);

/* returns a dynamically allocated string with text representation of a Maze,
 * the buffer has to be freed by the caller manually */
char *get_maze_scheme (const Maze *maze);
// Maze saving to file
bool save_maze_to_file (const Maze *maze, const char *filename);

// Maze rendering
void render_maze (const Maze *maze);

// Perfect maze generation using Eller's algorithm
Maze *generate_perfect_maze (int rows, int cols);

// Check if maze is perfect (exactly one path between any two cells)
bool is_perfect_maze (const Maze *maze);

void print_maze (const Maze *maze);

#endif
