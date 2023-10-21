/************************************************************************
 *									*
 *			Copyright (c) 1982, Fred Fish			*
 *			    All Rights Reserved				*
 *									*
 *	This software and/or documentation is released for public	*
 *	distribution for personal, non-commercial use only.		*
 *	Limited rights to use, modify, and redistribute are hereby	*
 *	granted for non-commercial purposes, provided that all		*
 *	copyright notices remain intact and all changes are clearly	*
 *	documented.  The author makes no warranty of any kind with	*
 *	respect to this product and explicitly disclaims any implied	*
 *	warranties of merchantability or fitness for any particular	*
 *	purpose.							*
 *									*
 ************************************************************************
 */



/*
 *  TEST PROGRAM
 *
 *	testtcp   test termcap functions
 *
 *  KEY WORDS
 *
 *	test routines
 *	termcap test
 *
 *  SYNOPSIS
 *
 *	termcap [-efns] terminal [capability [capability ...]]
 *
 *		-e  =>   expand string capability given by -s
 *		-f  =>   determine boolean capabilities for terminal
 *		-n  =>   determine numeric capabilities for terminal
 *		-s  =>   determine string capabilities for terminal
 *
 *		terminal =>  terminal name as given in termcap file
 *		capability => a boolean, numeric, or string capability
 *
 *		NOTE:  All capabilities must be of same type, as
 *		       given by [-fns].
 *
 *		If terminal is only argument then entire entry is
 *		printed.
 *
 *  DESCRIPTION
 *
 *	Provides way to test termcap functions.  Can find
 *	and print an entire termcap terminal entry, or various
 *	capabilities from the entry.
 *
 *  AUTHOR
 *
 *	Fred Fish
 *
 */

#include <stdio.h>

#define TRUE 1
#define FALSE 0
#define NO_FILE	 -1			/* Returned if can't open file */
#define NO_ENTRY  0			/* Returned if can't find entry */
#define SUCCESS   1			/* Returned if entry found ok */
#define TRUNCATED 2			/* Returned if entry found but trunc */
#define BUFFER_SIZE 1024

int eflag = FALSE;
int fflag = FALSE;
int nflag = FALSE;
int sflag = FALSE;

int got_terminal = FALSE;
int got_capability = FALSE;

int ospeed = 15;	/* fake lots of padding */


/*
 *  FUNCTION
 *
 *	main   termcap test entry point
 *
 *  KEY WORDS
 *
 *	main
 *
 *  SYNOPSIS
 *
 *	main(argc,argv)
 *	int argc;
 *	char *argv[];
 *
 *  DESCRIPTION
 *
 *	This is where the termcap test starts executing.  All argument list
 *	switches are processed first, then all the specified
 *	capability identification strings are processed.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin main
 *	    Process command line options.
 *	    For each argument list field
 *		If field was not erased during option processing
 *		    If terminal name field not yet processed then
 *			Process an assumed terminal name field.
 *			Set terminal name processed flag.
 *		    Else
 *			Process a capability field.
 *			Set capability field processed flag.
 *		    End if
 *		End if
 *	    End for
 *	    If no capabilities processed then
 *		Simply dump buffer.
 *	    End if
 *	End main
 *
 */

main(argc, argv)
int argc;
char *argv[];
{
    char *argp;
    int argnum;
    char buffer[BUFFER_SIZE];

    options(argc,argv);
    for (argnum = 1; argnum < argc; argnum++) {
        if ((argp = argv[argnum]) != NULL) {
	    if (!got_terminal) {
		terminal(buffer,argp);
		got_terminal = TRUE;
	    } else {
		capability(argp);
		got_capability = TRUE;
	    }
        }
    }
    if (got_terminal && !got_capability) {
	printf("size = %d\n%s",strlen(buffer),buffer);
    }
}


/*
 *  FUNCTION
 *
 *	options   process command line options
 *
 *  SYNOPSIS
 *
 *	options(argc,argv)
 *	int argc;
 *	char *argv[];
 *
 *  DESCRIPTION
 *
 *	Scans argument list, processing each switch as it is
 *	found.  The pointer to each switch string is then
 *	replaced with a NULL to effectively erase the switch
 *	argument.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin options
 *	    For each argument in the argument list
 *		Get pointer to first char of argument.
 *		If the argument is a switch then
 *		    Replace argument pointer with NULL.
 *		    Look at next argument character.
 *		    While there is another argument character
 *			Switch on the argument character
 *			Case "EXPAND":
 *			    Set expand (e) flag.
 *			    Break out of switch.
 *			Case "BOOLEAN":
 *			    Set boolean (f) flag.
 *			    Break out of switch.
 *			Case "NUMERIC":
 *			    Set numeric flag.
 *			    Break out of switch.
 *			Case "STRING":
 *			    Set string flag.
 *			    Break out of switch.
 *			Default:
 *			    Abort with usage message.
 *			End switch
 *		    End while
 *		End if
 *	    End for
 *	End options
 *
 */


options(argc, argv)
int argc;
char *argv[];
{
    int i;
    char c;		/* 1st char of current command-line argument */
    char *cp;		/* current argument pointer */

    for (i=1; i<argc; i++) {
        cp = argv[i];
        if (*cp == '-') {
            argv[i] = NULL;
	    cp++;
	    while (c = *cp++) {
	        switch (c) {
		case 'e':
		    eflag = TRUE;
		    break;
		case 'f':
		    fflag = TRUE;
	            break;
	        case 'n':
		    nflag = TRUE;
	            break;
	        case 's':
		    sflag = TRUE;
	            break;
	        default:
	            usage();
	        }
            }
        }
    }
}


/*
 *  FUNCTION
 *
 *	usage   give usage message and abort
 *
 *  KEY WORDS
 *
 *	usage
 *	help processing
 *	abort locations
 *
 *  SYNOPSIS
 *
 *	usage()
 *
 *  DESCRIPTION
 *
 *	Usage is typically called when a problem has been
 *	detected in the argument list.
 *	It prints a usage message and exits.
 *
 */


/*
 *  PSEUDO CODE
 *
 *	Begin usage
 *	    Print usage message.
 *	    Exit.
 *	End usage
 *
 */

usage()
{
    printf("Usage: termcap [-fns] terminal [capability [capability ... ]]\n");
    exit();
}



terminal(buffer,name)
char *buffer;
char *name;
{
    int status;

    status = tgetent(buffer,name);
    switch (status) {
    case NO_FILE:
	fprintf(stderr,"Can't find a termcap data base file.\n");
	exit();
    case NO_ENTRY:
	fprintf(stderr,"Can't find entry \"%s\"\n",name);
	exit();
    case TRUNCATED:
	fprintf(stderr,"Warning --- entry \"%s\" too long\n",name);
	break;
    case SUCCESS:
        break;
    default:
        fprintf(stderr,"? tgetent returned illegal status %d\n",status);
	exit();
    }
}


capability(id)
char *id;
{
    int value;
    char buffer[256];
    char *area;
    char *ep, *tgoto();

    if (fflag) {
	value = tgetflag(id);
	if (value) {
	    printf("%s TRUE\n",id);
	} else {
	    printf("%s FALSE\n",id);
	}
    } else if (nflag) {
	value = tgetnum(id);
	printf("%s = %o octal %d decimal\n",id,value,value);
    } else if (sflag) {
	area = buffer;
	tgetstr(id,&area);
	if (eflag) {
	    ep = tgoto(buffer,75,23);
	}
	doprint(id,buffer);
	if (eflag) {
	    doprint(id,ep);
	    ep = tgoto(buffer,1,2);
	    doprint(id,ep);
	}
    }
}


/*
 *  Use tputs to get a clearer picture of exactly what
 *  goes out to the terminal....
 */

princ(c)
int c;
{
	if (c < 040)
	    printf("^%c",c |= 0100);
	else
	    printf("%c",c);
}

doprint(id,cp)
char *id;
char *cp;
{
    printf("%s = \"",id);
    tputs(cp, 1, princ);
    printf("\"\n");
}

