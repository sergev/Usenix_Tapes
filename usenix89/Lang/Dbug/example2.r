#include <stdio.h>

int debug = 0;

main (argc, argv)
int argc;
char *argv[];
{
    /* printf ("argv = %x\en", argv) */
    if (debug) printf ("argv[0] = %d\en", argv[0]);
    /*
     *  Rest of program
     */
#ifdef DEBUG
    printf ("== done ==\en");
#endif
}
