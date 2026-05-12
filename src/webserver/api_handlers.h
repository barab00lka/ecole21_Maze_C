#ifndef API_HANDLERS_H_
#define API_HANDLERS_H_

#include "ht_sep_chaining.h"

// Request handlers
/* на вход должен поступать валидный запрос в форме нуль-терминированной строки
 */
void handle_maze_generate (char *post_request, Hashtable *map);
void handle_maze_solve (char *post_request, Hashtable *map);
void handle_maze_save (char *post_request, Hashtable *map);
void handle_maze_load (char *post_request, Hashtable *map);
void handle_cave_generate (char *post_request, Hashtable *map);
void handle_cave_simulate (char *post_request, Hashtable *map);
void handle_cave_save (char *post_request, Hashtable *map);
void handle_cave_load (char *post_request, Hashtable *map);
void handle_qlearning_train (char *post_request, Hashtable *map);
void handle_qlearning_path (char *post_request, Hashtable *map);

#endif
