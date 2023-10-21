$ !
$ ! Use with DCL, not csh.
$ !
$	babble :== write sys$output
$	babble "Creating 0readme."
$ 	create 0readme.

	Welcome to the net distribution of MAKE.  MAKE is a program I wrote
to provide the same functionality as the program by the same name that is
provided with UNIX.  Please note that this is not a MAKE clone!  The syntax
is slightly different, and the semantics can be very different.  Personally,
I like my version better, but I wrote it, so I don't have an unbiased view
on that matter.  I guess I really should have come up with a different name
for MAKE, but anybody who has spent many months switching between systems can
appreciate the use for keeping the command names the same everywhere...

	The main difference between my MAKE and the "other" one is the approach
to makefiles.  Under UNIX, a makefile is essentially a data structure describing
the program to be generated.  My MAKE sees a makefile as more of a program to
be executed than a static data structure.  Rudimentary (I mean RUDIMENTARY)
variables and flow control structures are provided.  See MAKE.DOC for a more
complete discussion of just how this version of MAKE works.

	To create the initial version of MAKE, use MAKEMAKE.COM.  Note that
you will have to edit it to remove the leading X's before you invoke it.  After
MAKEMAKE has finished, you should never need it again, since, if you ever need
to make a new version, just use MAKE...

	If you are running version 4 VMS, you may want to consider defining
VMS_V4 to change some of the library to file name translations.  The difference
is this: under version 3, modules names were truncated to 9 characters and had
underscores removed to generate file names.  If VMS_V4 is defined, this
translation will not be performed.  This behavior can be changed at run time
with the MAKE$VMS_V4 symbol as well.

	MAKE runs as a DCL foreign command, so you need to define a symbol
of the following form (either in your login.com file, or in syslogin.com):

		make :== $disk:[directory]make

where DISK and DIRECTORY are the right names to give a pathname to the MAKE
executable.

	Note that MAKE needs a subprocess quota of at least 1 to run, as well as
TMPMBX privelege.

	Please return bug reports to me, and I will try to spread them around.


Jonathan Corbet
National Center for Atmospheric Research, Field Observing Facility
(303) 497-8793		{hplabs|seismo}!hao!boulder!jon
$
$ 	babble "Creating 1makefile.
$	create 1makefile.
!
! Makefile for make version 3.
!
# defclp
# define c_opts /nolist
# define linkopts /nomap
# ifdef INSTALL
	# define linkopts ^linkopts /notrace
# endif
# define LIB	makelib.olb
# define MAKE$TIMER
# define MAKE$FILE_STATS
!
! Fancy constructions.
!
# cons .c (.olb)
	# message ^MAKE$SOURCE\	[^MAKE$OUT_OF_DATE]
	- cc ^c_opts ^MAKE$SOURCE
	- library ^LIB ^MAKE$SOURCE
	- delete ^MAKE$SOURCE.obj;*
# end
# cons .c .obj
	# message ^MAKE$SOURCE\	[^MAKE$OUT_OF_DATE]
	- cc ^c_opts ^MAKE$SOURCE
# end

# cons x .olb
	# message Library is gone -- recreating
	- library /create ^$
	# wait
# end

!
! Now do it.
!
^LIB:

make.obj:		make
# debug findcons
^LIB(chdir):		make
^LIB(cons|cons.c):		ltype
^LIB(ccons):
^LIB(charutl):
^LIB(checkdef):
^LIB(comline):
^LIB(dcl):
^LIB(defclp):
^LIB(dirty):
^LIB(docond):		fdate
^LIB(execute):
^LIB(fdate):		fdate	make
^LIB(file):		make
^LIB(filetype):
^LIB(findcons):		make
^LIB(gettarget):
^LIB(libtarget):
^LIB(linetype):		ltype
^LIB(log):
^LIB(mexit):
^LIB(subproc):		make
^LIB(processl):		ltype
^LIB(refsym):
^LIB(search):		make
^LIB(setdebug):
^LIB(sharp):		make
^LIB(shexec):
^LIB(shmake):		make	fdate
^LIB(shskip):
^LIB(skiptoels):	make
^LIB(stab):
^LIB(timer):		make
^LIB(whoarewe):
!
! Do the link, if required.
!
# cons .obj .exe
	link ^linkopts make,^LIB/lib,sys$library:crtlib/lib
# end
# wait
make.exe:	^LIB(chdir)	^LIB(cons)	^LIB(ccons)	^LIB(charutl) \
		^LIB(comline)	^LIB(dcl)	^LIB(dirty)	^LIB(docond)  \
		^LIB(execute)	^LIB(fdate)	^LIB(file)	^LIB(filetype)\
		^LIB(findcons)	^LIB(gettarget)	^LIB(libtarget)	^LIB(linetype)\
		^LIB(mexit)	^LIB(processl)	^LIB(refsym)	^LIB(sharp)   \
		^LIB(skiptoels) ^LIB(stab)	^LIB(search)	^LIB(checkdef)\
		^LIB(timer)	^LIB(defclp)	^LIB(subproc)	^LIB(log)     \
		^LIB(whoarewe)
# dump file_stats

$
$ 	babble "Creating CCONS.C"
$	create ccons.c
/*
 * Routines to handle the construction table.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
# include <stdio.h>

# define MAX_LINES 20           /* Maximum # of lines in a construction */
# define VMS
/*
 * A construction table entry.
 */
struct cons
{
        char c_s_type[10];              /* Source type          */
        char c_d_type[10];              /* Dest type            */
        char *c_lines[MAX_LINES + 1];   /* Construction lines   */
        struct cons *c_next;            /* Next entry           */
};


/*
 * The construction table.
 */
static struct cons *c_tbl = NULL;

extern char * malloc ();


struct cons *ct_add_cons (source, dest)
char *source, *dest;
/*
 * Add a construction to the table.
 */
{
        struct cons *cp, *ecp;
        char src[50], dst[50];

/*
 * Convert the types to upper case.
 */
# ifdef VMS
        str$upcase (descr_n (src, 50), descr (source));
        str$upcase (descr_n (dst, 50), descr (dest));
        src[strlen (source)] = NULL;
        dst[strlen (dest)] = NULL;
# else
        strcpy (src, source);
        strcpy (dst, dest);
# endif
/*
 * See if the construction already exists.
 */
        ecp = NULL;
        for (cp = c_tbl; cp != NULL; cp = cp->c_next)
        {
                if ( ! strcmp (src, cp->c_s_type) &&
                     ! strcmp (dst, cp->c_d_type))
                {
                        ecp = cp;
                        break;
                }
        }
/*
 * If it exists, free up all the old lines.
 */
        if (ecp)
        {
                int i;

                for (i = 0; ecp->c_lines[i]; i++)
                {
                        free (ecp->c_lines[i]);
                        ecp->c_lines[i] = 0;
                }
        }
/*
 * Otherwise allocate a new structure and add it to the list.
 */
        else
        {
                cp = (struct cons *) malloc (sizeof (struct cons));
                strcpy (cp->c_s_type, src);
                strcpy (cp->c_d_type, dst);
                cp->c_next = c_tbl;
                cp->c_lines[0] = NULL;
                c_tbl = ecp = cp;
        }
/*
 * In either case, we now have ecp pointing to the new entry.
 */
        return (ecp);
}



ct_dump_ctbl ()
/*
 * Dump out the construction table.
 */
{
        int i;
        struct cons *cp;

        for (cp = c_tbl; cp != NULL; cp = cp->c_next)
        {
                printf ("\nFrom %s  To %s", cp->c_s_type, cp->c_d_type);
                for (i = 0; cp->c_lines[i]; i++)
                        printf ("\n\t%s", cp->c_lines[i]);
        }
}



ct_add_line (cp, line)
struct cons *cp;
char *line;
/*
 * Add a line to the given construction.
 */
{
        int i;
/*
 * Find the end of the current line list.
 */
        for (i = 0; cp->c_lines[i]; i++) ;
        if (i >= MAX_LINES)
                c_panic ("Max line count exceeded");
/*
 * Allocate a new line.
 */
        if ((cp->c_lines[i] = malloc (strlen (line) + 1)) == NULL)
                c_panic ("Out of memory");
        strcpy (cp->c_lines[i], line);
        cp->c_lines[i + 1] = NULL;
}




char **
ct_find_cons (source, dest)
char *source, *dest;
/*
 * Search for a construction with the given source and dest types.  If
 * found, a pointer to a list of construction lines is returned.  Else,
 * NULL is returned.
 */
{
        struct cons *cp;
        char src[10], dst[10];

/*
 * Convert the source and dest types to upper case.
 */
# ifdef VMS
        str$upcase (descr_n (src, 10), descr (source));
        src[strlen (source)] = NULL;
        str$upcase (descr_n (dst, 10), descr (dest));
        dst[strlen (dest)] = NULL;
# else
        strcpy (src, source);
        strcpy (dst, dest);
# endif
/*
 * Now search the construction table.
 */
        for (cp = c_tbl; cp != NULL; cp = cp->c_next)
                if (! strcmp (cp->c_s_type, src) &&
                    ! strcmp (cp->c_d_type, dst))
                        return (cp->c_lines);
        return (NULL);
}
$
$ 	babble "Creating charutl.c"
$	create charutl.c
# include <stdio.h>
# include <ctype.h>
/*
 * Various character utilities.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */

char *
l_blank (line)
char *line;
/*
 * Return a pointer to the first non-white space of line.  The first char
 * is ignored.
 */
{
	if (*line)
	{
		line++;
		while (*line == ' ' || *line == '\t')
			line++;
	}
	return (line);
}



char *
mknstr (line, nch)
char *line;
int nch;
/*
 * Dynamically allocate a string long enough to hold nch characters, 
 * starting at "line".
 */
{
	char *cp;

	if ((cp = malloc (nch + 1)) == NULL)
		c_panic ("Out of memory.");
	strncpy (cp, line, nch);
	cp[nch] = NULL;
	return (cp);
}




char *
non_alpha (line)
char * line;
/*
 * Return a pointer to the first non-alpha character in the line, which
 * may be the terminating null.
 */
{
	while (isalnum (*line) || *line == '_' || *line == '$')
		line++;
	return (line);
}



strcpyUC (dest, source)
char *dest, *source;
/*
 * Copy "source" to "dest", converting to upper case.
 */
{
	char *ss = source, *sd = dest;
	while (*source)
		if (islower (*source))
			*dest++ = *source++ - 'a' + 'A';
		else
			*dest++ = *source++;
	*dest = NULL;
}
$
$	babble	"Creating CHDIR.C"
$	create chdir.c
# include <stdio.h>
# include "make.h"

ch_dir (dir)
char *dir;
/*
 * Change the default directory of both processes to the given one.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
{
	sys$setddir (descr (dir), ___, ___);
	st_defsym ("MAKE$SETDDIR", "$ set default ");
	p_write (st_symcat ("MAKE$SETDDIR", dir));
}
$
$	babble "Creating CHECKDEF.C"
$	create checkdef.c
/*
 * Check symbol definitions to see if it is one of interest.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
# define fun extern int
fun	tim_init (), dcl_silent (), dcl_noisy ();
fun	tim_show (), fd_dis_file (), fd_en_file (), fd_dis_lib (), fd_en_lib ();

static struct special_symbol
{
	char *ss_sym;		/* The special symbol	*/
	int (*ss_d_fun) ();	/* Function to call when defined */
	int (*ss_u_fun) ();	/* Function to call when ufdef'd */
} sp_syms[] =
{
	{"MAKE$TIMER",		tim_init,	tim_show	},
	{"MAKE$SILENT",		dcl_silent,	dcl_noisy	},
	{"MAKE$DIS_FILE_CACHE",	fd_dis_file,	fd_en_file	},
	{"MAKE$DIS_LIB_CACHE",	fd_dis_lib,	fd_en_lib	},
	{ 0 }
};



check_def (sym)
char *sym;
/*
 * Check sym, which is now being defined, for a special symbol.
 */
{
	int i;

	for (i = 0; sp_syms[i].ss_sym; i++)
		if (! strcmp (sym, sp_syms[i].ss_sym))
		{
			(*sp_syms[i].ss_d_fun) (sym);
			return;
		}
}


check_undef (sym)
char *sym;
/*
 * Check sym, which is being undefined, for a special symbol
 */
{
	int i;

	for (i = 0; sp_syms[i].ss_sym; i++)
		if (! strcmp (sym, sp_syms[i].ss_sym))
		{
			(*sp_syms[i].ss_u_fun) (sym);
			return;
		}
}
$
$	babble "Creating COMLINE.C"
$	create comline.c
# include <stdio.h>

com_line (argc, argv)
int argc;
char **argv;
/*
 * Interpret the command line.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
{
	int param = 1;
	char cparam[20], *ep, *strchr ();
	char *file = "1makefile.";

/*
 * Just loop on each parameter.
 */
	argc--; argv++;
	for (; argc; argc--, argv++)
	{
	/*
	 * Check for a command line flag.
	 */
		if (**argv == '-')
		{
			switch (*(*argv + 1))
			{
			/*
			 * -f filename -- Specify an alternate
			 * 		  makefile.
			 */
			   case 'f':
			   case 'F':
				if (--argc)
				{
					file = argv[1];
					argv++;
				}
				else
				{
					printf ("\nMissing file with -f");
					exit (1);
				}
				break;
			/*
			 * -dsymbol = value -- Give a symbol an initial
			 *		       definition.
			 */
			   case 'd':
			   case 'D':
				if ((ep = strchr (*argv, '=')) == NULL)
					st_defsym (*argv + 2, "");
				else
				{
					*ep++ = NULL;
					st_defsym (*argv + 2, ep);
				}
				break;
			   default:
				printf ("\nBad flag: %s", *argv);
			}
		}
	/*
	 * Otherwise this is a positional parameter.  Define it.
	 */
		else
		{
			sprintf (cparam, "%d", param++);
			st_defsym (cparam, *argv);
		}
	}
/*
 * Open the input file.
 */
	file_open (file);
/*
 * Define the argc symbol.
 */
	sprintf (cparam, "%d", param - 1);
	st_defsym ("MAKE$ARGC", cparam);
}
$
$	babble "Creating CONS.C"
$	create cons.c
# include <stdio.h>
# include "ltype.h"

cons (line)
char *line;
/*
 * Handle a # cons line.  It is assumed that the "#cons" part has
 * been removed.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
{
	char from[10], to[10], *cp, *token (), *l_blank ();
	char tok[20];
	int const, bad;

/*
 * Check for the existence of the source type.
 */
	if (*line == NULL)
	{
		printf ("\nNull #cons.");
		return;
	}
/*
 * Get the source type.
 */
	cp = token (line, from);
	if (*cp == NULL)
	{
		printf ("\nMissing destination type in #cons");
		return;
	}
/*
 * Get the destination type.
 */
	cp = token (l_blank (cp), to);
	if (*cp != NULL)
		printf ("\nExtra stuff after #cons ignored: %s", cp);
/*
 * Define the construction.
 */
	const = ct_add_cons (from, to);
/*
 * Now grab up the lines.
 */
	while (file_getline ("MAKE$CONS_LINE", 0))
	{
	/*
	 * Get the line.
	 */
		if ((cp = st_getsym ("MAKE$CONS_LINE")) == NULL)
			c_panic ("Cant find input line");
	/*
	 * Check for a # end.
	 */
		if (*cp == C_SHARP)
		{
			token (l_blank (cp), tok);
			if (! strcmp (tok, "end"))
				return;
			bad = (! strcmp (tok, "ifdef")) ||
			      (! strcmp (tok, "else" )) ||
			      (! strcmp (tok, "endif")) ||
			      (! strcmp (tok, "cons" )) ||
			      (! strcmp (tok, "include"));
			if (bad)
			{
				printf ("\n# %s may not appear with a # cons", tok);
				m_exit ();
			}

		}
	/*
	 * Add the line to the construction.
	 */
		ct_add_line (const, cp);
	}
/*
 * We really shouldn't be here.
 */
	printf ("\nMissing # end for construction definiton.");
}
$
$	babble "Creating DCL.C"
$	create dcl.c
# include <stdio.h>
static int all_silent = FALSE;

dcl (lp)
char *lp;
/*
 * Handle a DCL command line.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
{
	int silent;
	char *l_blank ();
/*
 * Close libraries, just in case.
 */
	fd_close_lib ();
/*
 * A '-' means don't echo the command.
 */
	if (*lp == '-')
	{
		silent = TRUE;
		lp = l_blank (lp);
	}
	else
		silent = FALSE;
/*
 * > implies no $ at the beginning.
 */
	if (*lp == '>')
	{
		lp = l_blank (lp);
		st_defsym ("MAKE$DCL", "");
	}
	else
		st_defsym ("MAKE$DCL", "$ ");
/*
 * Shove off the command.
 */
	p_write (st_symcat ("MAKE$DCL", lp));
/*
 * Echo it if desired.
 */
	if (! silent && ! all_silent)
		printf ("\n%s\r", st_getsym ("MAKE$DCL"));
}


/*
 * Routines for define MAKE$SILENT.
 */
dcl_silent ()
/*
 * Make all commands silent.
 */
{
	all_silent = TRUE;
}


dcl_noisy ()
/*
 * Make things noisy again.
 */
{
	all_silent = FALSE;
}
$
$	babble "Creating DEBUG.H"
$	create debug.h
/*
 * Debug flags.
 */
int db_flags = 0;

/*
 * Flag definitions.
 */
# define 	D_FINDCONS	0x0001
# define	D_FDATE		0x0002
# define	D_DOCOND	0x0004
# define	D_P_WRITE	0x0008
/*
 * Debug macro.
 */
# define debug(flag)	(db_flags & flag)
$
$ 	babble "Creating DEFCLP.C"
$	create defclp.c
# include <stdio.h>

defclp ()
/*
 * Define symbols that appeared on the command line.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
{
	char symbuf[200], *cp;
	int argc;
/*
 * Get the number of command line parameters.
 */
	sscanf (st_getsym ("MAKE$ARGC"), "%d", &argc);
/*
 * Now define them.
 */
	for (; argc; argc--)
	{
		sprintf (symbuf, "%d", argc);
		if ((cp = st_getsym (symbuf)) == NULL)
			c_panic ("Can't get symbol %s", symbuf);
		st_defsym (cp, "");
	/*
	 * Also define in upper case for compatibility.
	 */
		strcpyUC (symbuf, cp);
		st_defsym (symbuf, "");
		check_def (symbuf);
	}
}
$
$	babble "Creating DIRTY.C"
$	create dirty.c
# include <stdio.h>

dirty (line)
char *line;
/*
 * Process a #dirty line.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
{
	int lib;
/*
 * See if this is a library specification.
 */
	lib = (strchr (line, '(')) != NULL;
	if (lib)
	{
	/*
	 * Parse the specification.
	 */
		st_defsym ("MAKE$DIRTY", line);
		lib_target ("MAKE$DIRTY", "MAKE$XXX", "MAKE$D_MOD", ".OLB");
		fd_lib_dirty (st_getsym ("MAKE$DIRTY"),
			      st_getsym ("MAKE$D_MOD"));
	}
	else
		fd_dirty (line);
}
$
$	babble "Creating DOCOND.C"
$	create docond.c
# include <stdio.h>
# include "fdate.h"

do_cond (line)
char *line;
/*
 * Handle a conditional line.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
{
	char *cp, *get_target ();
	int h_lib, lib, exist, fd_date (), no_source;
	char **cons, **find_cons (), tok[200];
	f_date_t t_date, check;

/*
 * Get the target file.
 */
	if ((cp = get_target (line, "MAKE$TARGET")) == NULL)
		return;
/*
 * See if we have a library reference here.
 */
	lib = (strchr (st_getsym ("MAKE$TARGET"), '(') != NULL);
	if (lib)
		lib_target ("MAKE$TARGET", "MAKE$SOURCE", "MAKE$MODULE", ".OLB");
/*
 * Get the construction number.
 */
	if ((cons = find_cons ("MAKE$TARGET", "MAKE$SOURCE", lib)) == NULL)
	{
		printf ("\nUnable to make %s.", line);
		m_exit ();
	}
	no_source = strcmp (st_getsym ("MAKE$SOURCE_TYPE"), "X") == 0;
/*
 * Get the date of the target file.
 */
	if (! lib)
		exist = fd_date (st_getsym ("MAKE$TARGET"), t_date, 0);
	else
	{
		exist = fd_lib_date (st_getsym ("MAKE$TARGET"),
				     st_getsym ("MAKE$MODULE"), t_date);
		if (! exist)
		{
			printf ("\n%s does not exist -- can't make %s",
				st_getsym ("MAKE$TARGET"),
				st_getsym ("MAKE$MODULE"));
			m_exit ();
		}
		exist = (exist == 1);
	}
/*
 * If the target does not exist, make it.
 */
	if (! exist)
	{
		st_defsym ("MAKE$OUT_OF_DATE", st_getsym ("MAKE$TARGET"));
		execute (cons, lib);
		return;
	}
/*
 * Get the date of the source file, and compare.
 */
	if (! no_source)
	{
		st_defsym ("MAKE$FSN", st_getsym ("MAKE$SOURCE"));
		st_symcat ("MAKE$FSN", st_getsym ("MAKE$SOURCE_TYPE"));
		if (! fd_date (st_getsym ("MAKE$FSN"), check, 0))
		{
			printf ("\n%s disappeared!", st_getsym ("MAKE$FSN"));
			m_exit ();
		}
		if (fd_cmp_date (t_date, check) < 0)
		{
			st_defsym ("MAKE$OUT_OF_DATE", st_getsym ("MAKE$FSN"));
			execute (cons, lib);
			return;
		}
	}
/*
 * Now we get to check all the include files.
 */
	for (cp = l_blank (token (cp, tok)); tok[0];
	     cp = l_blank (token (cp, tok)))
	{
	/*
	 * Define this file name as a symbol.
	 */
		st_defsym ("MAKE$INCLUDE", tok);
	/*
	 * If this is a library reference, parse it.
	 */
		h_lib = strchr (tok, '(') != NULL;
		if (h_lib)
			lib_target ("MAKE$INCLUDE", "MAKE$INCF", "MAKE$MODULE",
				    ".TLB");
		else
		{
			st_defsym ("MAKE$INCF", tok);
		/*
		 * Force an include file type of ".H" if necessary.
		 */
			if (! file_type (st_getsym ("MAKE$INCF")))
				st_symcat ("MAKE$INCF", ".H");
		}
	/*
	 * Now get the date.
	 */
		if (h_lib)
		{
			exist = fd_lib_date (st_getsym ("MAKE$INCLUDE"),
					     st_getsym ("MAKE$MODULE"), check);
			if (exist != 1)
			{
				printf ("\n%s does not exist in %s",
					st_getsym ("MAKE$MODULE"),
					st_getsym ("MAKE$INCLUDE"));
				printf (" -- can't make %s",
					st_getsym ("MAKE$SOURCE"));
				m_exit ();
			}
		}
		else
		{
			exist = fd_date (st_getsym ("MAKE$INCF"), check, 1);
			if (! exist)
			{
				printf ("%s does not exist -- can't make %s",
					st_getsym ("MAKE$INCF"),
					st_getsym ("MAKE$SOURCE"));
				m_exit ();
			}
		}
	/*
	 * Now check the dates.
	 */
		if (fd_cmp_date (t_date, check) < 0)
		{
			if (h_lib)
			{
				st_defsym ("MAKE$OUT_OF_DATE",
					st_getsym ("MAKE$INCLUDE"));
				st_symcat ("MAKE$OUT_OF_DATE", "(");
				st_symcat ("MAKE$OUT_OF_DATE",
					st_getsym ("MAKE$MODULE"));
				st_symcat ("MAKE$OUT_OF_DATE", ")");
			}
			else
				st_defsym ("MAKE$OUT_OF_DATE",
					st_getsym ("MAKE$INCF"));
			execute (cons, h_lib);
			return;
		}
	}
}
$
$	babble "---- End of MAKE part 1 ----"
$	exit
-- 
Jonathan Corbet
National Center for Atmospheric Research, Field Observing Facility
{seismo|hplabs}!hao!boulder!jon		(Thanks to CU CS department)
