/*
 * Chmod.c
 *
 *	Change the protection mode of the argument files to read-only or 
 *	read/write.
 *
 *	Designed for use with the Microsoft 3.00 C Compiler for MS-DOS 2.X.
 *	To effect wildcard expansion, link with the file SSETARGV.OBJ.
 *
 *	Hastily authored by Scott Rose at the University of Washington.
 *	Public domain.
 *
 */


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>




main(argc, argv)

int argc;
char *argv[];
{
    int tmp;
    int	index;

    if(argc < 3)
    {
    	fprintf(stderr, "usage: chmod r|w <file-list>\n");
	exit(-1);
    }

    if(*argv[1] == 'w')
    	tmp = S_IREAD | S_IWRITE;
    else if(*argv[1] == 'r')
    	tmp = S_IREAD;
    else
    {
    	fprintf(stderr, "chmod: invalid mode spec.\n");
	exit(-1);
    }

    for(index = 2; index < argc; index++)
    	if(CheckArg(argv[index]))
	{
	    fprintf(stderr, "No match.\n");
	    exit(-1);
	}

    for(index = 2; index < argc; index++)
    	if(chmod(argv[index], tmp))
	    fprintf(stderr, "chmod: file %s not found.\n", argv[index]);

}	/* end chmod */




/*
 * CheckArg
 *
 *	Scan the argument filename for wildcard characters.
 *
 */

static
CheckArg(name)

char	*name;
{
    register char	c;

    while(c = *name++)
	if(c == '?' || c == '*')
	    return(-1);

    return(0);

}	/* end CheckArg */
