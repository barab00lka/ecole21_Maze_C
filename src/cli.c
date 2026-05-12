#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cave.h"
#include "maze.h"
#include "rl_solver.h"
#include "solver.h"

enum
{
  LOAD_MAZE = 1,
  GENERATE_MAZE,
  SOLVE_MAZE,
  SOLVE_MAZE_ML,
  SAVE_MAZE,
  LOAD_CAVE,
  GENERATE_CAVE,
  SIMULATE_CAVE,
  SAVE_CAVE,
  EXIT
};

typedef struct
{
  bool exists_maze, exists_cave, maze_from_file, cave_from_file, maze_saved,
      cave_saved;
  char filename[FILENAME_MAX];
  char cave_fname[FILENAME_MAX];
  Maze *maze; // main Maze instance
  Cave *cave; // main Cave instance
  Path *path; // shortest Maze path
  RLAgent *rl_agent;
} State;

static void
color_print (const char *color, const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  printf ("%s", color);
  vprintf (fmt, args);
  printf ("\033[0m");
  va_end (args);
}

#define C_RESET "\033[0m"
#define C_TITLE "\033[1;35m"
#define C_MENU "\033[1;33m"
#define C_STATUS "\033[0;32m"
#define C_ERROR "\033[1;31m"
#define C_PROMPT "\033[1;37m"

void
print_start_menu (State *s)
{
  color_print (C_TITLE, "\n=== Maze and Cave Solver ===\n");
  color_print (C_MENU, "%d. Load maze from file ", LOAD_MAZE);
  if (s->exists_maze & s->maze_from_file)
    color_print ("\033[0;96m", "Loaded %s. rows: %d / cols: %d\n", s->filename,
                 s->maze->rows, s->maze->cols);
  else
    printf ("\n");

  color_print (C_MENU, "%d. Generate a maze ", GENERATE_MAZE);
  if (s->exists_maze && !s->maze_from_file)
    color_print (C_STATUS, "Generated. rows: %d / cols: %d\n", s->maze->rows,
                 s->maze->cols);
  else
    printf ("\n");

  color_print (C_MENU, "%d. Solve a maze\n", SOLVE_MAZE);

  color_print (C_MENU, "%d. Solve a maze using reinforcement learning\n",
               SOLVE_MAZE_ML);

  color_print (C_MENU, "%d. Save maze to file ", SAVE_MAZE);
  if (s->maze_saved == 1)
    color_print (C_STATUS, "Current maze is saved to disk\n");
  else
    printf ("\n");

  color_print (C_MENU, "%d. Load cave from file ", LOAD_CAVE);
  if (s->exists_cave & s->cave_from_file)
    color_print ("\033[0;96m", "Loaded %s. rows: %d / cols: %d\n",
                 s->cave_fname, s->cave->rows, s->cave->cols);
  else
    printf ("\n");

  color_print (C_MENU, "%d. Generate a cave ", GENERATE_CAVE);
  if (s->exists_cave && !s->cave_from_file)
    color_print (C_STATUS, "Generated. rows: %d / cols: %d\n", s->cave->rows,
                 s->cave->cols);
  else
    printf ("\n");

  color_print (C_MENU, "%d. Cave simulation\n", SIMULATE_CAVE);

  color_print (C_MENU, "%d. Save cave to file ", SAVE_CAVE);
  if (s->cave_saved)
    color_print (C_STATUS, "Current cave is saved to disk\n");
  else
    printf ("\n");

  color_print (C_MENU, "%d. Exit\n", EXIT);
  color_print (C_PROMPT, "Choose an option (%d-%d): ", LOAD_MAZE, EXIT);
}

static void
clear_screen ()
{
  printf ("\033[2J\033[H"); // clear screen, move cursor to top-left
  fflush (stdout);
}

void
run_cli ()
{
  int choice = 0;
  State *s = &(State){ 0 }; // initial program stat
  char err[512] = { 0 };                  // error string buffer

  while (choice != EXIT)
    {
      clear_screen ();
      if (*err)
        {
          color_print (C_ERROR, "%s", err);
          *err = 0; // flush error buffer
        }
      print_start_menu (s);

      if (scanf ("%d", &choice) != 1)
        {
          // Clear input buffer
          while (getchar () != '\n')
            ;
          snprintf (err, sizeof err, "%s\n",
                    "Error: Invalid input. Please enter a number.");
          continue;
        }

      // Clear input buffer
      while (getchar () != '\n')
        ;
      clear_screen ();

      switch (choice)
        {
        case LOAD_MAZE:
          {
            char ch;
            color_print (C_PROMPT,
                         "Enter path to file e.g: \"my_maze.txt\"...\n");

            if (scanf ("%4095s%c", s->filename, &ch) != 2)
              { // 4095 == FILENAME_MAX - 1
                snprintf (err, sizeof err, "%s\n", "Error: Invalid input");
                break;
              }
            color_print (C_STATUS, "Loading maze from file...\n");
            Maze *new = NULL;
            if ((new = load_maze_from_file (s->filename)) == NULL)
              {
                size_t max_name
                    = sizeof err
                      - strlen ("Error: Failed to load maze from file \n") - 1;
                snprintf (err, sizeof err,
                          "Error: Failed to load maze from file %.*s\n",
                          (int)max_name, s->filename);
                s->maze_from_file = s->exists_maze = 0;
                (*s->filename) = '\0';
                break;
              }
            if (s->maze)
              destroy_maze (s->maze);
            s->maze = new; // save loaded maze for next iterations
            render_maze (s->maze);
            s->maze_saved = 0;
            s->maze_from_file = s->exists_maze = 1;

            color_print (C_PROMPT,
                         "Press 'Enter' to get solution, or 'q' to go back to "
                         "main menu...\n");
            char inp = '\n';
            if ((inp = getchar ()) == 'q')
              {
                // Clear input buffer
                while (getchar () != '\n')
                  ;
                break;
              }
            else // fallthrough straight to SOLVE_MAZE if user entered newline
              {
                choice = SOLVE_MAZE;
              }
          }
          if (s->rl_agent)
            {
              rl_agent_free (s->rl_agent); //
              s->rl_agent = NULL;
            }
          __attribute__ ((fallthrough));
        case GENERATE_MAZE:
          if (choice == GENERATE_MAZE)
            {
              char ch;
              color_print (C_PROMPT, "Enter rows / cols (%d-%d): \n", 1,
                           MAZE_SIZEMAX);
              int rows = 0, cols = 0;
              if (scanf ("%d %d%c", &rows, &cols, &ch) != 3)
                {
                  snprintf (err, sizeof err, "%s\n", "Error: Invalid input");
                  break;
                }
              Maze *new = NULL;
              if ((new = generate_perfect_maze (rows, cols)) == NULL)
                {
                  snprintf (err, sizeof err, "%s\n",
                            "Error: incorrect rows / cols");
                  s->exists_maze = 0;
                  break;
                }
              color_print (C_STATUS, "Generated a perfect maze\n");
              if (s->maze)
                destroy_maze (s->maze);
              s->maze = new; // save generated maze for next iterations
              render_maze (s->maze);
              (*s->filename) = s->maze_from_file = s->maze_saved = 0;
              s->exists_maze = 1;
              color_print (
                  C_PROMPT,
                  "Enter any key to get solution, or 'q' to go back to "
                  "main menu...\n");
              char inp = '\n';
              if ((inp = getchar ()) == 'q')
                {
                  // Clear input buffer
                  while (getchar () != '\n')
                    ;
                  break;
                }
              // fallthrough straight to SOLVE_MAZE if user didn't enter 'q'
              choice = SOLVE_MAZE;
            }
          if (s->rl_agent)
            {
              rl_agent_free (s->rl_agent);
              s->rl_agent = NULL;
            }
          __attribute__ ((fallthrough));
        case SOLVE_MAZE:
          if (s->exists_maze == 0)
            {
              snprintf (err, sizeof err, "%s\n",
                        "No maze to solve. Use geneator or load from file!");
              break;
            }
          Position start, end = { 0 };
          char ch;
          color_print (C_PROMPT,
                       "Enter start position (x,y) and end position (x,y): ");
          if (scanf ("%d %d %d %d%c", &start.row, &start.col, &end.row,
                     &end.col, &ch)
              != 5)
            {
              snprintf (err, sizeof err, "%s\n", "Error: Invalid input");
              break;
            }

          Path *new_path = solve_maze (s->maze, start, end);
          if (new_path == NULL)
            {
              snprintf (err, sizeof err, "%s\n", "Error: No path was found");
              break;
            }

          if (s->path)
            destroy_path (s->path);
          s->path = new_path;
          render_path (s->maze, s->path);

          color_print (C_PROMPT, "\nPress Enter to return to menu...");
          getchar (); // waits for Enter
          break;
        case SAVE_MAZE:
          if (s->exists_maze == 0)
            {
              snprintf (err, sizeof err, "%s\n",
                        "No maze to save. Use geneator or load from file!");
              break;
            }
          if (!s->maze_saved)
            {
              color_print (C_PROMPT,
                           "Enter path to file e.g: \"maze_save.txt\"...\n");

              char fn[FILENAME_MAX];
              if (scanf ("%4095s%c", fn, &ch) != 2)
                { // 4095 == FILENAME_MAX - 1
                  snprintf (err, sizeof err, "%s\n", "Error: Invalid input");
                  break;
                }

              if (!save_maze_to_file (s->maze, fn))
                snprintf (err, sizeof err, "%s\n",
                          "Error: Failed to save maze");
              else
                s->maze_saved = 1;
            }

          break;
        case LOAD_CAVE:
          {
            char ch;
            color_print (C_PROMPT,
                         "Enter path to file e.g: \"my_cave.txt\"...\n");

            if (scanf ("%4095s%c", s->cave_fname, &ch) != 2)
              { // 4095 == FILENAME_MAX - 1
                snprintf (err, sizeof err, "%s\n", "Error: Invalid input");
                break;
              }
            Cave *new = NULL;
            color_print (C_STATUS, "Loading seed from file...\n");
            if ((new = load_cave_from_file (s->cave_fname)) == NULL)
              {
                size_t max_name
                    = sizeof err
                      - strlen ("Error: Failed to load cave seed from file \n")
                      - 1;
                snprintf (err, sizeof err,
                          "Error: Failed to load cave seed from file %.*s\n",
                          (int)max_name, s->cave_fname);
                s->cave_from_file = s->exists_cave = s->cave_saved = 0;
                (*s->filename) = '\0';
                break;
              }

            if (s->cave)
              {
                destroy_cave (s->cave);
              }
            s->cave = new; // save loaded cave for next iterations
            render_cave (s->cave);
            s->cave_saved = 0;
            s->cave_from_file = s->exists_cave = 1;

            color_print (
                C_PROMPT,
                "Enter any key to enter cave simulation, or 'q' to return to "
                "main menu...\n");
            char inp = '\n';
            if ((inp = getchar ()) == 'q')
              {
                // Clear input buffer
                while (getchar () != '\n')
                  ;
                break;
              }
            else
              {
                choice = SIMULATE_CAVE;
              }
          }
          __attribute__ ((fallthrough));
        case GENERATE_CAVE:
          if (choice == GENERATE_CAVE)
            {
              char ch;
              int cols = -1, rows = -1;
              printf ("Enter cave size: ");
              if (scanf ("%d %d%c", &rows, &cols, &ch) != 3 || rows < 1
                  || cols < 1 || rows > 50 || cols > 50)
                {
                  snprintf (err, sizeof err, "%s\n", "Error: Invalid size");
                  break;
                }
              Cave *new_cave = create_cave (rows, cols);
              printf ("Enter chance for the 'birth' of the cell (0-100%%): ");
              if (scanf ("%d%c", &rows, &ch) != 2 || rows < 0 || rows > 100)
                {
                  snprintf (err, sizeof err, "%s\n",
                            "Error: Invalid percentage");
                  break;
                }
              initialize_random_cave (new_cave, rows);
              if (s->cave)
                {
                  destroy_cave (s->cave);
                }
              s->cave = new_cave; // save loaded cave for next iterations
              render_cave (s->cave);
              (*s->cave_fname) = s->cave_from_file = s->cave_saved = 0;
              s->exists_cave = 1;
              color_print (C_PROMPT, "Enter any key to enter cave simulation, "
                                     "or 'q' to return to "
                                     "main menu...\n");
              char inp = '\n';
              if ((inp = getchar ()) == 'q')
                {
                  // Clear input buffer
                  while (getchar () != '\n')
                    ;
                  break;
                }
              else
                {
                  choice = SIMULATE_CAVE;
                }
            }
          __attribute__ ((fallthrough));
        case SIMULATE_CAVE:

          if (s->exists_cave == 0)
            {
              snprintf (err, sizeof err, "%s\n",
                        "No cave seed. Use geneator or load from file!");
              break;
            }

          int bl, dl;
          printf ("Enter 'birth limit' (0-7) and 'death limit' (0-7): ");
          if (scanf ("%d %d%c", &bl, &dl, &ch) != 3 || bl < 0 || dl > 7
              || dl < 0 || bl > 7)
            {
              snprintf (err, sizeof err, "%s\n", "Error: Invalid limits");
              printf ("%s", err);
              break;
            }

          printf ("Press 'i' for interactive mode or 'a' for automatic: ");
          scanf ("%c", &ch);
          if (ch == 'a')
            {
              int i = 0, delay = 1000;
              printf ("Enter count of iterations: ");
              scanf ("%d", &i);
              printf ("Enter delay in milliseconds: ");
              scanf ("%d", &delay);
              render_lifecycles (s->cave, bl, dl, delay, i);
            }
          else if (ch == 'i')
            {
              render_state_interactive (s->cave, bl, dl);
            }

          getchar (); // waits for Enter
          break;
        case SAVE_CAVE:
          if (s->exists_cave == 0)
            {
              snprintf (err, sizeof err, "%s\n",
                        "No cave to save. Use geneator or load from file!");
              break;
            }
          if (!s->cave_saved)
            {
              color_print (C_PROMPT,
                           "Enter path to file e.g: \"cave_save.txt\"...\n");

              char fn[FILENAME_MAX];
              if (scanf ("%4095s%c", fn, &ch) != 2)
                { // 4095 == FILENAME_MAX - 1
                  snprintf (err, sizeof err, "%s\n", "Error: Invalid input");
                  break;
                }

              if (!save_cave_to_file (s->cave, fn))
                snprintf (err, sizeof err, "%s\n",
                          "Error: Failed to save cave");
              else
                s->cave_saved = 1;
            }

          break;
        case SOLVE_MAZE_ML:
          if (s->exists_maze == 0)
            {
              snprintf (
                  err, sizeof err,
                  "No maze to solve. Use generator or load from file!\n");
              break;
            }

          if (s->rl_agent == NULL)
            {
              Position goal;
              color_print (
                  C_PROMPT,
                  "Agent is not trained. Enter GOAL position (row col): ");
              if (scanf ("%d %d", &goal.row, &goal.col) != 2)
                {
                  snprintf (err, sizeof err, "Invalid goal coordinates\n");
                  break;
                }
              color_print (C_STATUS,
                           "Training agent (this may take a moment)... \n");
              s->rl_agent = rl_train (s->maze, goal); //
              if (!s->rl_agent)
                {
                  snprintf (err, sizeof err, "Training failed!\n");
                  break;
                }
              color_print (C_STATUS, "Training complete!\n");
            }

          Position ml_start;
          color_print (
              C_PROMPT,
              "Enter START position (row col) or -1 to reset agent: ");
          if (scanf ("%d %d", &ml_start.row, &ml_start.col) != 2)
            {
              while (getchar () != '\n')
                ; // очистка буфера
              break;
            }

          if (ml_start.row == -1)
            {
              rl_agent_free (s->rl_agent);
              s->rl_agent = NULL;
              break;
            }

          Path *ml_path = rl_get_path (s->rl_agent, s->maze, ml_start);
          if (ml_path == NULL)
            {
              snprintf (err, sizeof err, "No path found from this point.\n");
              break;
            }

          if (s->path)
            destroy_path (s->path);
          s->path = ml_path;
          render_path (s->maze, s->path);

          color_print (C_PROMPT, "\nPress Enter to return to menu...");
          getchar ();
          getchar ();
          break;
        case EXIT:
          color_print (C_STATUS, "Exiting...\n");
          break;
        default:
          color_print (
              C_ERROR,
              "Invalid option. Please choose a number between 1 and 7.\n");
          break;
        }
    }
  // Cleanup
  if (s->maze)
    {
      destroy_maze (s->maze);
    }
  if (s->cave)
    {
      destroy_cave (s->cave);
    }
  if (s->path)
    {
      destroy_path (s->path);
    }
}
