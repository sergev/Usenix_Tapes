#include <stdio.h>

main (argc, argv)
int argc;
char *argv[];
{
#   ifdef DEBUG
    printf ("argv[0] = %d\en", argv[0]);
#   endif
    /*
     *  Rest of program
     */
#   ifdef DEBUG
    printf ("== done ==\en");
#   endif
}
