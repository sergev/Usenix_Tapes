#include "test0.h"

static void init (), init_1 ();

static void
init ()
{
  init_1 ();
} 


static ofile bye ("/dev/tty");

static void
init_1 ()
{
  bye << "And a very good bye from bye.cc!\n";
}
