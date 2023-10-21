
$ !
$ ! This is VMS make part three.
$ !
$	babble :== write sys$output
$	babble "Creating FILETYPE.C"
$	create filetype.c
# include <stdio.h>

char *
file_type (file)
char *file;
/*
 * Return a pointer to the type of the file, or NULL if there is
 * none.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
{
	char *dp, *strchr ();
/*
 * Find the offset to the end of the directory name.
 */
	if ((dp = strchr (file, ']')) == NULL)
		dp = strchr (file, '>');
	return (strchr (dp ? dp : file, '.'));
}
$
$	babble "Creating FINDCONS.C"
$	create findcons.c
# include <stdio.h>
# include "make.h"
# include "debug.h"

char **
find_cons (target, source, lib)
char *target, *source;
int lib;
/*
 * Find a construction for the given (indirect) target.  A NULL return
 * implies it can't be done.  If lib is TRUE, then source is used
 * instead of a modification of target for the source type.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
{
	char *fp, *tp, *wcp, *mknstr (), *st_getsym (), *strchr ();
	char *file_type (), *temp, file[200], st_symcat (), **ct_find_cons ();
	char **cons;
	int len, status, has_type;

/*
 * Get the target file name.
 */
	if ((fp = st_getsym (target)) == NULL)
		c_panic ("No file name for %s", target);
	if (debug (D_FINDCONS)) 
		printf ("\nLib is %d.  Target is %s", lib, fp);
/*
 * Get the type of the target.  Force it to be .obj if necessary.
 */
	if ((tp = file_type (fp)) == NULL)
	{
		st_symcat (target, ".OBJ");
		fp = st_getsym (target);
		tp = file_type (fp);
	}
	st_defsym ("MAKE$TARGET_TYPE", tp);
	if (debug (D_FINDCONS)) 
		printf (" is now %s", fp);
/*
 * For libraries, make the target type "(typ)" so as not to
 * confuse things in the construction table.
 */
	if (lib)
	{
		st_defsym ("MAKE$IN_LIB", "(");
		st_symcat ("MAKE$IN_LIB", tp);
		st_symcat ("MAKE$IN_LIB", ")");
		tp = st_getsym ("MAKE$IN_LIB");
	}
/*
 * Now we need to create a file name with type ".*".  If we have a
 * library here, take the name from "source".
 */
	if (lib)
	{
		char *ltp = st_getsym (source);
		if ((temp = file_type (ltp)) == NULL)
			st_defsym ("MAKE$FILE_TEMP", ltp);
		else
		{
			temp = mknstr (ltp, temp - ltp);
			st_defsym ("MAKE$FILE_TEMP", temp);
			free (temp);
		}
	}
/*
 * Else derive it from the target name.
 */
	else
	{
		temp = mknstr (fp, tp - fp);
		st_defsym ("MAKE$FILE_TEMP", temp);
		st_defsym (source, temp);
		free (temp);
	}
	st_symcat ("MAKE$FILE_TEMP", ".*;");
	wcp = st_getsym ("MAKE$FILE_TEMP");
	if (debug (D_FINDCONS)) 
		printf ("\nSearching on %s", wcp);
/*
 * Do a search on the file.
 */
	sea_init (wcp);
	cons = 0;
	while (sea_search (file))
	{
		*strchr (file, ';') = NULL;
		if (debug (D_FINDCONS)) 
			printf ("\nSearching for %s to %s", file_type(file),tp);
		if ((cons = ct_find_cons (file_type (file), tp)))
		{
			st_defsym ("MAKE$SOURCE_TYPE", file_type (file));
			break;
		}
	}
	if (debug (D_FINDCONS)) 
		printf ("\nStatus is %d, file is %s", status, file);
/*
 * If no luck, try for a create from thin air.
 */
	if (! cons)
		if ((cons = ct_find_cons ("X", tp)) != NULL)
			st_defsym ("MAKE$SOURCE_TYPE", "X");

	return (cons);
}
$
$	babble "Creating GETTARGET.C"
$	create gettarget.c
# include <stdio.h>

char *
get_target (line, sym)
char *line, *sym;
/*
 * Isolate the target file name from the conditional line.  This
 * file name is stored as "sym".  The return value is the portion
 * of the line beyond the target name, or NULL for a trashy line.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
{
	char *cp = NULL, *colon, *sp, *mknstr (), *l_blank ();
	char *strchr (), *strrchr ();

/*
 * Check for new format conditional with <
 */
	if (colon = strchr (line, '<'))
		cp = colon;
/*
 * Otherwise, look for an occurence of ": ".
 */
	else
	{
		colon = line;
		while (colon = strchr (colon + 1, ':'))
			if (*(colon + 1) == ' ')
			{
				cp = colon;
				break;
			}
		if (! cp)
			if ((cp = strrchr (line, ':')) == NULL)
			{
				printf ("\nWeird conditional: %s", line);
				return (NULL);
			}
	}
/*
 * Now, hopefully, we have cp pointing to the end of the target.
 */
	sp = mknstr (line, cp - line);
	st_defsym (sym, sp);
	return (l_blank (cp));
}
$
$	babble "Creating LINETYPE.C"
$	create linetype.c
# include "ltype.h"

char line_type (line)
char *line;
/*
 * Classify the given line.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
{
	if (*line == C_SHARP)
		return (LT_SHARP);
	else if (*line == C_COMMAND)
		return (LT_COMMAND);
	else if (strchr (line, C_COND) || strchr (line, '<'))
		return (LT_COND);
	else
		return (LT_TRASH);
}
$
$ 	babble "Creating LOG.C"
$	create log.c
log () {}	/* Someday */
$
$	babble "Creating LTYPE.H"
$	create ltype.h
/*
 * Ltype.h.
 *
 * Definitions of makefile line types.
 */
# define LT_COMMAND	1	/* DCL command		*/
# define LT_COND	2	/* Conditional line	*/
# define LT_SHARP	3	/* # line		*/
# define LT_TRASH	4	/* Unrecognized line	*/

/*
 * Command characters.
 */
# define C_SHARP	'#'	/* Indicates # line	*/
# define C_COMMAND	'$'	/* Indicates DCL line	*/
# define C_COND		':'	/* Indicates conditional */
$
$ 	babble "Creating MAKE.C"
$	create make.c
/*
 * Make program, version 3.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
# include <stdio.h>
# include "make.h"

extern int handler ();

main (argc, argv)
int argc;
char **argv;
{
	int i, cons;
	char string[80];

/*
 * Set up exit and ^C handlers.
 */
	set_ctl_c (handler);
	set_exh   (handler);
	/* set_ctl_d (); */
/*
 * Initialize the file date module.
 */
	fd_init ();
/*
 * Initialize global variables.
 */
	if_level = 0;
/*
 * Define the default symbols.  Note this has to be done before interpreting
 * the command line.
 */
	st_defsym ("for_opts", "/nolist");
	st_defsym ("mac_opts", "/nolist");
	st_defsym ("pas_opts", "/nolist /nostandard");
	st_defsym ("c_opts",   "/nolist");
	st_defsym ("msg_opts", "/nolist");
	st_defsym ("lib_opts", "");

# ifdef VMS_V4
/*
 * Uncomment this if you are running version four, and no longer wish to have
 * the module name to file name translation (remove _'s and truncate to nine
 * characters) applied.
 */
	st_defsym ("MAKE$VMS_V4", "");
# endif

/*
 * Define the default constructions.
 */
	cons = ct_add_cons ("x", ".tlb");
	       ct_add_line (cons, "library /create /text ^MAKE$TARGET");
	       ct_add_line (cons, "# wait");
	cons = ct_add_cons ("x", ".olb");
	       ct_add_line (cons, "library /create ^MAKE$TARGET");
	       ct_add_line (cons, "# wait");

	cons = ct_add_cons (".for", ".obj");
	       ct_add_line (cons, "fortran ^for_opts ^$");
	cons = ct_add_cons (".mar", ".obj");
	       ct_add_line (cons, "macro ^mac_opts ^$");
	cons = ct_add_cons (".msg", ".obj");
	       ct_add_line (cons, "message ^msg_opts ^$");
	cons = ct_add_cons (".pas", ".obj");
	       ct_add_line (cons, "pascal ^pas_opts ^$");
	cons = ct_add_cons (".c", ".obj");
	       ct_add_line (cons, "cc ^c_opts ^$");

	cons = ct_add_cons (".mar", "(.olb)");
	       ct_add_line (cons, "macro ^mac_opts ^MAKE$SOURCE");
	       ct_add_line (cons,"library ^lib_opts ^MAKE$TARGET ^MAKE$SOURCE");
	       ct_add_line (cons, "delete ^MAKE$SOURCE.obj;");

	cons = ct_add_cons (".pas", "(.olb)");
	       ct_add_line (cons, "pascal ^pas_opts ^MAKE$SOURCE");
	       ct_add_line (cons,"library ^lib_opts ^MAKE$TARGET ^MAKE$SOURCE");
	       ct_add_line (cons, "delete ^MAKE$SOURCE.obj;");
	
	cons = ct_add_cons (".msg", "(.olb)");
	       ct_add_line (cons, "message ^msg_opts ^MAKE$SOURCE");
	       ct_add_line (cons,"library ^lib_opts ^MAKE$TARGET ^MAKE$SOURCE");
	       ct_add_line (cons, "delete ^MAKE$SOURCE.obj;");

	cons = ct_add_cons (".obj", ".exe");
	       ct_add_line (cons, "link ^linkopts ^$,^LIB/lib");
	cons = ct_add_cons (".h", "(.tlb)");
	       ct_add_line (cons,
		"library ^lib_opts /text /replace ^MAKE$TARGET ^MAKE$SOURCE");

	cons = ct_add_cons (".c", "(.olb)");
	       ct_add_line (cons, "cc ^c_opts ^MAKE$SOURCE");
	       ct_add_line (cons,"library ^lib_opts ^MAKE$TARGET ^MAKE$SOURCE");
	       ct_add_line (cons, "delete ^MAKE$SOURCE.obj;");

	cons = ct_add_cons (".for", "(.olb)");
	       ct_add_line (cons, "fortran ^for_opts ^MAKE$SOURCE");
	       ct_add_line (cons,"library ^lib_opts ^MAKE$TARGET ^MAKE$SOURCE");
	       ct_add_line (cons, "delete ^MAKE$SOURCE.obj;");
/*
 * Deal with the command line.
 */
	com_line (argc, argv);
/*
 * Find out who we are.
 */
	who_are_we ();
/*
 * Fire off the subprocess.  We don't want AST's here since a ^C during
 * the spawn will leave the process around, but we won't have it's PID.
 */
	sys$setast (0);
/*	p_open (); */
	sys$setast (1);
/*
 * Process the file.
 */
	while (file_getline ("MAKE$INPUT", TRUE))
		process_line ("MAKE$INPUT");
/*
 * Clean up and exit.
 */
	if (if_level)
		printf ("\nMissing #endif.");
	m_exit ();
}




int handler ()
/*
 * Drastic cleanup and exit.
 */
{
	if (timing)
		tim_show ();
	p_zap ();
	exit (1);
}
$
$	babble "Creating MAKE.H"
$	create make.h
/*
 * Global definitions of interest.
 */
# define C_QUOTE 0x80		/* Set high bit of characters.	*/
# define ebit(c) (c & 0xff)
# define error(st) (! (st & 0x1))	/* check for system service error */
# define ___ 0

/*
 * Global variables, hopefully not too many.
 */
extern int if_level;		/* How deeply nested in #if's we are.	*/
extern int timing;		/* Are we timing or not?		*/
extern int in_sh_make;		/* Are we in a #make?			*/
$
$	babble	"Creating MEXIT.C"
$	create mexit.c
m_exit ()
/*
 * Exit the program gracefully.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
{
	p_close ();
	exit (1);
}
$
$	babble "Creating processl.c"
$	create processl.c
# include <stdio.h>
# include "ltype.h"

process_line (sym)
char *sym;
/*
 * Process an input line.  "Sym" is the name of the line.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
{
	char l_type, line_type (), *lp, *st_getsym (), *l_blank ();
	int silent;
/*
 * Classify the line.
 */
	lp = st_getsym (sym);
	/** printf ("P|%s.\n", lp); **/
	l_type = line_type (lp);
	switch (l_type)
	{
	/*
	 * Simply complain about trashy lines.
	 */
	   case LT_TRASH:
		printf ("Garbage line: %s\n", lp);
		break;
	/*
	 * DCL command.
	 */
	   case LT_COMMAND:
		lp = l_blank (lp);
		dcl (lp);
		break;
	/*
	 * Make command.
	 */
	   case LT_SHARP:
		lp = l_blank (lp);
		sharp (lp);
		break;
	/*
	 * Conditional line.
	 */
	   case LT_COND:
		do_cond (lp);
		break;
	/*
	 * Something very strange.
	 */
	   otherwise:
		c_panic ("Unknown line type %d\n", l_type);
	}
}
$
$	babble "Creating REFSYM.C"
$	create refsym.c
# include <stdio.h>
# include <ctype.h>

refsym (sym)
char *sym;
/*
 * Replace the line named by "sym" with a copy in which
 * all symbol references have been replaced with their
 * values.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
{
	char *st_getsym (), *strchr (), *mknstr (), *non_alpha ();
	char *cp, *sp, *ap;
/*
 * Make sure we have a line.
 */
	if (! st_getsym (sym))
		c_panic ("No line for symbol %s", sym);
/*
 * Copy the line over to a temporary, and reset it.
 */
	st_defsym ("MAKE$REFSYM", st_getsym (sym));
	st_defsym (sym, "");
	if ((cp = st_getsym ("MAKE$REFSYM")) == NULL)
		c_panic ("MAKE$REFSYM disappeared");
/*
 * Now substitute symbols.
 */
	while (ap = strchr (cp, '^'))
	{
	/*
	 * Copy everything up to the symbol.
	 */
		*ap++ = NULL;	/* Trashes line in symbol table! */
		st_symcat (sym, cp);
	/*
	 * Find the end of the symbol.
	 */
		if (*ap == '{')
		{
			ap++;
			if ((cp = strchr (ap, '}')) == NULL)
			{
				printf ("\nMissing '}'");
				cp = ap;
				break;
			}
			sp = mknstr (ap, cp - ap);
			cp++;
		}
	/*
	 * The symbol is not delimited by {} -- Go
	 * forward to the first non-alpha character.
	 */
		else
		{
			cp = non_alpha (ap);
			sp = mknstr (ap, cp - ap);
		}
	/*
	 * In any case, we now have:
	 *
	 *	cp -> The first character beyond the symbol
	 *	sp -> the symbol.
	 *
	 * It is now time to substitute in the symbol.
	 */
		if ((ap = st_getsym (sp)) == NULL)
		{
		/*
		 * Undefined symbol.  Be silent if it is a positional
		 * parameter, otherwise complain.
		 */
			if (strlen (sp) != 1 || ! isdigit (*sp))
				printf ("\nUndefined symbol: %s", sp);
		}
		else
			st_symcat (sym, ap);
		free (sp);
	/*
	 * Go around again.
	 */
	}
/*
 * Finally, copy anything left over at the end.
 */
	st_symcat (sym, cp);
	/** st_undef ("MAKE$REFSYM"); **/
}
$
$	babble "Creating SEARCH.C"
$	create search.c
/*
 * File searching routines.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
# include <stdio.h>
# include <rmsdef.h>
# include <fab.h>
# include <nam.h>
# include "make.h"

static struct FAB s_fab;
static struct NAM s_nam;
static char rs_name[200];
static int rs_len;
static char esn[200];
static int esl;

sea_init (file)
char *file;
/*
 * Initialize for a search on the given file.
 */
{
	int status;
/*
 * Set up the RMS blocks.
 */
	s_fab = cc$rms_fab;
	s_fab.fab$l_fna = file;
	s_fab.fab$b_fns = strlen (file);
	s_fab.fab$l_nam = &s_nam;
	s_fab.fab$l_fop = FAB$M_NAM;
	s_fab.fab$l_dna = ".*;";
	s_fab.fab$b_dns = 3;
	s_nam = cc$rms_nam;
	s_nam.nam$l_rsa = rs_name;
	s_nam.nam$b_rss = 200;
	s_nam.nam$l_esa = esn;
	s_nam.nam$b_ess = 200;
/*
 * Do the parse.
 */
	status = sys$parse (&s_fab);
	if (error (status))
	{
		errmes (&status);
		printf ("\nUnable to do parse on %s", file);
		m_exit ();
	}
}



int
sea_search (file)
char *file;
/*
 * Do a search on the file given to sea_init.  If there is another file
 * that matches the specification, return it in "file" and return TRUE.
 * Else FALSE is returned.
 */
{
	int status;

	status = sys$search (&s_fab);
	if (status == RMS$_NMF || status == RMS$_FNF)
		return (FALSE);
	else if (error (status))
	{
		errmes (&status);
		printf ("\nUnable to do search");
		m_exit ();
	}
	strncpy (file, rs_name, s_nam.nam$b_rsl);
	file[s_nam.nam$b_rsl] = NULL;
	return (TRUE);
}
$
$	babble "Creating SETDEBUG.C"
$	create setdebug.c
/*
 * Debug stuff.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
# include "debug.h"

static struct db_st
{
	char *db_name;		/* Debug flag name	*/
	int db_fl;		/* Debug flag value	*/
} debs[] = {
	{ "findcons",		D_FINDCONS	},
	{ "fdate",		D_FDATE		},
	{ "p_write",		D_P_WRITE	},
	{ 0 }
};


set_debug (flag)
char *flag;
/*
 * Toggle a debug flag.
 */
{
	int i;
/*
 * Search for the flag.
 */
	for (i = 0; debs[i].db_name; i++)
		if (! strcmp (debs[i].db_name, flag))
		{
			db_flags ^= debs[i].db_fl;
			return;
		}
	printf ("\nUnknown debug flag: %s", flag);
}
$
$	babble "Creating SHARP.C"
$	create sharp.c
# include <stdio.h>
# include "make.h"

sharp (line)
char *line;
/*
 * Handle a # line.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
{
	int nch;
	char *strchr (), *cp, *nlp, *token ();
	char tok[100];
/*
 * Pull the first token from the line.
 */
	cp = token (line, tok);
	line = l_blank (cp);
/*
 * Attempt to recognize it.
 */

/*
 * Chdir	Change to a new directory.
 */
	if (! strcmp (tok, "chdir"))
		ch_dir (line);
/*
 * Cons		Define a construction.
 */
	else if (! strcmp (tok, "cons"))
		cons (line);
/*
 * Debug.	Set debug flags.
 */
	else if (! strcmp (tok, "debug"))
		set_debug (line);
/*
 * Dirty	Clear a file/library cache entry.
 */
	else if (! strcmp (tok, "dirty"))
		dirty (line);
/*
 * Dump		Dump a table of interest.
 */
	else if (! strcmp (tok, "dump"))
		if ((cp = token (line, tok)) == NULL)
			printf ("\nNull dump command");
		else
			if (! strcmp (tok, "symbols"))
				st_dump ();
			else if (! strcmp (tok, "constructions"))
				ct_dump_ctbl ();
			else if (! strcmp (tok, "file_cache"))
				fd_dump_cache ();
			else if (! strcmp (tok, "lib_cache"))
				fd_dump_lib ();
			else if (! strcmp (tok, "file_stats"))
				fd_put_stats ();
			else if (! strcmp (tok, "vm_stats"))
				lib$show_vm ();
			else
				printf ("\nI can't dump %s!", tok);
/*
 * Defclp.	Define the command line parameters as
 *		symbols in their own right.
 */
	else if (! strcmp (tok, "defclp"))
		defclp ();
/*
 * Define	Define a symbol.
 */
	else if (! strcmp (tok, "define"))
		if (*cp == NULL)
			printf ("\nNull symbol definition.");
		else
		{
			cp = l_blank (token (line, tok));
			st_defsym (tok, cp);
			check_def (tok);
		}
/*
 * Else		Just skip to the # endif.
 */
	else if (! strcmp (tok, "else"))
		skip_to_else (FALSE);
/*
 * Endif	Decrement the #if level.
 */
	else if (! strcmp (tok, "endif"))
		if (if_level <= 0)
			printf ("\nExtra #endif.");
		else
			if_level--;
/*
 * Endm.	End of a #make range.
 */
	else if (! strcmp (tok, "endm"))
		if (! in_sh_make)
			printf ("Error -> #endm while not in a #make.\n");
		else
			in_sh_make = 0;
/*
 * Exit.	Stop reading the current file.
 */
	else if (! strcmp (tok, "exit"))
	{
		if (file_close ())
		{
			printf ("\nExit.");
			m_exit ();
		}
	}
/*
 * Ifdef	Execute the following lines iff the given
 *		symbol is defined.
 */
	else if (! strcmp (tok, "ifdef"))
		if (*cp == NULL)
			printf ("\nBad #ifdef");
		else
		{
			if_level++;
			cp = token (line, tok);
			if (*cp)
				printf ("\nJunk beyond #ifdef: %s", cp);
			if (! st_getsym (tok))
				skip_to_else (TRUE);
		}
/*
 * Include	Include the contents of another file.
 */
	else if (! strcmp (tok, "include"))
		file_open (line);
/*
 * Log		Write an entry to the log file.
 */
	else if (! strcmp (tok, "log"))
		log (line);
/*
 * Make.  Type II conditional line.
 */
	else if (! strcmp (tok, "make"))
		sh_make (line);
/*
 * Message	Put out a message to the terminal.
 */
	else if (! strcmp (tok, "message"))
		printf ("\n%s\r", line);
/*
 * Quit.	Terminate execution.
 */
	else if (! strcmp (tok, "quit"))
	{
		printf ("\nQuit.");
		m_exit ();
	}
/*
 * Undef.	Undefine a symbol.
 */
	else if (! strcmp (tok, "undef"))
		if (*cp == NULL)
			printf ("\nNull # undef.");
		else
		{
			st_undef (line);
			check_undef (line);
		}
/*
 * Wait.	Wait for the subprocess to finish
 *		whatever it is doing.
 */
	else if (! strcmp (tok, "wait"))
		p_write ("$ ");

	else
		printf ("\nBad # line: #%s %s", tok, line);
}




char *
token (line, tok)
char *line, *tok;
/*
 * Return the next token from "line" into "token".  The returned
 * pointer is to the first space beyond the token, unless "line" is
 * a null string, in which case "line" is returned.
 */
{
	char *cp, *strchr ();

	if ((cp = strchr (line, ' ')) != NULL)
	{
		int len = cp - line;
		strncpy (tok, line, len);
		tok[len] = NULL;
		return (cp);
	}
	else
	{
		strcpy (tok, line);
		while (*line)	/* Return EOS */
			line++;
		return (line);
	}
}
$
$	babble "Creating SHEXEC.C"
$	create shexec.c
# include "make.h"

int in_sh_make = 0;

sh_exec ()
/*
 * Execute all lines up to a #endm
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
{
        in_sh_make = 1;
}
$
$ 	babble "Creating SHMAKE.C"
$	create shmake.c
# include "make.h"
# include "fdate.h"

sh_make (line)
char *line;
/*
 * Implement the #make line.
 *
 *      # make target [from a b c ....]
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
{
        char target[40], incl[40], *token ();
        int exist;
        f_date_t t_date, i_date;
/*
 * Separate out the target name.
 */
        if (*line == 0)
        {
                printf ("Error -> Null # make.\n");
                return;
        }
        line = token (line, target);
/*
 * Get the date.
 */
        exist = fd_date (target, &t_date, 0);
/*
 * Check for the existence of source files.  If there are none, do the
 * make if the target does not exist.
 */
        if (*line == 0)
        {
                if (! exist)
                {
                        st_defsym ("MAKE$OUT_OF_DATE", target);
                        sh_exec ();
                }
                else
                        sh_skip ();
                return;
        }
/*
 * Otherwise we enter a loop to check each dependancy file.
 */
        while (*line)
        {
        /*
         * Pull off the next file.
         */
                line = token (line + 1, incl);
        /*
         * Get the date.
         */
                if (! fd_date (incl, &i_date, 0))
                {
                        printf ("%s does not exist -- can't make %s\n",
                                        incl, target);
                        exit (1);
                }
        /*
         * Compare dates.
         */
                if (fd_cmp_date (t_date, i_date) < 0)
                {
                        st_defsym ("MAKE$OUT_OF_DATE", incl);
                        sh_exec ();
                        return;
                }
        }
/*
 * No make here, do the skip to the #endm.
 */
        sh_skip ();
}
$
$ 	babble "Creating SHSKIP.C"
$	create shskip.c
sh_skip ()
/*
 * Skip through input to a #endm
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
{
        char *cp, *st_getsym (), tok[20], *token (), *l_blank ();

        while (file_getline ("MAKE$SKIP_ENDM", 0))
        {
        /*
         * See if this is a # line.
         */
                if ((cp = st_getsym ("MAKE$SKIP_ENDM")) == 0)
                        c_panic ("MAKE$SKIP_ENDM disappeared!");
                if (*cp == '#')
                {
                        cp = token (l_blank (cp), tok);
                        if (! strcmp (tok, "endm"))
                                return;
                }
        }

        printf ("Error -> Missing #endm\n");
        m_exit ();
}
$
$	babble "Creating SKIPTOELS.C"
$	create skiptoels.c
# include <stdio.h>
# include "make.h"

skip_to_else (else_ok)
int else_ok;
/*
 * Skip through this branch of a # if.  If "else_ok", we can stop at
 * a # else, otherwise a # else is an error.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
{
	char *cp, *l_blank (), *st_getsym (), *token (), tok[100];
	int if_count = 1;

	while (if_count)
	{
	/*
	 * Get the next line.
	 */
		if (! file_getline ("MAKE$SKIP_ELSE", FALSE))
		{
			printf ("\nMissing # endif");
			m_exit ();
		}
	/*
	 * See if this is a # line.
	 */
		if ((cp = st_getsym ("MAKE$SKIP_ELSE")) == NULL)
			c_panic ("MAKE$SKIP_ELSE disappeared");
		if (*cp == '#')
		{
			cp = token (l_blank (cp), tok);
		/*
		 * If this is a #if, we need to increment
		 * increment the if count.
		 */
			if (! strncmp (tok, "if", 2))
				if_count++;
		/*
		 * Else if it is a # else, we pay attention only if
		 * we are not nested inside skipped #if's.
		 */
			else if ((! strcmp (tok, "else")) && if_count == 1)
			{
				if (else_ok)
					if_count = 0;
				else
				printf ("\nBad #if structure - extra # else.");
			}
		/*
		 * If it is a # endif, decrement the if count, and perhaps
		 * the if level.
		 */
			else if (! strcmp (tok, "endif"))
			{
				if_count--;
				if (if_count <= 0)
					if_level--;
			}
		}
	}
}
$
$	babble "---- End of MAKE part 3 ----"
$	exit
-- 
Jonathan Corbet
National Center for Atmospheric Research, Field Observing Facility
{seismo|hplabs}!hao!boulder!jon		(Thanks to CU CS department)
