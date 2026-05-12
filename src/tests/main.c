#include <check.h>
#include <stdlib.h>
#include "tests.h"

int
main (void)
{
  int number_failed = 0;

  number_failed += test_generate ();
  number_failed += test_solve ();
  number_failed += test_rl_solver ();
  number_failed += test_cave ();

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
