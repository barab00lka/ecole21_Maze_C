#ifndef CAVE_H
#define CAVE_H

#include <stdbool.h>

#define CAVE_SIZEMAX 50

// Structure to represent a cave
typedef struct
{
  int rows, cols;
  bool **cells; // true if cell is alive (wall), false if dead (empty)
} Cave;

// Cave creation and destruction
Cave *create_cave (int rows, int cols);
void destroy_cave (Cave *cave);

/* returns a dynamically allocated string with text representation of a Cave,
 * the buffer has to be freed by the caller manually */
char *get_cave_scheme (const Cave *cave);

/* universal cave loading from char string */
Cave *load_cave (const char *content);

// Cave loading from file wrapper
Cave *load_cave_from_file (const char *filename);

// Cave saving to file
bool save_cave_to_file (const Cave *cave, const char *filename);

// Compare caves. returns 0 if equal
bool is_alive_cave (Cave *old, Cave *cur);
// Cave simulation step
bool simulate_cave (Cave *cave, int birth_limit, int death_limit);

// Cave rendering
void render_cave (const Cave *cave);

// Renders cave simulation for <total_ticks> cycles with <delay> milliseconds
// delay
bool render_lifecycles (Cave *cave, int birth_limit, int death_limit,
                        int delay, int total_ticks);

/* Plays cave lifecycle step-by-step with user input. Returns on 2 conditions:
 * 1) if cave is no longer alive 2) if user pressed 'q'. Return value is total
 * number of ticks. Blocks stdin. */
int render_state_interactive (Cave *cave, int birth_limit, int death_limit);

// Random cave initialization
void initialize_random_cave (Cave *cave, int chance);

#endif
