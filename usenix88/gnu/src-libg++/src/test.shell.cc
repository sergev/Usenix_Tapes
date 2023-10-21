#include "test0.h"

static void init (), init_1 ();

static void
init ()
{
  init_1 ();
} 


static void
init_1 ()
{
  printf ("forking shell...\n");
  system ("csh");
  printf ("shell exiting...");
}
