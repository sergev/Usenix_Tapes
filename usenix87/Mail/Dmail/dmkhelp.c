
/*
 * DMKHELP.C
 *
 *
 *  Matthew Dillon, 6 December 1985
 *
 *  (C) 1985  Matthew Dillon
 *
 *  Standalone C source.
 *
 *  Takes the file DMAIL.HELP (or that specified), and puts quotes and 
 * commas around each line, the output to stdout.  Used by Makefile to
 * place the help file on line (by making it a static array).  See the
 * Makefile.
 *
 */

#include <stdio.h>

#define HELPC "dmail.help"

static char buf[1024];

main(argc, argv)
char *argv[];
{
    FILE *fi;
    char *ptr;
    int len;
    register int i;

    if (argc == 1)
        fi = fopen (HELPC, "r");
    else
        fi = fopen (argv[1], "r");
    if (fi == NULL) {
        puts ("CANNOT OPEN");
        exit (1);
    }
    while (fgets (buf, 1024, fi)) {
        len = strlen(buf) - 1;
        buf[len] = '\0';
        putchar ('\"');
        for (i = 0; i < len; ++i) {
            if (buf[i] == '\"') {
                putchar ('\\');
                putchar ('\"');
            } else {
                putchar (buf[i]);
            }
        }
        puts ("\",");
    }
    puts ("NULL");
    fclose (fi);
}


