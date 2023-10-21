
/* 
 * 
 *  dis [-p predefineds] file
 *
 *  The -p option may be repeated.
 */

#include "dis.h"

char *predef[NPREDEF];
int  npredef = 0;
char *file;
char *progname = "dis";
int  bopt = 0;

initopts(argc,argv)
int argc;
char *argv[];
{
    int ai;
    char *ca;
    int fileset = 0;

    progname = argv[0];

    while (--argc) {
        if ((*++argv)[0] == '-') {
	    ca = *argv;
	    for(ai = 1; ca[ai] != '\0'; ai++)
                switch (ca[ai]) {
                case 'p':
		    predef[npredef] = *++argv;
		    npredef++;
                    argc--;
                    break;
		case 'b':
		    bopt = 1;
		    break;
                default: crash("Invalid option letter");
                }
        } else if (!fileset) {
	    file = *argv;
	    fileset++;
	} else crash("Usage: [-p predef] file");
    }
    if (!fileset)
	 crash("Usage: [-p predef] file");
}
