/*
 * Os9/68k fileio.c for MicroGnuEmacs by Robert A. Larson
 *	 system dependent file io routines
 */
#include	"def.h"

char	*getenv();

static	FILE	*ffp;

/*
 * Open a file for reading.
 */
ffropen(fn)
char	*fn;
{
	if ((ffp=fopen(fn, "r")) == NULL)
		return (FIOFNF);
	return (FIOSUC);
}

/*
 * Open a file for writing.
 * Return TRUE if all is well, and
 * FALSE on error (cannot create).
 */
ffwopen(fn)
char	*fn;
{
	if ((ffp=fopen(fn, "w")) == NULL) {
		ewprintf("Cannot open file for writing");
		return (FIOERR);
	}
	return (FIOSUC);
}

/*
 * Close a file.
 * Should look at the status.
 */
ffclose()
{
	fclose(ffp);
	return (FIOSUC);
}

/*
 * Write a line to the already
 * opened file. The "buf" points to the
 * buffer, and the "nbuf" is its length, less
 * the free newline. Return the status.
 * Check only at the newline.
 */
ffputline(buf, nbuf)
register char	buf[];
{
	register int	i;

	for (i=0; i<nbuf; ++i)
		putc(buf[i]&0xFF, ffp);
	putc('\n', ffp);
	if (ferror(ffp) != FALSE) {
		ewprintf("Write I/O error");
		return (FIOERR);
	}
	return (FIOSUC);
}

/*
 * Read a line from a file, and store the bytes
 * in the supplied buffer. Stop on end of file or end of
 * line. Don't get upset by files that don't have an end of
 * line on the last line; this seem to be common on CP/M-86 and
 * MS-DOS (the suspected culprit is VAX/VMS kermit, but this
 * has not been confirmed. If this is sufficiently researched
 * it may be possible to pull this kludge). 
 */
ffgetline(buf, nbuf)
register char	*buf;
{
	register int	c;
	register int	i;

	i = 0;
	for (;;) {
		c = getc(ffp);
		if (c==EOF || c=='\n')		/* End of line.		*/
			break;
		if (i >= nbuf-1) {
			ewprintf("File has long line");
			return (FIOERR);
		}
		buf[i++] = c;
	}
	if (c == EOF) {				/* End of file.		*/
		if (ferror(ffp) != FALSE) {
			ewprintf("File read error");
			return (FIOERR);
		}
		if (i == 0)			/* Don't get upset if	*/
			return (FIOEOF);	/* no newline at EOF.	*/
	}
	buf[i] = 0;
	return (FIOSUC);
}

#ifdef BACKUP
/*
 * Rename the file "fname" into a backup copy.
 * The backup copy is the same name with ".BAK" appended unless the file
 * name is to long.  If your file name is 28 characters long ending in ".BAK"
 * you lose.  The error handling is all in "file.c". 
 */
fbackupfile(fname)
char	*fname;
{
	register char	*params;
	int	status;
	register char	*fn;
	register int	fnamel;
	register int	fnl;
	char	*rindex();

	if((fn = rindex(fname, '/')) == NULL) fn = fname; else fn++;
	fnamel = strlen(fname);
	fnl = strlen(fn);
	if((params = malloc(strlen(fname)+strlen(fn)+6)) == NULL) 
		return(ABORT);
/* delete the old backup */
	strcpy(params, fname);
	if(fnl < 25) strcat(params, ".BAK");
		else strcpy(params+(fnamel-fnl+24), ".BAK");
	unlink(params);				/* ignore errors */
/* now do the rename (This is rather akward) */
	strcpy(params, fname);
	strcat(params, " ");
	strcat(params, fn);
	if(fnl < 25) strcat(params, ".BAK");
		else strcpy(params+fnamel+1+24, ".BAK");
	if(os9fork("rename", strlen(params)+1, params, 0, 0, 0, 0)==-1) {
		free(params);
		return (FALSE);
	}
	wait(&status);
	free(params);
	return ((status & 0xffff)==0);
}
#endif

/*
 * The string "fn" is a file name.
 * Perform any required case adjustments. All sustems
 * we deal with so far have case insensitive file systems.
 * We zap everything to lower case.  The problem we are trying
 * to solve is getting 2 buffers holding the same file if
 * you visit one of them with the "caps lock" key down.
 * On UNIX file names are dual case, so we leave
 * everything alone.  Os9's dual case storage but non-case sensitivity 
 * does not seem to be accounted for here.  I'm treating it as a 
 * mono-case system, but it would be better to beleive the file (if found)
 * or the user (if not).
 */
adjustcase(fn)
register char	*fn;
{
	register int	c;

	while ((c = *fn) != 0) {
		if (c>='A' && c<='Z')
			*fn = c + 'a' - 'A';
		++fn;
	}
}

#ifndef MICRO
char *startupfile()
{
	static char startname[64];
	char *cp;
  
	if ((cp = getenv("HOME")) == NULL) return ".mg";
	strncpy(startname, cp, 64 - 4);
	strcat(startname, "/.mg");
	return startname;
}
#endif
