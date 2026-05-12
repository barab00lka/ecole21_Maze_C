#include <fcgi_stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../cave.h"
#include "../debug.h"
#include "../maze.h"
#include "../rl_solver.h"
#include "../solver.h"
#include "api_handlers.h"
#include "cJSON.h"
#include "helper.h"

#define FILESIZE_MAX 12000
#define MCOLS MAZE_SIZEMAX
#define MROWS MAZE_SIZEMAX

enum
{
  IO_ERR = -1,
  OK,
  BAD_REQUEST
};

typedef struct
{
  int rows;
  int cols;
  int status;                                // 0 = ok, 1 = bad request
  Position start, end;                       // for maze only
  int birth_limit, death_limit, init_chance; // for cave only

  size_t filesize;
  char uid[37]; // uuid is 36 chars + null
  char error[96];
  char content[FILESIZE_MAX + 1];

} maze_request;

typedef struct
{
  Maze *maze; // current Maze instance
  Path *path; // shortest Maze path
  Cave *cave; // current Cave instance
  RLAgent *rl_agent;
} User;

static maze_request
parse_maze_request (const char *post_request)
{
  maze_request req = { 0 };
  cJSON *json = cJSON_Parse (post_request);
  if (!json)
    {
      const char *error_ptr = cJSON_GetErrorPtr ();
      if (error_ptr != NULL)
        {
#ifdef DEBUG
          fprintf (stderr, "Error before: %s\n", error_ptr);
#endif
        }

      return (maze_request){ .status = BAD_REQUEST,
                             .error = "Invalid request\n" };
    }

  /* parse uid for all requests */
  const cJSON *uid = cJSON_GetObjectItem (json, "uid");
  if (!cJSON_GetStringValue (uid))
    {
      LOG_DEBUG (fprintf (stderr, "Missing uid\n"));
      return (maze_request){ .status = BAD_REQUEST,
                             .error
                             = "Missing uid\n" }; // always need a valid uid
    }
  strncpy (req.uid, uid->valuestring, 36);

  /* if client tries to load file */
  const cJSON *content = cJSON_GetObjectItem (json, "content");
  const char *content_payload = cJSON_GetStringValue (content);

  if (content_payload != NULL)
    {
      req.filesize = strlen (content_payload);
      if (req.filesize <= FILESIZE_MAX)
        {
          /* copy file content into (maze_request){.content} if its not too
           * large */
          strcpy (req.content, content_payload);
        }
      else
        {
          LOG_DEBUG (fprintf (stderr, "user_Id: %s, File too large (%lu>%u)\n",
                              req.uid, req.filesize, FILESIZE_MAX));
          maze_request bad = { .status = BAD_REQUEST };
          snprintf (bad.error, sizeof (bad.error),
                    "file must not be more than %u bytes", FILESIZE_MAX);
        }
    }

  /* parse values for maze solve */
  const cJSON *x_start = cJSON_GetObjectItem (json, "startCol");
  const cJSON *y_start = cJSON_GetObjectItem (json, "startRow");
  const cJSON *x_end = cJSON_GetObjectItem (json, "endCol");
  const cJSON *y_end = cJSON_GetObjectItem (json, "endRow");
  if (!cJSON_IsNumber (x_start) || !cJSON_IsNumber (x_end)
      || !cJSON_IsNumber (y_start) || !cJSON_IsNumber (y_end))
    {
      char str[] = "invalid position";
      // req.status = BAD_REQUEST;
      strncpy (req.error, str, strlen (req.error));
    }
  else
    {
      req.start.col = x_start->valueint, req.end.col = x_end->valueint;
      req.start.row = y_start->valueint, req.end.row = y_end->valueint;
    }

  /* parse values for cave generate */
  const cJSON *init_chance = cJSON_GetObjectItem (json, "ic");
  if (!cJSON_IsNumber (init_chance))
    {
      char str[] = "invalid cave gen parameters";
      // req.status = BAD_REQUEST;
      strncpy (req.error, str, strlen (req.error));
    }
  else
    {
      req.init_chance = init_chance->valueint;
    }

  /* parse values for cave simulate */
  const cJSON *birth_limit = cJSON_GetObjectItem (json, "bl");
  const cJSON *death_limit = cJSON_GetObjectItem (json, "dl");
  if (!cJSON_IsNumber (birth_limit) || !cJSON_IsNumber (death_limit))
    {
      char str[] = "invalid simulation parameters";
      // req.status = BAD_REQUEST;
      strncpy (req.error, str, strlen (req.error));
    }
  else
    {
      req.birth_limit = birth_limit->valueint;
      req.death_limit = death_limit->valueint;
    }

  /* parse values for maze AND cave generate */
  const cJSON *rows = cJSON_GetObjectItem (json, "rows");
  const cJSON *cols = cJSON_GetObjectItem (json, "cols");
  if (!cJSON_IsNumber (cols) || !cJSON_IsNumber (rows))
    {
      char str[] = "invalid maze dimensions";
      // req.status = BAD_REQUEST;
      strncpy (req.error, str, strlen (req.error));
    }
  else
    {
      req.rows = rows->valueint;
      req.cols = cols->valueint;
    }

  cJSON_Delete (json);
  return req;
}

static void
send_json (cJSON *obj)
{
  char *str = cJSON_Print (obj);
  printf ("Content-type: application/json\r\n\r\n%s", str);
  free (str);
  cJSON_Delete (obj);
}

static void
send_json_error (int status_code, const char *msg)
{
  cJSON *obj = cJSON_CreateObject ();
  cJSON_AddNumberToObject (obj, "status", status_code);
  if (msg)
    cJSON_AddStringToObject (obj, "error", msg);
  send_json (obj);
}

/* /api/maze/generate */
void
handle_maze_generate (char *post_request, Hashtable *map)
{
  maze_request req = parse_maze_request (post_request);
  if (req.status != OK || strcmp (req.error, "invalid maze dimensions") == 0)
    {
      send_json_error (BAD_REQUEST, req.error);
      return;
    }

  if (req.rows < 1 || req.rows > MROWS || req.cols < 1 || req.cols > MCOLS)
    {
      LOG_DEBUG (fprintf (stderr, "Invalid rows / cols size.\n"));
      send_json_error (req.status = BAD_REQUEST,
                       "Invalid rows / cols size. Must be 1-50");
      return;
    }

  /* if client JSON request is valid, now we can build response */
  Maze *new = generate_perfect_maze (req.rows, req.cols);
  if (new == NULL)
    {
      send_json_error (IO_ERR, "Failed to generate maze");
      return;
    }

  List *save = ht_lookup (map, req.uid);
  if (!save)
    {
      User *new_user = calloc (1, sizeof (User));
      if (!new_user)
        {
          destroy_maze (new);
          LOG_DEBUG (fprintf (stderr, "Memory allocation failed\n"));
          send_json_error (IO_ERR, "memory alloc failure");
          return;
        }
      new_user->maze = new, new_user->path = NULL;
      List *head = ht_set (map, req.uid);
      head->data = new_user;
    }
  else
    {
    User *old = (User *)save->data;
    if (old->maze) // discard old maze if exists
      destroy_maze (old->maze);
    if (old->path)
      destroy_path (old->path);
	if (old->rl_agent) {
		rl_agent_free(old->rl_agent);
		old->rl_agent = NULL;
	}

      old->maze = new, old->path = NULL;
    }

  // Create a new JSON object
  cJSON *response_json = cJSON_CreateObject ();
  if (response_json == NULL)
    {
      LOG_DEBUG (fprintf (stderr, "Allocation Error\n"));
      send_json_error (IO_ERR, "out of mem");
      return;
    }
  // initialize zero error status
  cJSON_AddNumberToObject (response_json, "status", OK);
  cJSON_AddNumberToObject (response_json, "rows", req.rows),
      cJSON_AddNumberToObject (response_json, "cols", req.cols);
  maze_to_json (new, response_json);

  send_json (response_json);
}

void
handle_maze_solve (char *post_request, Hashtable *map)
{
  /* /api/maze/solve */
  maze_request req = parse_maze_request (post_request);
  if (req.status != OK || strcmp (req.error, "invalid position") == 0)
    {
      send_json_error (BAD_REQUEST, req.error);
      LOG_DEBUG (fprintf (stderr, "invalid position\n"));
      return;
    }

  List *save = ht_lookup (map, req.uid);
  if (!save || !save->data)
    {
      send_json_error (BAD_REQUEST,
                       "No user save. Use maze geneator or load from file");
      LOG_DEBUG (fprintf (
          stderr, "No user save. Use maze geneator or load from file\n"));
      return;
    }

  User *old = (User *)save->data;
  if (!old->maze)
    { // если еще нет лабиринта, в котором искать путь (не
      // получен от
      // пользователя или не сгенерирован)
      send_json_error (BAD_REQUEST,
                       "No maze to solve. Generate or load from file");
      LOG_DEBUG (
          fprintf (stderr, "No maze to solve. Generate or load from file\n"));
      return;
    }

  /* здесь можно сделать дополнительную проверку флага is_solved, если
   * пользователь отправляет запрос на кратчайший путь такой же путь одного и
   * того же лабиринта не первый раз, не пересчитывать заново путь, а отправить
   * сохраненный Path*... но мне лень это писать, да и для учебного проекта это
   * не так критично. */
  Path *sol = solve_maze (old->maze, req.start, req.end);
  if (!sol)
    {
      send_json_error (BAD_REQUEST, "couldn't find path");
      LOG_DEBUG (fprintf (stderr, "couldnt find path!\n"));
      return;
    }

  cJSON *response_json = cJSON_CreateObject ();
  if (response_json == NULL)
    {
      LOG_DEBUG (fprintf (stderr, "Allocation Error\n"));
      send_json_error (IO_ERR, "out of mem");
      return;
    }
  // initialize zero error status
  cJSON_AddNumberToObject (response_json, "status", OK);
  cJSON_AddStringToObject (response_json, "error", "Found shortest path");

  maze_to_json (old->maze, response_json);
  path_to_json (sol, response_json);

  send_json (response_json);
}

void
send_content_binary (const char *content)
{
  printf ("Content-Type: application/octet-stream\r\n");
  printf ("Content-Disposition: attachment; filename=\"%s\"\r\n\r\n",
          "maze.txt");

  fputs (content, stdout); // stdout here is FCGI_stdout via macro
}

void
send_file_binary (const char *filepath)
{
  printf ("Content-Type: application/octet-stream\r\n");
  printf (
      "Content-Disposition: attachment; filename=\"%s\"\r\n\r\n",
      filepath
          + strlen ("/tmp")); // filename is filepath without leading "/tmp"
  int fd = open (filepath, O_RDONLY);
  char buf[4096];
  ssize_t n;
  while ((n = read (fd, buf, sizeof (buf))) > 0)
    fwrite (buf, 1, n, stdout); // stdout here is FCGI_stdout via macro
  close (fd);
}

void
handle_maze_save (char *post_request, Hashtable *map)
{
  maze_request req = parse_maze_request (post_request);
  if (req.status != OK)
    {
      send_json_error (req.status, req.error);
      return;
    }
  List *save = ht_lookup (map, req.uid);
  if (!save || !save->data)
    {
      send_json_error (BAD_REQUEST,
                       "No user save. Use maze geneator or load from file");
      return;
    }

  User *old = (User *)save->data;
  if (!old->maze)
    {
      send_json_error (BAD_REQUEST,
                       "No maze to save. Generate or load from file");
      return;
    }

  send_content_binary (get_maze_scheme (old->maze));
}

void
handle_maze_load (char *post_request, Hashtable *map)
{
  maze_request req = { 0 };
  req = parse_maze_request (post_request);
  if (strlen (req.content) == 0 || strlen (req.uid) == 0)
    { // empty content
      send_json_error (req.status, req.error);
      return;
    }

  Maze *new = load_maze (req.content);
  if (new == NULL)
    {
      send_json_error (BAD_REQUEST, "error loading maze");
      return;
    }

  List *save = ht_lookup (map, req.uid);
  if (!save)
    {
      User *new_user = calloc (1, sizeof (User));
      if (!new_user)
        {
          destroy_maze (new);
          LOG_DEBUG (fprintf (stderr, "Memory allocation failed\n"));
          send_json_error (IO_ERR, "memory alloc failure");
          return;
        }
      new_user->maze = new, new_user->path = NULL;
      List *head = ht_set (map, req.uid);
      head->data = new_user;
    }
  else
    {
      User *old = (User *)save->data;
    if (old->maze) // discard old maze if exists
      destroy_maze (old->maze);
    if (old->path)
      destroy_path (old->path);
	if (old->rl_agent) {
		rl_agent_free(old->rl_agent);
		old->rl_agent = NULL;
	}
      old->maze = new, old->path = NULL;
    }

  cJSON *response_json = cJSON_CreateObject ();
  if (response_json == NULL)
    {
      LOG_DEBUG (fprintf (stderr, "Allocation Error\n"));
      send_json_error (IO_ERR, "out of mem");
      return;
    }
  // initialize zero error status
  cJSON_AddNumberToObject (response_json, "status", OK);

  maze_to_json (new, response_json);
  cJSON_AddStringToObject (response_json, "error", "maze loaded");
  send_json (response_json);
}

void
handle_cave_generate (char *post_request, Hashtable *map)
{ /* /api/cave/generate */
  maze_request req = parse_maze_request (post_request);
  if (req.status != OK || strcmp (req.error, "invalid maze dimensions") == 0)
    {
      send_json_error (BAD_REQUEST, "invalid cave dimensions");
      return;
    }

  else if (req.rows < 1 || req.rows > MROWS || req.cols < 1
           || req.cols > MCOLS)
    {
      LOG_DEBUG (fprintf (stderr, "Invalid rows / cols size.\n"));
      send_json_error (req.status = BAD_REQUEST,
                       "Invalid rows / cols size. Must be 1-50");
      return;
    }
  else if (req.init_chance < 0 || req.init_chance > 100)
    {
      LOG_DEBUG (fprintf (
          stderr, "Invalid starting initialization chance for sell.\n"));
      send_json_error (req.status = BAD_REQUEST,
                       "Invalid chance. Must be 0-100%");
      return;
    }
  /* if client JSON request is valid, now we can build response */
  Cave *new = create_cave (req.rows, req.cols);
  initialize_random_cave (new, req.init_chance);
  if (new == NULL)
    {
      send_json_error (IO_ERR, "Failed to generate cave");
      return;
    }

  List *save = ht_lookup (map, req.uid);
  if (!save)
    {
      User *new_user = calloc (1, sizeof (User));
      if (!new_user)
        {
          destroy_cave (new);
          LOG_DEBUG (fprintf (stderr, "Memory allocation failed\n"));
          send_json_error (IO_ERR, "memory alloc failure");
          return;
        }

      new_user->cave = new;
      List *head = ht_set (map, req.uid);
      head->data = new_user;
    }
  else
    {
      User *old = (User *)save->data;
      if (old->cave) // discard old cave if exists
        destroy_cave (old->cave);
      old->cave = new;
    }

  // Create a new JSON object
  cJSON *response_json = cJSON_CreateObject ();
  if (response_json == NULL)
    {
      LOG_DEBUG (fprintf (stderr, "Allocation Error\n"));
      send_json_error (IO_ERR, "out of mem");
      return;
    }
  // initialize zero error status
  cJSON_AddNumberToObject (response_json, "status", OK);
  cave_to_json (new, response_json);

  send_json (response_json);
}
void
handle_cave_simulate (char *post_request, Hashtable *map)
{ /* /api/cave/simulate */
  maze_request req = parse_maze_request (post_request);
  if (req.status != OK
      || strcmp (req.error, "invalid simulation parameters") == 0)
    {
      send_json_error (BAD_REQUEST, req.error);

      fprintf (stderr, req.error);
      return;
    }
  else if (req.birth_limit < 0 || req.death_limit < 0 || req.death_limit > 7
           || req.birth_limit > 7)
    {
      send_json_error (BAD_REQUEST, "birth/death limits must be 0-7");

      fprintf (stderr, "birth/death limits must be 0-7");
      return;
    }
  /* if client JSON request is valid, now we can build response */

  List *save = ht_lookup (map, req.uid);
  if (!save || !save->data)
    {
      send_json_error (BAD_REQUEST,
                       "No user save. Use cave generator or load from file");
      LOG_DEBUG (fprintf (stderr, "No user save\n"));
      return;
    }

  User *old = (User *)save->data;
  if (!old->cave)
    {
      send_json_error (BAD_REQUEST,
                       "No cave found. Generate or load from file");
      LOG_DEBUG (
          fprintf (stderr, "No cave found. Generate or load from file\n"));
      return;
    }

  cJSON *response_json = cJSON_CreateObject ();
  if (response_json == NULL)
    {
      LOG_DEBUG (fprintf (stderr, "Allocation Error\n"));
      send_json_error (IO_ERR, "out of mem");
      return;
    }

  // initialize zero error status
  cJSON_AddNumberToObject (response_json, "status", OK);
  cJSON_AddNumberToObject (
      response_json, "alive",
      simulate_cave (old->cave, req.birth_limit, req.death_limit));

  cave_to_json (old->cave, response_json);
  send_json (response_json);
}

void
handle_cave_save (char *post_request, Hashtable *map)
{ /* /api/cave/save */

  maze_request req = parse_maze_request (post_request);
  if (req.status != OK)
    {
      send_json_error (req.status, "unable to load maze");
      return;
    }
  List *save = ht_lookup (map, req.uid);
  if (!save || !save->data)
    {
      send_json_error (BAD_REQUEST,
                       "No user save. Use cave geneator or load from file");
      return;
    }

  User *old = (User *)save->data;
  if (!old->cave)
    {
      send_json_error (BAD_REQUEST,
                       "No cave to save. Generate or load from file");
      return;
    }

  send_content_binary (get_cave_scheme (old->cave));
}

void
handle_cave_load (char *post_request, Hashtable *map)
{ /* /api/cave/load */
  maze_request req = { 0 };
  req = parse_maze_request (post_request);
  if (strlen (req.content) == 0 || strlen (req.uid) == 0)
    { // empty content
      send_json_error (req.status, "unable to load cave");
      return;
    }

  Cave *new = load_cave (req.content);
  if (new == NULL)
    {
      send_json_error (BAD_REQUEST, "error loading maze");
      return;
    }

  List *save = ht_lookup (map, req.uid);
  if (!save)
    {
      User *new_user = calloc (1, sizeof (User));
      if (!new_user)
        {
          destroy_cave (new);
          LOG_DEBUG (fprintf (stderr, "Memory allocation failed\n"));
          send_json_error (IO_ERR, "memory alloc failure");
          return;
        }
      new_user->cave = new;
      List *head = ht_set (map, req.uid);
      head->data = new_user;
    }
  else
    {
      User *old = (User *)save->data;
      if (old->cave) // discard old maze if exists
        destroy_cave (old->cave);
      old->cave = new;
    }

  cJSON *response_json = cJSON_CreateObject ();
  if (response_json == NULL)
    {
      LOG_DEBUG (fprintf (stderr, "Allocation Error\n"));
      send_json_error (IO_ERR, "out of mem");
      return;
    }
  // initialize zero error status
  cJSON_AddNumberToObject (response_json, "status", OK);

  cave_to_json (new, response_json);
  cJSON_AddStringToObject (response_json, "error", "cave loaded");
  send_json (response_json);
}

void
handle_qlearning_train (char *post_request, Hashtable *map)
{ /* /api/qlearning/train */
  maze_request req = parse_maze_request (post_request);
  if (req.status != OK || strcmp (req.error, "invalid position") == 0)
    {
      send_json_error (BAD_REQUEST, req.error);
      return;
    }

  List *save = ht_lookup (map, req.uid);
  if (!save || !save->data)
    {
      send_json_error (BAD_REQUEST, "No user save.");
      return;
    }

  User *old = (User *)save->data;
  if (!old->maze)
    {
      send_json_error (BAD_REQUEST, "No maze to train on.");
      return;
    }

  // 1. Если агент уже был, удаляем старого перед переобучением
  if (old->rl_agent) {
      rl_agent_free(old->rl_agent);
  }

  // 2. Обучаем нового агента для указанной цели (req.end)[cite: 2]
  old->rl_agent = rl_train(old->maze, req.end);
  if (!old->rl_agent)
    {
      send_json_error (IO_ERR, "Training failed");
      return;
    }

  // 3. Сразу получаем путь от стартовой точки для ответа[cite: 2]
  Path *sol = rl_get_path(old->rl_agent, old->maze, req.start);
  
  if (old->path) destroy_path (old->path);
  old->path = sol;

  cJSON *response_json = cJSON_CreateObject ();
  cJSON_AddNumberToObject (response_json, "status", OK);
  cJSON_AddStringToObject (response_json, "error", "Q-Learning training completed");
  maze_to_json (old->maze, response_json);
  if (sol) path_to_json (sol, response_json);

  send_json (response_json);
}

void
handle_qlearning_path (char *post_request, Hashtable *map)
{ /* /api/qlearning/path */
  maze_request req = parse_maze_request (post_request);
  if (req.status != OK)
    {
      send_json_error (BAD_REQUEST, req.error);
      return;
    }

  List *save = ht_lookup (map, req.uid);
  if (!save || !save->data)
    {
      send_json_error (BAD_REQUEST, "No user save. Train agent first.");
      return;
    }
  User *old = (User *)save->data;

  if (!old->rl_agent)
    {
      send_json_error (BAD_REQUEST, "Agent not trained.");
      return;
    }

  // Используем координаты из req.start, которые заполнил parse_maze_request
  Path *new_path = rl_get_path (old->rl_agent, old->maze, req.start);

  if (!new_path)
    {
      send_json_error (404, "Could not find a path.");
      return;
    }

  cJSON *response_json = cJSON_CreateObject ();
  cJSON_AddNumberToObject (response_json, "status", OK);
  maze_to_json (old->maze, response_json);
  path_to_json (new_path, response_json);

  send_json (response_json);
  
  if (old->path) destroy_path(old->path);
  old->path = new_path; 
}
