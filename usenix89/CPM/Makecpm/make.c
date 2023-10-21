
/* 
 *
 * Changed for CPM 3.0 w/Aztec C by Mike Kersenbrock  Sept. 1985 
 *				    Aloha, Oregon
 *
 * Note:  CP/M 3.0 must be an implementation with timestamps
 *	  (and the timestamps turned "on")
 *
 *	The routine "ftime()" returns the "last-update" time in
 *	the same format as UNIX (tm of AT&T) time() routine, excepting
 *	for files.  This probably is the same as stat(somethingorother).
 *
 *	This program works by generating a submit file, then
 *	"chaining" execution to it.
 *
 *	Below is an edited version of header upon header added upon by
 *	the zillion of versions this has been through.  All names 
 *	and ownership/copyright notices are kept.  The original version
 *	was for the DeSmet IBM PC compiler, later modifed for the Lattice
 *	compiler (still for the IBM PC).
 *
 *	In addition to O.S. conversion, I also added several minor
 *	enhancements:
 *
 *		o Verbose usage message
 *		o Makefile lines starting with # are comment lines
 */

/* 

   John M. Sellens' Make, modified to run with Lattice C 2.04 
   Changed for Lattice C by Guido van Rossum, these changes are
   Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1984. 


   Wildcard characters filenames are not useful.

   Extra features (for Unixophiles):
	- a colon is allowed after a target;
	- make without parameters makes the first target in the first file read.

   Make Version 1.1, May 14, 1984

	Any "contributions" gratefully accepted.  I "suggest"
	a contribution of $25 (assuming that you find this useful).

	(c) Copyright, 1984, John M. Sellens

	This a called 'Make' and is a much simplified version of
	the make utility on UNIX (a trademark or something of AT&T).

	'Make' takes a file of dependencies (a 'makefile') and
	decides what commands have to be executed to bring the files
	up to date.  These commands are either executed directly through
	'Make' or written to the standard output without executing
	them.

'Makefile' format:
	- There must be a 'makefile'; you can't take input from the
	standard input.
	- The default name of the 'makefile' is 'MAKEFILE.DAT' on the
	default disk.  Different 'makefiles' can be specified using
	the '-f' option on the command line.  If the '-f' option is
	used, the default 'makefile' is not processed.
	- Any blank lines in the 'makefile(s)' are ignored.
	- A line in a 'makefile' that starts with a tab character is
	a 'howto' line and consists of a command name followed by
	arguments.  The command name must be a full file name, e.g.
	'cc.exe'.  When commands are executed, the PATH environment
	variable is used to find the command, in (hopefully) the
	same manner as DOS does.  If a command name contains a ':'
	or a '\', it is assumed to be a complete pathname and the
	PATH is not searched.  'Howto' lines apply to the most
	recently preceding 'dependency' line.  It is an error for
	a 'howto' line to precede the first 'dependency' line.
	- Any other non-blank line is a 'dependency' line.  'Dependency'
	lines consist of a filename followed by a (possibly empty) list
	of dependent filenames.

Operation:
	Syntax:
		make [filename] [-f makefilename] [-i] [-n]
	-i means continue even if an error is encountered while
		executing a command.
	-n means don't execute the commands, just write the ones that
		should be executed to the standard output.  This is useful
		for creating batch files, for example.
	-f specifies that the following argument is the name of a makefile
		to be used instead of the default (MAKEFILE.DAT).
	All arguments may be repeated and relative position of the
	arguments is not important.  If multiple definitions of a file
	are found, only the first one is significant.

	First, 'Make' reads all of the makefiles.  It then proceeds through
	all of the filename arguments, 'making' each one in turn.  A file
	is remade if it is out of date with respect to the files it depends
	on or is non-existent.	Dependencies are processed in a 'tree' fashion,
	so that the lowest-order files are remade first.

	The code is a little kludgy in places.

	No guarantees or warranties of any kind:  I think it works and
	I use it.

	Any suggestions for improvements gratefully accepted.

	I believe that commercial versions exist.  I also beleive that they
	would be superior to this.

	If you do not have DeSmet C, I am willing to provide source,
	object, and linked files for 'Make', 'Find' and 'Path' for a price.
	Send me a floppy in a sturdy mailer, and $15 (you can tell I really
	don't want to do this, but I will), and I will return your disk to you.
	The $15 includes postage back to you.

		>-------------------------------------<

Written by John M Sellens, April, 1984

Code is all original except where indicated otherwise.


After August, 1984:
	c/o 1135 Lansdowne Ave. SW
	Calgary, Alberta
	T2S 1A4

(c) Copyright 1984 John M Sellens
Permission is granted to use, distribute and/or modify this code unless
done for direct commercial profit.  If you find these routines useful,
modest contributions (monetary or otherwise) will be gratefully accepted.
Author's name, address and this notice must be included in any copies.


		.--------------------------------------<
*/



#define AZTEC 		/* Use AZTEC C compiler and library */
#include "a:stdio.h"

#define TRUE 1
#define FALSE 0

#define void int 	/* AZTEC C doesn't know (void) -- yet */

#ifdef max
#undef max	 	/* AZTEC stdio.h defines max and min... */
#endif

#ifdef min
#undef min    /* ...it shouldn't because this bites our own min/max routines */
#endif

#define errout(s) fputs(s, stderr) /* No need for DeSmet kludge */

#define MAKERUN "MAKE@@@.SUB" /* File on which commands are written */

#define DEFAULT "MAKEFILE.DAT"

#define INMAX	BUFSIZ		/* maximum input line length (1024 bytes) */


struct howrec {
	char *howcom,*howargs;
	struct howrec *nexthow;
};

struct deprec {
	char *name;
	struct defnrec *def;
	struct deprec *nextdep;
};

struct defnrec {
	char *name;
	int uptodate;
	long modified;
	struct deprec *dependson;
	struct howrec *howto;
	struct defnrec *nextdefn;
};

struct dorec {
	char *name;
	struct dorec *nextdo;
};

struct defnrec *defnlist;
struct dorec *dolist;

int execute;
int stopOnErr;
int madesomething;
int knowhow;
long LongNull = 0l;	/* This is used to make ftime() return its answer */
long ftime();		/* instead of loading it to LongNull itself	  */
FILE *execfile;


main(argc,argv)
int argc;
char *argv[];
{

	long make();

	unlink(MAKERUN);	/* delete possible existing make-submit file */
	init(argc,argv);

	/* now fall down the dolist and do them all */

	while (dolist != NULL) {
		madesomething = FALSE;
		(void)make(dolist->name);	/* ignore return value */
		if (!madesomething) {
			if (knowhow) {
				errout("Make: '");
				errout(dolist->name);
				errout("' is up to date\r\n");
			} else {
				errout("Make: Don't know how to make '");
				errout(dolist->name);
				errout("'\r\n");
				if (stopOnErr)
					exit(-1);
			}
		}
		dolist = dolist->nextdo;
	}
	
	if (execfile == NULL) {
		exit(0);	/* no file made, must be up to date! */
	}
	exec("era ",MAKERUN); /* have the submit file delete itself */
	fclose(execfile);	/* close the writing to this file */
	strcpy((char *)0x80,MAKERUN);	/* load submit file name into DMA buf*/
	bdos(47,0xff);		/* chain to the generated submit file */
}


init(argc,argv)
int argc;
char *argv[];
{
    int i, usedefault;
    dolist = NULL;
    defnlist = NULL;
    usedefault = TRUE;
    execute = TRUE;
    stopOnErr = TRUE;

    for (i=1; i < argc; i++) {
    	if (argv[i][0] == '-') {	  /* option */
            switch (argv[i][1]) {
                case 'f':
		case 'F':  		  /* arg following is a makefile */
                    if (++i < argc) {
                        readmakefile(argv[i]);
                        usedefault = FALSE;
                    } 
		    else {
                        errout("Make: '-f' requires filename\r\n");
                        exit(-1);
                    }
                    break;

                case 'i':
		case 'I':    		/* ignore errors on execution */
                    stopOnErr = FALSE;
                    break;

                case 'n':
		case 'N':       /* don't execute commands - just print */
                    execute = FALSE;
                    break;

                default:
                  errout("Make: unknown option '");
		  errout(argv[i]);
                  errout("'\r\n");
		  errout("Usage: make [filename] [-f makefilename] [-i] [-n]");
                  errout("'\r\n");
		  errout("	-f specifies a makefile instead \r\n");
		  errout("	   of \"MAKEFILE.DAT\".\r\n");
		  errout("	-n for no execution\r\n");
		  errout("	-i don't stop on errors\r\n");
		  exit();
            }
        } 
        else {		    /* it must be something to make */
            add_do(argv[i]);
        }
    }
    if (usedefault)
        readmakefile(DEFAULT);
    if (dolist == NULL && defnlist != NULL)
      add_do(defnlist->name);     /* Default: make first entry in first file */
}

long make(s)	/* returns the modified date/time */
char *s;
{

	struct defnrec *defnp;
	struct deprec *depp;
	struct howrec *howp;
	long latest, max();


	/* look for the definition */

	defnp = defnlist;
	while (defnp != NULL) {
		if (strcmp(defnp->name,s) == 0) {
			break;
		}
		defnp = defnp->nextdefn;
	}

	if (defnp == NULL) {	/* don't know how to make it */
		knowhow = FALSE;
		latest = ftime(s,&LongNull);
		if (latest==0) { /* doesn't exist but don't know how to make */
			errout("Make: Can't make '");
			errout(s);
			errout("'\r\n");
			exit(-1);
		}
		else { /* exists,assume it's up to date since we don't know */
			return(latest);
		}
	}

	if (defnp->uptodate)
		return(defnp->modified);

	/* now make sure everything that it depends on is up to date */

	latest = 0;
	depp = defnp->dependson;
	while (depp != NULL) {
		latest = max(make(depp->name),latest);
		depp = depp->nextdep;
	}

	knowhow = TRUE;       /* has dependencies therefore we know how */

	/* if necessary, execute all of the commands to make it */
	/* if (out of date) || (depends on nothing)		*/

	if (latest > defnp->modified || defnp->dependson==NULL) {
		/* make those suckers */
		howp = defnp->howto;
		while (howp != NULL) {
			printf("%s %s\n",howp->howcom,howp->howargs);
			if (execute) {
				char filename[100];	/* extra space */
				if (find(howp->howcom,filename)) {
					if (exec(filename,howp->howargs)!= 0) {
						errout("\r\nMake: error on '");
						errout(filename);
						errout(" ");
						errout(howp->howargs);
						errout("'");
						if (stopOnErr)
							exit(-1);
					}
				} 
				else {
					errout("\r\nMake: Can't find '");
					errout(howp->howcom);
					errout("'\r\n");
					if (stopOnErr)
						exit(-1);
				}
			}
			howp = howp->nexthow;
		}
/*	
	Only say it`s as recent as it's latest dependent - that way
	if we don't actually have a file for this dependency, it works
	ok
*/
		defnp->modified = latest;
		defnp->uptodate = TRUE;
		if (defnp->howto != NULL)	/* we had instructions */
			madesomething = TRUE;
	}

	return(defnp->modified);

}


add_do(s)
char *s;
{
	struct dorec *ptr1, *ptr2;
	char *getmem();

	ptr1 = (struct dorec *) getmem(sizeof(struct dorec));

	ptr1->name = s; /* okay since only called with an argv */
	ptr1->nextdo = NULL;

	uppercase(ptr1->name);

	/* now go down the dolist */
	if (dolist == NULL)
		dolist = ptr1;
	else {
		ptr2 = dolist;
		while (ptr2->nextdo != NULL)
			ptr2 = ptr2->nextdo;
		ptr2->nextdo = ptr1;
	}

}


readmakefile(s)
char *s;
{
	FILE *fil;
	int doneline, pos, i, j;
	char inline[INMAX], info[INMAX];
	char *getmem();
	struct defnrec *defnp, *defnp2;
	struct deprec *depp, *depp2;
	struct howrec *howp, *howp2;


	if ( (fil = fopen(s,"r")) == NULL ) {
		errout("Make: Couldn't open '");
		errout(s);
		errout("'\r\n");
		return;
	}

	while (fgets(inline,INMAX,fil) != NULL) {
						  /* strip trailing newline */

		while ((inline[strlen(inline)-1] == '\n' ||
			inline[strlen(inline)-1] == '\r') && 
			strlen(inline) > 0 ) {
			inline[strlen(inline)-1] = '\0';
		}

				/* ignore blank lines and comment lines */

		if (inline[0] == '\0' || inline[0] == '#' )
			continue;

		if (!isspace(inline[0])) {	/* start of a new definition */
			uppercase(inline);
			dropcolon(inline);

			/* get what we're defining into info */
			if (sscanf(inline,"%s ",info) != 1) {
				errout("Make: Can't scan: '");
				errout(inline);
				errout("'\r\n");
				continue;
			}

			/* test for wildcards in file being defined */
			for(i=0;info[i]!='\0';i++) {
			    if (s[i]=='?' || s[i]=='*') {
				errout("Make: Definition of '");errout(info);
				errout("' contains wildcard character(s)\r\n");
				errout("Make: This makes no sense\r\n");
				break;
			    }
			}

			/* get a new struct */
			defnp=(struct defnrec *)getmem(sizeof(struct defnrec));
			/* add it to the end of defnlist */
			if (defnlist == NULL)
				defnlist = defnp;
			else {
				defnp2 = defnlist;
				while (defnp2->nextdefn != NULL)
					defnp2 = defnp2->nextdefn;
				defnp2->nextdefn = defnp;
			}
			/* initialize it */
			defnp->name = getmem(strlen(info)+1);

			strcpy(defnp->name,info);
			defnp->uptodate = FALSE;	/* actually unknown */
		        defnp->modified = ftime(defnp->name,&LongNull);
			defnp->dependson = NULL;
			defnp->howto = NULL;
			defnp->nextdefn = NULL;

			/* now go through all of its dependecies */
			/* first move past the first name */
			pos = 0;
			while (isspace(inline[pos]))
				pos++;
			while (!isspace(inline[pos]) && inline[pos]!='\0')
				pos++;

			/* now loop through those suckers */
			doneline = FALSE;
			while (!doneline) {
				while (isspace(inline[pos]))
					pos++;
				if (inline[pos] == '\0') {
					doneline = TRUE;
					continue;
				}
				for(i = 0; !isspace(inline[pos]) &&
							   inline[pos]!='\0'; )
					info[i++] = inline[pos++];
				info[i] = '\0';
				/* get a new struct */
				depp = (struct deprec *) getmem(sizeof(struct deprec));
				/* add it to the end of deplist */
				if (defnp->dependson == NULL)
					defnp->dependson = depp;
				else {
					depp2 = defnp->dependson;
					while (depp2->nextdep != NULL)
						depp2 = depp2->nextdep;
					depp2->nextdep = depp;
				}
				depp->name = getmem(strlen(info)+1);
				strcpy(depp->name,info);
				depp->nextdep = NULL;
			}
		} 
		else {
			/* a how to line that starts with blank or tab */
			if (defnp == NULL) {
			   errout("Make: Howto line without a definition\r\n");
			   errout("Make: '");errout(inline);errout("'\r\n");
			}
			/* now split the line up into command and args */
			for (pos=0;isspace(inline[pos]); pos++);
				;
			for (i=pos; !isspace(inline[i]) && inline[i]!='\0'; i++)
				;
			/* if there is something there, allocate mem and copy */
			if (i != pos) {
				/* get a new struct */
				howp = (struct howrec *) getmem(sizeof(struct howrec));
				/* add it to the end of howlist */
				if (defnp->howto == NULL)
					defnp->howto = howp;
				else {
					howp2 = defnp->howto;
					while (howp2->nexthow != NULL)
						howp2 = howp2->nexthow;
					howp2->nexthow = howp;
				}
				/* copy command filename */
				howp->howcom = getmem(i-pos+1);
				for(j=0; pos < i; )
					howp->howcom[j++] = inline[pos++];
				howp->howcom[j] = '\0';
				/* now look for any argumentative part */
				while (isspace(inline[pos]))
					pos++;
				howp->howargs = getmem(strlen(inline)-pos + 1);
				for(i=0; inline[pos] != '\0'; )
					howp->howargs[i++] = inline[pos++];
				howp->howargs[i] = '\0';
				howp->nexthow = NULL;
			}
		}
	}
	fclose(fil);
}


uppercase(s)
char *s;
{
	for( ; *s != '\0'; s++)
		*s = toupper(*s);
}

dropcolon(s) /* Routine to change a colon to a space */
char *s;
{
	while (!isspace(*s)) {
		if (*s == ':') {
			*s = ' ';
			break;
		}
		++s;
	}
}


char *getmem(size)
int size;
{
	char *p;

	if ((p = alloc(size)) == 0) {
		errout("Make: Ran out of memory...\r\n");
		exit(-1);
	}
	return(p);
}

long max(a,b)
long a,b;
{
	return(a>b ? a : b);
}

find(command,filename)
char *command;
char *filename;
{
	strcpy(filename, command);
	return TRUE;
}

exec(command,args)
char *command;
char *args;
{
	if (!execfile) {
		execfile = fopen(MAKERUN, "w");
		if (!execfile) {
			errout("Make: can't create ");
			errout(MAKERUN);
			errout("\r\n");
			exit(-1);
		}
	}
	fprintf(execfile, "%s %s\n", command, args);
	return 0;
}

