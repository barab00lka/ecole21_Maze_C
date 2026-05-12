#include <fcgi_stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "api_handlers.h"

/* AI-generated routing table logic */
typedef void (*route_handler_t) (char *data, Hashtable *ht);

typedef struct
{
  const char *pattern; // e.g. "/api/maze/" or "/api/maze/generate"
  route_handler_t handler;
} route;

/* маршрутизация по URL префиксу для разных функций приложения */
route *match_route (const char *uri);

route routes[] = {
  { "/api/maze/generate", handle_maze_generate },
  { "/api/maze/solve", handle_maze_solve },
  { "/api/maze/save", handle_maze_save },
  { "/api/maze/load", handle_maze_load },
  { "/api/cave/generate", handle_cave_generate },
  { "/api/cave/simulate", handle_cave_simulate },
  { "/api/cave/save", handle_cave_save },
  { "/api/cave/load", handle_cave_load },
  { "/api/qlearning/train", handle_qlearning_train },
  { "/api/qlearning/path", handle_qlearning_path },
  { NULL, NULL } // sentinel
};

// Matching function: exact matches take priority over prefix matches
route *
match_route (const char *uri)
{
  // exact matches
  for (int i = 0; routes[i].pattern != NULL; i++)
    {
      if (strcmp (uri, routes[i].pattern) == 0)
        return &routes[i];
    }
  return NULL; // 404
}

/* AI-generated code end */

void respond_debug_info (int *NREQUESTS);

int
main (void)
{
  int count = 0;

  Hashtable map = { 0 };
  ht_make (&map);

  // Main request-processing loop
  while (FCGI_Accept () >= 0)
    {
      char *uri = getenv ("REQUEST_URI");
      // 1. Parse Url prefix for later handling
      route *req_match = match_route (uri);

      // 1.5 Check request type
      char *request_method = getenv ("REQUEST_METHOD");
      if (request_method && strcmp (request_method, "POST") == 0)
        {
          // 2. Get the content length
          char *content_length_str = getenv ("CONTENT_LENGTH");
          if (content_length_str)
            {
              long content_length
                  = atol (content_length_str); // Use atol for long

              // Add a safety check to prevent reading too much data
              if (content_length > 0 && content_length < 1024 * 1024)
                { // Limit to 1MB for safety
                  // 3. Allocate a buffer and read the data from FCGI_stdin
                  char *post_data = (char *)malloc (content_length + 1);
                  if (post_data)
                    {
                      size_t bytes_read
                          = fread (post_data, 1, content_length, stdin);
                      post_data[bytes_read]
                          = '\0'; // Null-terminate the string

                      // 4 Pass POST data to handlers
                      if (req_match)
                        {
                          req_match->handler (
                              post_data,
                              &map); // call the corresponding handler
                          // 5. Send the response back to the client
                        }
                      else
                        {
                          /* if URI prefix hasn't been matched to handler -
                           * send 404 (this is already handled by nginx
                           * middleware) */
                        }
                      free (post_data);
                    }
                  else
                    {
                      printf ("Content-type: text/plain\r\n\r\nMemory "
                              "allocation error!");
                    }
                }
              else if (content_length >= 1024 * 1024)
                {
                  printf ("Content-type: text/plain\r\n\r\nError: POST data "
                          "too large.");
                  // You still need to consume the data from stdin to prevent
                  // issues
                  char buffer[1024];
                  long remaining = content_length;
                  while (remaining > 0)
                    {
                      long to_read = (remaining < 1024) ? remaining : 1024;
                      fread (buffer, 1, to_read, stdin);
                      remaining -= to_read;
                    }
                }
              else
                {
                  printf ("Content-type: text/plain\r\n\r\nNo POST data "
                          "received.");
                }
            }
          else
            {
              printf ("Content-type: text/plain\r\n\r\nContent-Length header "
                      "missing.");
            }
          /* handle GET request */
        }
      else if (request_method && strcmp (request_method, "GET") == 0)
        {
          respond_debug_info (&count);
        }
    }

  ht_destroy (&map);
  return 0;
}

void
respond_debug_info (int *NREQUESTS)
{
  char *vars[] = { "DOCUMENT_ROOT",
                   "GATEWAY_INTERFACE",
                   "HTTP_ACCEPT",
                   "HTTP_ACCEPT_ENCODING",
                   "HTTP_ACCEPT_LANGUAGE",
                   "HTTP_CACHE_CONTROL",
                   "HTTP_CONNECTION",
                   "HTTP_HOST",
                   "HTTP_PRAGMA",
                   "HTTP_RANGE",
                   "HTTP_REFERER",
                   "HTTP_TE",
                   "HTTP_USER_AGENT",
                   "HTTP_X_FORWARDED_FOR",
                   "PATH",
                   "QUERY_STRING",
                   "REMOTE_ADDR",
                   "REMOTE_HOST",
                   "REMOTE_PORT",
                   "REQUEST_METHOD",
                   "REQUEST_URI",
                   "SCRIPT_FILENAME",
                   "SCRIPT_NAME",
                   "SERVER_ADDR",
                   "SERVER_ADMIN",
                   "SERVER_NAME",
                   "SERVER_PORT",
                   "SERVER_PROTOCOL",
                   "SERVER_SIGNATURE",
                   "SERVER_SOFTWARE" };
  char *v;
  int NVARS = sizeof (vars) / sizeof (vars[0]);

  printf ("Content-type: text/plain\r\n\r\n"
          "Request number %d\n",
          ++(*NREQUESTS));

  for (int i = 0; i < NVARS; ++i)
    {
      v = getenv (vars[i]);
      if (v == NULL)
        printf ("%s: \n", vars[i]);
      else
        printf ("%s: %s\n", vars[i], v);
    }
}
