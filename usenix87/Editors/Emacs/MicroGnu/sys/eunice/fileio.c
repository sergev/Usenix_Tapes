/*
 * 	Eunice BSD 4.2 file I/O
 */
#include	"def.h"

#ifndef	F_OK
#define	F_OK FRDONLY
#endif

static	FILE	*ffp;
extern	char	*getenv();

/*
 * handle C-shell style names
 */
static char *bsd(fn, buf, bufsiz) char *fn, *buf; int bufsiz;
{
	if (*fn != '~')
		return (fn);
	else {	/* C-shell $HOME-relative names */
		strncpy(buf, getenv("HOME"), bufsiz);
		strncat(buf, fn + 1, bufsiz);
		return (buf);
	}
}

/*
 * Open a file for reading.
 */
ffropen(fn) char *fn; {
	char buf[NFILEN];
	fn = bsd(fn, buf, sizeof(buf));
	if ((ffp=fopen(fn, "r")) == NULL)
		return (FIOFNF);
	return (FIOSUC);
}

/*
 * Open a file for writing.
 * Return TRUE if all is well, and
 * FALSE on error (cannot create).
 */
ffwopen(fn) char *fn; {
	char buf[NFILEN];
	fn = bsd(fn, buf, sizeof(buf));
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
ffclose() {
	(VOID) fclose(ffp);
	return (FIOSUC);
}

/*
 * Write a line to the already
 * opened file. The "buf" points to the
 * buffer, and the "nbuf" is its length, less
 * the free newline. Return the status.
 * Check only at the newline.
 */
ffputline(buf, nbuf) register char buf[]; {
	register int	i;

	/* What's with putc? */
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
 * followed by an LF. This is mainly for runoff documents,
 * both on VMS and on Ultrix (they get copied over from
 * VMS systems with DECnet).
 */
ffgetline(buf, nbuf) register char buf[]; {
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

#if	BACKUP
/*
 * Rename the file "fname" into a backup
 * copy. On Unix the backup has the same name as the
 * original file, with a "~" on the end; this seems to
 * be newest of the new-speak. The error handling is
 * all in "file.c". The "unlink" is perhaps not the
 * right thing here; I don't care that much as
 * I don't enable backups myself.
 */
fbackupfile(fn) char *fn; {
	register char	*nname;
	char		*malloc();
	char		buf[NFILEN];

	fn = bsd(fn, buf, sizeof(buf));
	if ((nname=malloc(strlen(fn)+1+1)) == NULL) {
		ewprintf("Can't get %d bytes", strlen(fname) + 1);
		return (ABORT);
	}
	(void) strcpy(nname, fn);
	(void) strcat(nname, "~");
	(void) unlink(nname);			/* Ignore errors.	*/
	if (rename(fname, nname) < 0) {
		free(nname);
		return (FALSE);
	}
	free(nname);
	return (TRUE);
}
#endif
/*
 * The string "fn" is a file name.
 * Perform any required case adjustments. All sustems
 * we deal with so far have case insensitive file systems.
 * We zap everything to lower case. The problem we are trying
 * to solve is getting 2 buffers holding the same file if
 * you visit one of them with the "caps lock" key down.
 * On UNIX file names are dual case, so we leave
 * everything alone.
 */
/*ARGSUSED*/
adjustcase(fn) register char *fn; {
#if	0
	register int	c;

	while ((c = *fn) != 0) {
		if (c>='A' && c<='Z')
			*fn = c + 'a' - 'A';
		++fn;
	}
#endif
}

#ifdef	STARTUP
#include <sys/file.h>
/*
 * find the users startup file, and return it's name. Check for
 * $HOME/.mg then for $HOME/.emacs, then give up.
 */

char *
startupfile() {
	register char	*file;
	static char	home[NFILEN];
	char		*getenv();

	if ((file = getenv("HOME")) == NULL) return NULL;
	if (strlen(file)+7 >= NFILEN - 1) return NULL;
	(VOID) strcpy(home, file);
	file = &(home[strlen(home)]);
	*file++ = '/';

	(VOID) strcpy(file, ".mg");
	if (access(home, F_OK ) == 0) return home;

	(VOID) strcpy(file, ".emacs");
	if (access(home, F_OK) == 0) return home;

	return NULL;
}
#endif
