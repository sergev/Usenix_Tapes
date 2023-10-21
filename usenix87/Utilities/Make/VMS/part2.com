$
$	babble :== write sys$output
$	babble "Creating EXECUTE.C"
$	create execute.c
# include <stdio.h>

execute (list, lib)
char **list;
int lib;
/*
 * Execute the list of commands.  Lib is true iff the target is in a
 * library.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
{
	char *lp, *st_getsym ();
/*
 * For compatibility, define the symbols $ and _.
 */
	st_defsym ("$", st_getsym ("MAKE$SOURCE"));
	st_defsym ("_", st_getsym ("MAKE$OUT_OF_DATE"));
/*
 * Mark the cache entry as having the current date.
 */
	if (! lib)
		fd_current (st_getsym ("MAKE$TARGET"));
	else
		fd_lib_current (st_getsym ("MAKE$TARGET"),
				st_getsym("MAKE$MODULE"));
/*
 * Close any open libraries.
 */
	fd_close_lib ();
/*
 * Now execute the list.
 */
	for (; *list; list++)
	{
	/*
	 * Put the line into the symbol table.
	 */
		st_defsym ("MAKE$EXECUTE", *list);
	/*
	 * Substitute symbols.
	 */
		refsym ("MAKE$EXECUTE");
		lp = st_getsym ("MAKE$EXECUTE");
	/*
	 * Execute the line.  If the first character is '#',
	 * then this is a sharp line, otherwise assume a DCL
	 * line.
	 */
		if (*lp == '#')
			sharp (l_blank (lp));
		else
		{
			st_symcat ("MAKE$EXECUTE", "\t! ");
			dcl (st_symcat ("MAKE$EXECUTE"),
				st_getsym ("MAKE$OUT_OF_DATE"));
		}
	}
}
$
$	babble "Creating FDATE.C"
$	create fdate.c
# include <stdio.h>
# include <ctype.h>
# include <fab.h>
# include <rab.h>
# include <xab.h>
# include <nam.h>
# include <rmsdef.h>
# include "fdate.h"
# include "make.h"
# include "lbrdef.h"
# include "debug.h"

/*
 * This module deals with file dates.  A cache is used to store these
 * dates, in an effort to minimize RMS calls.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
# define FD_FN_LEN	200	/* File name length */

/*
 * The file cache table.
 */
static struct fd_cache_entry
{
	char fdc_name[FD_FN_LEN];	/* File name		*/
	f_date_t fdc_date;		/* Modification date	*/
	struct fd_cache_entry *fdc_next;	/* Next entry	*/
} *fd_cache[36] = { 0 };

/*
 * The library cache table.
 */
static struct fd_lib_cache
{
	char fdl_l_name[FD_FN_LEN];	/* Library name		*/
	struct NAM fdl_nam;		/* NAM block.		*/
	struct fd_cache_entry *fdl_mod;	/* List of modules.	*/
	struct fd_lib_cache *fdl_next;	/* Next entry		*/
	int fdl_index;			/* Library index	*/
	char fdl_open;			/* Library is open	*/
} *fd_l_cache[36] = { 0 };

/*
 * RMS structures.
 */
static struct FAB fd_fab;
static struct RAB fd_rab;
static struct XABDAT fd_xab;
/*
 * Statistics.
 */
static int n_try = 0, n_hit = 0, n_l_try = 0, n_l_hit = 0;
static int n_lib_open = 0;	/* Number of library opens	*/
/*
 * Various bits of data.
 */
static char n_open_lib = 0;	/* Number of libraries open	*/
static int cache_files = TRUE;	/* Should we cache files?	*/
static int cache_lib = TRUE;	/* Should we cache libraries?	*/

fd_init ()
/*
 * Initialize the file date module.  This routine must be called
 * before any of the other fd_ routines.
 */
{
/*
 * Fill in the RMS structures.
 */
	fd_fab = cc$rms_fab;
	fd_fab.fab$b_shr = FAB$M_UPI;	/* out of ddate -- who knows?	*/
	fd_fab.fab$l_xab = &fd_xab;
	fd_fab.fab$b_fac = FAB$M_GET;

	fd_rab = cc$rms_rab;
	fd_rab.rab$l_fab = &fd_fab;

	fd_xab = cc$rms_xabdat;
}





int
fd_date (file, date, cache)
char *file;
f_date_t date;
int cache;
/*
 * Return the modification date of "file" in "date".  Returns TRUE iff
 * the file exists.  Only use the cache if "cache" is defined.
 */
{
	struct fd_cache_entry *fd_get_cache (), *cep;
	char uc_file[200];
	int index, status, sys$open ();

/*
 * If it is in the cache, Just return the date.
 */
	if (cache)
	{
		strcpyUC (uc_file, file);
		cep = fd_get_cache (uc_file);
		if (cep)
		{
			fd_cp_date (date, cep->fdc_date);
			return (TRUE);
		}
	}
/*
 * Fill in the FAB.
 */
	fd_fab.fab$l_fna = file;
	fd_fab.fab$b_fns = strlen (file);
	fd_clr_date (&fd_xab.xab$q_cdt);
	fd_clr_date (&fd_xab.xab$q_rdt);
/*
 * Attempt to open the file.
 */
	status = sys$open (&fd_fab);
	if (error (status))
	{
		if (status != RMS$_FNF)
			errmes (&status);
		return (FALSE);
	}
/*
 * Return the date and close the file.
 */
	fd_cp_date (date, &fd_xab.xab$q_rdt);
	if (! date[0] && ! date[1])
		fd_cp_date (date, &fd_xab.xab$q_cdt);
	sys$close (&fd_fab);
/*
 * Add an entry into the cache.
 */
	if (cache && cache_files)
	{
		if ((cep = (struct fd_cache_entry *)
			malloc (sizeof (struct fd_cache_entry))) == NULL)
			c_panic ("Out of memory");
		strcpy (cep->fdc_name, uc_file);
		fd_cp_date (cep->fdc_date, date);
		index = fd_i_index (file);
		cep->fdc_next = fd_cache[index];
		fd_cache[index] = cep;
	}

	return (TRUE);
}



int
fd_lib_date (lib, module, date)
char *lib, *module;
f_date_t date;
/*
 * Return the insertion date of "module" into library "lib" in "date".
 * Returns:
 *	-1 = Module not in library
 *	 1 = Module found
 *	 0 = Library not found.
 */
{
	struct fd_cache_entry *fd_get_lib (), *cep;
	struct fd_lib_cache *libp;
	char uc_module[200], uc_lib[200], header[600];
	int index, insert, status, h_len;
	long lib_index, rfa[2];
/*
 * Get the module name in upper case.
 */
	strcpyUC (uc_module, module);
	strcpyUC (uc_lib, lib);
	if (debug (D_FDATE))
		printf ("\nfd_lib_date (%s, %s)", uc_lib, uc_module);
/*
 * If the module is in the cache, just return the date.
 */
	cep = fd_get_lib (uc_lib, uc_module);
	if (cep)
	{
		if (debug (D_FDATE))
			printf ("  Found it in cache.");
		fd_cp_date (date, cep->fdc_date);
		return (1);
	}
/*
 * No such luck.  Lets look through the library list to see if we have
 * opened this library yet.
 */
	insert = FALSE;
	index = fd_i_index (uc_lib);
	for (libp = fd_l_cache[index]; libp; libp = libp->fdl_next)
	{
		if (debug (D_FDATE))
			printf ("\nComparing with %s", libp->fdl_l_name);
		if (! strcmp (uc_lib, libp->fdl_l_name))
			break;
	}
/*
 * If not, create a structure.  Do not insert it into the list
 * until the open works, however.
 */
	if (! libp)
	{
		if (debug (D_FDATE))
			printf ("\nDidn't find it.");
		libp = malloc (sizeof (struct fd_lib_cache));
		libp->fdl_nam = cc$rms_nam;
		strcpy (libp->fdl_l_name, uc_lib);
		libp->fdl_next = NULL;
		libp->fdl_mod  = NULL;
		libp->fdl_open = FALSE;
		libp->fdl_index = 0;
		insert = TRUE;
	}
/*
 * If the library index has not been created, do so now.
 */
	if (! libp->fdl_index)
	{
		status = lbr$ini_control (&libp->fdl_index, &LBR$C_READ,
			&LBR$C_TYP_UNK, &libp->fdl_nam);
		if (error (status))
		{
			errmes (&status);
			printf ("\nUnable to initialize library: %s",
				libp->fdl_l_name);
			return (0);
		}
	}
/*
 * If the library is not open, do so now.
 */
	if (! libp->fdl_open)
	{
		/*
		 * If this is the first open, provide the name, else use
		 * the NAM block.
		 */
		if (insert)
			status = lbr$open (&libp->fdl_index, descr (uc_lib));
		else
			status = lbr$open (&libp->fdl_index);
		if (error (status))
		{
			errmes (&status);
			printf ("\nUnable to open library %s",libp->fdl_l_name);
			return (0);
		}
		libp->fdl_open = TRUE;
		n_lib_open++;
		n_open_lib++;
	}
/*
 * Since the open worked, we can now insert the structure into the
 * list, if necessary.
 */
	if (insert)
	{
		if (debug (D_FDATE))
			printf ("\nInserting library now.");
		libp->fdl_next = fd_l_cache[index];
		fd_l_cache[index] = libp;
	}
/*
 * Look for the module in the library.
 */
	status = lbr$lookup_key (&libp->fdl_index, descr (uc_module), rfa);
	if (status == LBR$_KEYNOTFND)
		return (-1);
	else if (error (status))
	{
		errmes (&status);
		printf ("\nUnable to lookup %s in %s\n", uc_module, uc_lib);
		return (0);
	}
/*
 * Search out the module.
 */
	status = lbr$set_module (&libp->fdl_index, rfa, descr_n (header,600),
				 &h_len);
	if (error (status))
	{
		errmes (&status);
		printf("\nUnable to set to module %s in %s", uc_module, uc_lib);
		return (0);
	}
/*
 * We have got it!  Return the date and add the module to the list.
 */
	fd_cp_date (date, &header[8]);
	if (debug (D_FDATE))
		printf ("\nGot date, cache_lib is %d", cache_lib);
	if (cache_lib)
	{
		if (debug (D_FDATE))
			printf ("\nInserting module");
		cep = malloc (sizeof (struct fd_cache_entry));
		strcpy (cep->fdc_name, uc_module);
		fd_cp_date (cep->fdc_date, date);
		cep->fdc_next = libp->fdl_mod;
		libp->fdl_mod = cep;
	}
	return (1);
}



struct fd_cache_entry *
fd_get_lib (lib, module)
char *lib, *module;
/*
 * Return a pointer to the cache entry for "module" in library "lib",
 * if one exists.
 */
{
	int index;
	struct fd_lib_cache *libp;
	struct fd_cache_entry *cep;
/*
 * If the cache is disabled, just return.
 */
	if (debug (D_FDATE))
		printf ("\n\nfd_get_lib (%s %s)", lib, module);
	if (! cache_lib)
		return (NULL);
/*
 * Attempt to find the library in the cache.
 */
	index = fd_i_index (lib);
	n_l_try++;
	for (libp = fd_l_cache[index]; libp; libp = libp->fdl_next)
		if (! strcmp (lib, libp->fdl_l_name))
			break;
	if (! libp)
	{
		if (debug (D_FDATE))
			printf ("\nFailed to find library.");
		return (NULL);
	}
/*
 * We have the library.  Now search for the module.
 */
	for (cep = libp->fdl_mod; cep; cep = cep->fdc_next)
	{
		if (debug (D_FDATE))
			printf ("\nComparing %s.", cep->fdc_name);
		if (! strcmp (module, cep->fdc_name))
		{
			if (debug (D_FDATE))
				printf (" ... Found it!");
			n_l_hit++;
			return (cep);
		}
	}
	return (NULL);
}



int
fd_cmp_date (date1, date2)
f_date_t date1, date2;
/*
 * Returns:
 *	0	if date1 = date2
 *	< 0	if date1 < date2
 *	> 0	otherwise.
 */
{
	f_date_t temp;

	if (date1[0] == date2[0]  &&  date1[1] == date2[1])
		return (0);
	subquad (date1, date2, temp);
	return (temp[1]);
}




int
fd_dirty (file)
char *file;
/*
 * Invalidate the internal idea of what the date of "file" is,
 * requiring a check next time.  There is no problem if "file" is
 * not in the cache.
 */
{
	struct fd_cache_entry *cep, *last;
	char uc_file[200];
	int index;

	strcpyUC (uc_file, file);
	if (debug (D_FDATE))
		printf ("\nFd_dirty (%s)", uc_file);
	index = fd_i_index (uc_file);
	for (cep = fd_cache[index]; cep != NULL; cep = cep->fdc_next)
	{
		if (debug (D_FDATE))
			printf ("\nComparing with %s", cep->fdc_name);
		if (! strcmp (uc_file, cep->fdc_name))
		{
			if (debug (D_FDATE))
				printf (" Found it.");
			if (cep == fd_cache[index])
				fd_cache[index] = cep->fdc_next;
			else
				last->fdc_next = cep->fdc_next;
			free (cep);
			return;
		}
		last = cep;
	}
}



fd_lib_dirty (lib, module)
char *lib, *module;
/*
 * Invalidate the cache entry for the given library and module.  The
 * nonexistence of the entry causes a no-op.
 */
{
	int index;
	char uc_lib[200], uc_module[200];
	struct fd_cache_entry *cep, *last;
	struct fd_lib_cache *libp;

/*
 * Get everything in upper case.
 */
	strcpyUC (uc_lib, lib);
	strcpyUC (uc_module, module);
/*
 * See if we can find the library.
 */
	index = fd_i_index (uc_lib);
	for (libp = fd_l_cache[index]; libp; libp = libp->fdl_next)
		if (! strcmp (uc_lib, libp->fdl_l_name))
			break;
	if (! libp)
		return;
/*
 * Look for the module.
 *
 * Would you believe that when the loop was phrased as
 *
 *	for (cep = libp->fdl_mod; cep != 0; cep = cep->fdc_next)
 *
 * that it executed the first time with cep = 0?  Well, it happened.  I
 * am too tired to start looking at assembly code to figure it out,
 * so life will just have to go on with a while.
 */
	last = NULL;
	cep = libp->fdl_mod;
	while (cep != 0)
	{
		if (! strcmp (uc_module, cep->fdc_name))
		{
		/*
		 * Found it.  If this is the first entry in the list,
		 * we have to modify the library header structure.
		 */
			if (cep == libp->fdl_mod)
				libp->fdl_mod = cep->fdc_next;
		/*
		 * Else we just unlink it.
		 */
			else
				last->fdc_next = cep->fdc_next;
		/*
		 * Free up the module structure and go away.
		 */
			free (cep);
			return;
		}
		last = cep;
		cep = cep->fdc_next;
	}
	return;
}





int fd_i_index (file)
char *file;
/*
 * Return an index into the cache derived from the file name.
 */
{
	char *cp, *strchr ();
/*
 * If there is a directory or logical name in front of the file
 * name, skip over it.
 */
	if ((cp = strchr (file, ']')) == NULL)
		if ((cp = strchr (file, '>')) == NULL)
			cp = strchr (file, ':');
	if (cp)
		file = cp + 1;

	if (isdigit (*file))
		return (*file - '0');
	else if (isalpha (*file))
		return ((islower (*file) ? _toupper (*file) : *file) - 'A' +10);
	else
	{
		printf ("\nBad file name: %s", file);
		m_exit ();
	}
}



fd_cp_date (date1, date2)
f_date_t date1, date2;
/*
 * Copy date2 to date1
 */
{
	date1[0] = date2[0];
	date1[1] = date2[1];
}



struct fd_cache_entry
*fd_get_cache (file)
char *file;
/*
 * Return a pointer to the cache entry for the given file, or
 * NULL if no such entry exists.
 */
{
	int index = fd_i_index (file);
	struct fd_cache_entry *cep;
/*
 * If the cache is disabled, just return NULL.
 */
	if (! cache_files)
		return (NULL);
/*
 * Search out the file.
 */
	n_try++;
	for (cep = fd_cache[index]; cep; cep = cep->fdc_next)
		if (! strcmp (file, cep->fdc_name))
		{
			n_hit++;
			return (cep);
		}
	return (NULL);
}



fd_clr_date (date)
f_date_t date;
/*
 * Zero out the given date.
 */
{
	date[0] = date[1] = 0;
}



fd_dump_cache ()
/*
 * Dump out a listing of the file cache.
 */
{
	int slot;
	short len;
	struct fd_cache_entry *cp;
	char asctim[30];

	for (slot = 0; slot < 36; slot++)
		for (cp = fd_cache[slot]; cp; cp = cp->fdc_next)
		{
			sys$asctim (&len, descr_n (asctim, 100), cp->fdc_date, 0);
			asctim[len] = 0;
			printf ("\n%2d\t%-40s\t%s", slot, cp->fdc_name, asctim);
		}
}



fd_dump_lib ()
/*
 * Dump out the library cache.
 */
{
	int index, len;
	struct fd_lib_cache *libp;
	struct fd_cache_entry *cep;
	char date[40];

	for (index = 0; index < 36; index++)
		for (libp = fd_l_cache[index]; libp; libp = libp->fdl_next)
		{
			printf ("\nLibrary: %s", libp->fdl_l_name);
			for (cep = libp->fdl_mod; cep; cep = cep->fdc_next)
			{
				sys$asctim (&len, descr_n (date, 40), cep->fdc_date, 0);
				date[len] = 0;
				printf ("\n\t\t%-10s\t%s", cep->fdc_name, date);
			}
		}
}




fd_put_stats ()
/*
 * Put out file cache stats.
 */
{
	printf ("\nFile: %d tries %d hits.  Lib: %d tries %d hits",
		n_try, n_hit, n_l_try, n_l_hit);
	printf (" %d LBR$OPEN calls.", n_lib_open);
}


fd_dis_file ()
/*
 * Disable file cacheing.
 */
{
	cache_files = FALSE;
}


fd_en_file ()
/*
 * Enable file cacheing.
 */
{
	cache_files = TRUE;
}


fd_dis_lib ()
/*
 * Disable library cacheing.
 */
{
	cache_lib = FALSE;
}


fd_en_lib ()
/*
 * Enable library cacheing.
 */
{
	cache_lib = TRUE;
}


fd_cur_date (date)
f_date_t date;
/*
 * Return the current date in "date".
 */
{
	sys$gettim (date);
}



fd_current (file)
char *file;
/*
 * Set the date of the file to the current date in the file cache.  If
 * the file is not in the cache, it is not inserted.
 */
{
	struct fd_cache_entry *cep;
	char uc_file[200];

	strcpyUC (uc_file, file);
	if ((cep = fd_get_cache (uc_file)) != NULL)
		fd_cur_date (cep->fdc_date);
}



fd_lib_current (lib, module)
char *lib, *module;
/*
 * Mark the cache entry for lib and module as having the current date.
 */
{
	struct fd_cache_entry *cep;
	char uc_lib[200], uc_mod[200];

	strcpyUC (uc_lib, lib);
	strcpyUC (uc_mod, module);

	if (cep = fd_get_lib (uc_lib, uc_mod))
		fd_cur_date (cep->fdc_date);
}




fd_close_lib ()
/*
 * Close all the open libraries.
 */
{
	int i;
	struct fd_lib_cache *libp;

	if (debug (D_FDATE))
		printf ("\nClose_lib call");
/*
 * If there are no open libraries, blow it off.
 */
	if (! n_open_lib)
		return;
/*
 * No such luck.  Close them.
 */
	for (i = 0; i < 36; i++)
		for (libp = fd_l_cache[i]; libp; libp = libp->fdl_next)
			if (libp->fdl_open)
			{
				if (debug (D_FDATE))
					printf (" %s ", libp->fdl_l_name);
				lbr$close (&libp->fdl_index);
				libp->fdl_open = FALSE;
			/*
			 * Always force generation of a new index on
			 * the next open.
			 */
				libp->fdl_index = 0;
				if (! --n_open_lib)
					return;
			}
/*
 * We should never get here.
 */
	printf ("\nBUG: %d libs still open", n_open_lib);
}
$
$	babble "Creating FDATE.H"
$	create fdate.h
/*
 * Definitions for file dates.
 */
typedef int f_date_t[2];
$
$	babble "Creating FILE.C"
$	create file.c
/*
 * Handle input files.
 *
 * Jonathan Corbet
 * National Center for Atmospheric Research, Field Observing Facility.
 */
# include <stdio.h>
# include "make.h"

/*
 * File pointers are kept on a stack, to facilitate handling of #includes.
 * Here is a stack entry:
 */
static struct f_stack
{
	FILE *fs_fp;			/* File pointer */
	struct f_stack *fs_next;	/* Next entry.	*/
	int fs_if_level;		/* Saved if level */
} *file_stack = NULL;
static FILE *cur_fp = NULL;

int file_open (name)
char *name;
/*
 * Open the given file, pushing the old file onto the stack.
 */
{
	char f_sym[80];
	struct f_stack *fsp;
/*
 * Attempt to open the file first.
 */
	if ((cur_fp = fopen (name, "r")) == NULL)
	{
		perror (name);
		exit (1);
	}
/*
 * Push the file onto the stack.
 */
	sprintf (f_sym, "MAKE$FILE_%X", cur_fp);
	st_defsym (f_sym, name);
	if ((fsp = malloc (sizeof (struct f_stack))) == NULL)
		c_panic ("Out of memory");
	fsp->fs_fp = cur_fp;
	fsp->fs_next = file_stack;
	fsp->fs_if_level = if_level;
	file_stack = fsp;
}


int
file_getline (sym, doref)
char *sym;
int doref;
/*
 * Read a line from the file, and define it as the value of symbol "sym".
 * if doref, then symbols within the line are substituted in.  Returns
 * TRUE iff EOF was not hit.
 */
{
	char line[200], *lp;
	int c;
	char endline = FALSE, inquote = FALSE;
/*
 * Define the symbol as null.
 */
	st_defsym (sym, "");
/*
 * Read the line.
 */
	for (lp = line; ! endline;)
	{
	/*
	 * If EOF, we are certainly done reading the line.
	 */
		if ((c = fgetc (cur_fp)) == EOF)
		{
			if (lp == line)
			{
				if (file_close ())
					return (FALSE);
				continue;
			}
			endline = TRUE;
			*lp = NULL;
			st_symcat (sym, line);
		}
	/*
	 * \ means quote the next character.  If said next character is
	 * a newline, then continue onto the next line.  Flush the line
	 * buffer too.
	 */
		else if (c == '\\')
		{
			c = fgetc (cur_fp);
			if (c == '\n')
			{
				*lp++ = ' ';  *lp = NULL;
				st_symcat (sym, line);
				lp = line;
			}
			else
				*lp++ = c | C_QUOTE;
		}
	/*
	 * Delete leading blanks.
	 */
		else if ((line == lp) && ((c == ' ') || (c == '\t')))
			continue;
	/*
	 * An unquoted newline ends the line.  However, we want to
	 * delete blank lines.  We also need to search back for a
	 * quoted blank, which implies a continuation.
	 */
		else if (c == '\n')
		{
			if (lp == line)
				continue;
		/*
		 * Trim trailing blanks.
		 */
			while (*--lp == ' ')
				*lp = NULL;
			if (lp == line)
				continue;
		/*
		 * If the character is a quoted blank, continue the line.
		 */
			if (ebit (*lp) == (' ' | C_QUOTE))
			{
				*lp = ' ';  *++lp = NULL;
				lp = line;
			}
			else
			{
				endline = TRUE;
				*++lp = NULL;
			}
			st_symcat (sym, line);
		}
	/*
	 * ! Means comment the rest of the line.  This is slightly tricky,
	 *   since we want to put a comment past a line continuation.  Thus,
	 *   we must search back to the first nonblank character, and if it
	 *   is a continuation, continue the line.
	 */
		else if ((! inquote) && (c == '!'))
		{
			while (((c = fgetc (cur_fp)) != EOF) && (c != '\n')) ;
			if (line == lp)	/* ! was first char */
				continue;
			while ((*--lp == ' ') && lp > line) ;
			if ((ebit (*lp) == (' ' | C_QUOTE)) ||
				(ebit (*lp) == ('\t' | C_QUOTE)))
			{
				*lp = ' ';
				*++lp = NULL;
				lp = line;
			}
			else
			{
				*++lp = NULL;
				endline = TRUE;
			}
			st_symcat (sym, line);
		}

	/*
	 * Normal character.  Check for '"', and convert tabs to
	 * blanks.
	 */
		else
		{
			if (c == '\t')
				*lp++ = ' ';
			else
				*lp++ = c;
			if (c == '"')
				inquote = ! inquote;
		}
	}
		
/*
 * If requested, fill in the symbol.
 */
	if (doref)
		refsym (sym);
	return (TRUE);
}




int
file_close ()
/*
 * Close the current file.  Return .true. iff
 * this was the last file.
 */
{
	struct f_stack *fsp;
/*
 * Close the file.
 */
	fclose (cur_fp);
/*
 * Pop the file off the stack.
 */
	fsp = file_stack;
	file_stack = file_stack->fs_next;
	if (file_stack)
	{
		cur_fp = file_stack->fs_fp;
		if_level = file_stack->fs_if_level;
	}
	free ((char *) fsp);
	return (file_stack == NULL);
}
$
$ 	babble "---- End of MAKE part two ----"
$	exit
-- 
Jonathan Corbet
National Center for Atmospheric Research, Field Observing Facility
{seismo|hplabs}!hao!boulder!jon		(Thanks to CU CS department)
