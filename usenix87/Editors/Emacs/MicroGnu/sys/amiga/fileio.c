/*
 * Name:	MicroEMACS
 * Version:	Gnu v30
 *		Commodore Amiga file I/O.
 * Last edit:	30-Sep-86 ...ihnp4!seismo!ut-sally!ut-ngp!mic
 * Created:	23-Jul-86 ...ihnp4!seismo!ut-sally!ut-ngp!mic
 *		(from sys/bsd/fileio.c)
 *
 * Read and write ASCII files. All
 * of the low level file I/O knowledge is here.
 * Pretty much vanilla standard I/O.
 */
#include	"def.h"
/* With Uttice, make sure you use -Idf0:include/lattice/ to	*/
/* put the Lattice files in the right spot.			*/
#include	<fcntl.h>

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
 * it may be possible to pull this kludge). Delete any CR
 * followed by an LF.
 */
ffgetline(buf, nbuf)
register char	buf[];
{
	register int	c;
	register int	i;

	i = 0;
	for (;;) {
		c = getc(ffp);
		if (c == '\r') {		/* Delete any non-stray	*/
			c = getc(ffp);		/* carriage returns.	*/
			if (c != '\n') {
				if (i >= nbuf-1) {
					ewprintf("File has long line");
					return (FIOERR);
				}
				buf[i++] = '\r';
			}
		}
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

#ifdef	BACKUP
/*
 * Make a file name for a backup copy.
 */
fbackupfile(fname)
char	*fname;
{
	strcat(fname,"~");
	return (TRUE);
}
#endif	BACKUP

#ifdef	STARTUP
/*
 * Return name of user's startup file.  On Amiga, make it
 * s:.mg
 */

static char startname[] = ".mg";
static char altstartname[] = "s:.mg";

char *startupfile()
{
	FILE *f, *fopen();
	if (f = fopen(startname,"r")) {		/* first try	*/
		fclose(f);
		return(startname);
	}
	if (f = fopen(altstartname,"r")) {	/* second try	*/
		fclose(f);
		return (altstartname);
	}
	return (NULL);
}
#endif	STARTUP

/*
 * The string "fn" is a file name.
 * Perform any required case adjustments.
 * On the Amiga file names are dual case,
 * so we leave everything alone.
 */
adjustcase(fn)
register char	*fn;
{
	return (TRUE);
}
