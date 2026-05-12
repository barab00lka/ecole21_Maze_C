#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cave.h"
#include "cli.h"
#include "maze.h"
#include "rl_solver.h"
#include "solver.h"

// debug
#include "debug.h"
#undef DEBUG
#define DEBUG 0

#define MAX_FILENAME 256
#define APP_NAME "A1_MAZE_C"

void
print_app_name ()
{
  printf ("%s\n", APP_NAME);
}

void
print_help ()
{
  print_app_name ();
  printf ("Usage: maze [options]\n\n");
  printf ("Options:\n");
  printf ("  -g ROWS COLS FILENAME      Generate a perfect maze of given "
          "dimensions and save to file\n");
  printf ("  -l FILENAME           Load a maze from file\n");
  printf (
      "  -s FILENAME R1 C1 R2 C2 Solve the maze from (R1,C1) to (R2,C2)\n");
  printf ("  -c FILENAME           Load a cave from file\n");
  printf (
      "  -simulate B D     Simulate cave with birth limit B and death limit "
      "D\n");
  printf ("  -step             Step-by-step mode for cave simulation\n");
  printf ("  -auto N           Automatic mode for cave simulation with N ms "
          "delay\n");
  printf ("  -h, --help        Show this help message\n\n");
  printf ("Examples:\n");
  printf ("  maze -l <filename>         # Load maze from file\n");
  printf ("  maze -g <rows> <cols> <filename> # Generate and save maze\n");
  printf ("  maze -s <filename> 0 0 9 9 # Solve maze\n");
  printf ("  maze -cave cave.txt -simulate 4 4 -step # Interactive cave "
          "simulation\n");
  printf ("  maze -r FILENAME R1 C1 R2 C2 Solve the maze using RL from "
          "(R1,C1) to (R2,C2)\n");
}

int
handle_generate_maze (int argc, char *argv[])
{
  // maze -g 10 10 filename.txt
  const int params_num = 3;
  if (argc < 2 + params_num)
    {
      fprintf (stderr, "Error: Missing params, must be \"maze -g rows columns "
                       "filename_name\"\n");
      return 1;
    }

  int rows = atoi (argv[2]);
  int cols = atoi (argv[3]);
  char *filename = argv[4];

  fprintf (stderr, "optind: %d, rows: %d, cols: %d, filename: %s\n", optind,
           rows, cols, filename);
  Maze *maze = generate_perfect_maze (rows, cols);
  if (!maze)
    {
      fprintf (stderr, "Error: Failed to generate maze\n");
      return 1;
    }

  printf ("Generated %dx%d perfect maze\n", rows, cols);
  render_maze (maze);
  save_maze_to_file (maze, filename);
  destroy_maze (maze);
  printf ("Maze saved to file: %s\n", filename);
  return 0;
}

int
handle_load_maze (int argc, char *argv[])
{
  // load maze, maze -l filename.txt
  const int params_num = 1;
  if (argc < 2 + params_num)
    {
      fprintf (stderr, "Error: Missing filename for maze load\n");
      return 1;
    }

  const char *filename = argv[2];
  Maze *maze = load_maze_from_file (filename);
  if (!maze)
    {
      fprintf (stderr, "Error: Failed to load maze from file '%s'\n",
               filename);
      return 1;
    }
  printf ("Loaded maze from '%s':\n", filename);
  render_maze (maze);
  return 0;
}

int
handle_solve_maze (int argc, char *argv[])
{
  // solve maze, maze -s filename start_x start_y end_x end_y
  const int params_num = 5;
  if (argc < 2 + params_num)
    {
      fprintf (stderr, "Error: Missing params, must be \"maze -s filename "
                       "start_x start_y end_x end_y\"\n");
      return 1;
    }

  char *filename = argv[2];

  int start_x = atoi (argv[3]);
  int start_y = atoi (argv[4]);
  int end_x = atoi (argv[5]);
  int end_y = atoi (argv[6]);

  LOG_DEBUG (fprintf (stderr,
                      "Loading maze from: %s, start: (%d,%d), end: (%d,%d)\n",
                      filename, start_x, start_y, end_x, end_y));

  Maze *maze = load_maze_from_file (filename);
  if (!maze)
    {
      fprintf (stderr, "Error: Failed to load maze from file '%s'\n",
               filename);
      return 1;
    }

  LOG_DEBUG (fprintf (stderr, "maze was loaded\n"));
  LOG_DEBUG (render_maze (maze));

  Position start = { start_x, start_y };
  Position end = { end_x, end_y };

  Path *path = solve_maze (maze, start, end);
  if (!path)
    {
      printf ("No path found from (%d,%d) to (%d,%d)\n", start_x, start_y,
              end_x, end_y);
    }
  else
    {
      printf ("Solution for %s from (%d,%d) to (%d,%d):\n", filename, start_x,
              start_y, end_x, end_y);
      render_path (maze, path);
      destroy_path (path);
    }

  return 0;
}

int
handle_rl_solve_maze (int argc, char *argv[])
{
  // Использование: maze -r filename goal_row goal_col
  if (argc < 5) {
      fprintf(stderr, "Usage: maze -r <filename> <goal_row> <goal_col>\n");
      return 1;
  }

  char *filename = argv[2];
  Position goal = { atoi(argv[3]), atoi(argv[4]) };

  Maze *maze = load_maze_from_file(filename);
  if (!maze) return 1;

  printf("Training agent for goal (%d, %d)...\n", goal.row, goal.col);
  RLAgent *agent = rl_train(maze, goal); //[cite: 1, 2]
  
  if (!agent) {
      fprintf(stderr, "Failed to train agent.\n");
      destroy_maze(maze);
      return 1;
  }

  printf("Agent ready! Enter start coordinates (row col) to find path, or 'q' to quit:\n");
  int s_row, s_col;
  while (scanf("%d %d", &s_row, &s_col) == 2) {
      Position start = { s_row, s_col };
      Path *path = rl_get_path(agent, maze, start); //[cite: 1, 2]
      
      if (path) {
          render_path(maze, path);
          destroy_path(path);
      } else {
          printf("No path found from (%d, %d)\n", s_row, s_col);
      }
      printf("\nNext start point (row col) or 'q': ");
  }

  rl_agent_free(agent);
  destroy_maze(maze);
  return 0;
}

int
handle_load_cave (int argc, char *argv[])
{
  // load cave, maze -c filename.txt
  const int params_num = 1;
  if (argc < 2 + params_num)
    {
      fprintf (stderr, "Error: Missing filename for cave load\n");
      return 1;
    }

  const char *filename = argv[optind - 1];
  Cave *cave = load_cave_from_file (filename);
  if (!cave)
    {
      fprintf (stderr, "Error: Failed to load cave from file '%s'\n",
               filename);
      return 1;
    }

  printf ("Loaded cave from '%s':\n", filename);
  render_cave (cave);
  return 0;
}

int
main (int argc, char *argv[])
{
  Maze *maze = NULL;
  Cave *cave = NULL;

  int opt;
  opt = getopt (argc, argv, "g:l:c:m:hs");
  fprintf (stderr, "optind: %d, opt: %c\n", optind, opt);
  int err = 0;
  switch (opt)
    {
    case 'g':
      err = handle_generate_maze (argc, argv);
      break;
    case 'l':
      err = handle_load_maze (argc, argv);
      break;
    case 's':
      err = handle_solve_maze (argc, argv);
      break;
    case 'r':
      err = handle_rl_solve_maze (argc, argv);
      break;
    case 'c':

      err = handle_load_cave (argc, argv);
      break;
    case 'h':
      print_help ();
      return 0;
    case '?':
      return 0;
    default:
      run_cli ();
      // fprintf(stderr, "Unknown option: %c\n", opt);
      // print_help();
      return 1;

      if (err)
        {
          return err;
        }

      // Cleanup
      if (maze)
        {
          destroy_maze (maze);
        }
      if (cave)
        {
          destroy_cave (cave);
        }

      return 0;
    }
}
