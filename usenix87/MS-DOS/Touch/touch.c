/*
 * Touch.c
 *
 *	Mimics the Unix TOUCH command.  Designed for the Microsoft C Compiler
 *	for MS-DOS, version 3.00.
 *
 *	Touch updates the modify time of the files that match its arguments.
 *	Filenames are expanded.  Nonexistant arguments are created with a file
 *	size of zero; failure of any wildcard spec cancels the entire 
 *	operation (which semantics are purely for consistency with the Unix
 *	version of the command).
 *
 *	This code is designed to be compiled with the small model.  The file
 *	SSETARGV.OBJ must be linked in in order to replace the standard 
 *	command line argument handling with that which expands wildcard 
 *	characters automatically.
 *
 *	Authored by Scott Rose and Randy Day, University of Washington
 *	Computer Science.  Public domain for eternity.
 *
 */


#include <stdio.h>
#include <sys\types.h>
#include <sys\utime.h>
#include <sys\timeb.h>
#include <sys\stat.h>


static struct utimbuf	Time;




/*
 * main
 *
 *	Main execution control.
 *
 */

main(argc, argv)

int	argc;
char	*argv[];
{
    int		index;


    if(argc == 1)	/* no arguments */
    {
        fprintf(stderr, "usage: touch <file-list>\n");
	exit(-1);
    }

    /* Check for residual wildcard characters.
     *   The XSETARGV routines, at variance with the Unix shell filename
     *   expansion semantics, pass filename arguments with unmatched wildcard
     *   characters to the caller, unmodified.  Rather than attempt to use
     *   such bogus filenames, they are detected and the operation aborted,
     *   just as if detected by a shell.
     */

    for(index = 1; index < argc; index++)
    	if(CheckArg(argv[index]))
	{
	    fprintf(stderr, "No match.\n");
	    exit(-1);
	}

    FreezeTime(&Time);

    for(index = 1; index < argc; index++)
    	Touch(argv[index]);

    exit(0);

}	/* end main */




/*
 * Touch
 *
 *	Update the modify time of the specified file, creating it if 
 *	necessary.
 *
 */

static
Touch(name)

char	*name;
{
    if(access(name, 2))		/* check write privilege */
    {
    	if(access(name, 0))	/* check existence, creat if possible */
	{
	    int	tmp;

	    if((tmp = creat(name, S_IWRITE)) == -1)
	    {
	        fprintf(stderr, "touch: unable to create %s.\n", name);
		return(-1);
	    }
	    close(tmp);
	}
	else	/* no write privilege */
	{
	    fprintf(stderr, "touch: no write privilege for %s.\n", name);
	    return(-1);
	}
    }
	    
    return(utime(name, &Time));

}	/* end Touch */




/*
 * FreezeTime
 *
 *	Record the current time in the static global struct Time.
 *
 */

static
FreezeTime(uTimeBuf)

struct utimbuf	*uTimeBuf;
{
    struct timeb	timeBuf;

    ftime(&timeBuf);
    Time.modtime = timeBuf.time;

}	/* end FreezeTime */




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
