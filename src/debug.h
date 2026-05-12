#pragma once

#define DEBUG 1

#define LOG_DEBUG(f)                                                          \
  if (DEBUG)                                                                  \
    {                                                                         \
      f;                                                                      \
    }
