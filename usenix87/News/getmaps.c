#ifndef	lint
char	*Rcsid = "$Header: getmaps.c,v 1.4 85/01/04 13:56:20 jay Exp $";
#endif

/*
 * getmaps
 *
 * Get the net maps from USENET as published by Karen and Mark Horton, in
 * "shar" format. Because of paranoia the sh is not used but instead a DFA
 * recognizing the appropriate commands.
 *
 * lee Ward		10/13/84
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>

char	*mapgrp = "/usr/spool/news/mod/map/news";
char	*mapdir = "/usr/lib/news/maps";
char	*seqfil = "/usr/lib/news/maps/.seq";
#ifdef	LOG
char	*logfil = "/usr/lib/news/maps/.log";
#endif	LOG

#ifdef	LOG
char	*usestr = "[-l logfil] [-g group] [-s seqfile] [-a archiv-dir]";

FILE	*logsd = NULL;

int	seqn, curn;

#else	LOG
char	*usestr = "[-g group] [-s seqfile] [-a archiv-dir]";
#endif	LOG

void	domaps(), myabort(), mkmaps(), getwrd();
#ifdef	LOG
void	log(), logtime();
#endif	LOG

main(argc, argv)
	int	argc;
	char	*argv[];
{
	int	x;
	FILE	*seqsd;

	for (x = 1; x < argc; x++) {
		if (*argv[x]++ != '-') {
			fprintf(stderr, "Bad usage\n");
			fprintf(stderr, "Usage: %s %s\n", argv[0], usestr);
			exit(-1);
		}
		switch (*argv[x]) {

#ifdef	LOG
		case 'l':
			logfil = argv[++x];
			break;
#endif	LOG
		case 'g':
			mapgrp = argv[++x];
			break;
		case 's':
			seqfil = argv[++x];
			break;
		case 'a':
			mapdir = argv[++x];
			break;
		default:
			fprintf(stderr, "Bad switch\n");
			fprintf(stderr, "Usage: %s %s\n", argv[0], usestr);
			exit(-1);
		}
	}

#ifdef	LOG
	logsd = fopen(logfil, "a");

	logtime("Start");
#endif	LOG

	if (chdir(mapdir) != 0)
		myabort("Could not change directory to %s", mapdir);

	if ((seqsd = fopen(seqfil, "r")) != NULL) {
		if (fscanf(seqsd, "%d", &seqn) != 1)
			myabort("Bad seq file");
		(void )fclose(seqsd);
	}
	if ((seqsd = fopen(seqfil, "a")) == NULL)
		myabort("Could not open seq file for writing");
	(void )fseek(seqsd, 0L, 0);

	domaps(mapgrp, seqn, seqsd);
	(void )fclose(seqsd);

#ifdef	LOG
	logtime("End");
#endif	LOG
}

void
domaps(grp, seqn, seqsd)
	char	*grp;
	FILE	*seqsd;
{
	char	nbuf[BUFSIZ], *nptr;
	struct direct **filst;
	int	nfils, x;
	struct stat stbuf;
	extern int scandir();
	int	numsort(), select();
	extern char *strcpy(), *strncat();

	if ((nfils = scandir(grp, &filst, select, numsort)) == -1)
		myabort("scandir failed");

	(void )strncpy(nbuf, grp, BUFSIZ-2);
	nptr = nbuf + strlen(nbuf);
	*nptr++ = '/';
	*nptr = NULL;
	nbuf[BUFSIZ-1] = NULL;
#ifdef	LOG
	log("Getting maps from %s", nbuf);
#endif	LOG

	for (x = 0; x < nfils; x++) {
		(void )strncpy(nptr, filst[x]->d_name,
		    BUFSIZ - (nptr - nbuf) - 1);
		if (stat(nbuf, &stbuf) != 0) {
#ifdef	LOG
			log("Could not stat %s", nbuf);
#endif	LOG
			continue;
		}
		if ((stbuf.st_mode & S_IFMT) == S_IFDIR)
			continue;

		curn = atoi(filst[x]->d_name);
		mkmaps(nbuf);
		(void )fseek(seqsd, 0L, 0);
		(void )fprintf(seqsd, "%d\n", curn);
		(void )fflush(seqsd);
	}
}

void
mkmaps(file)
	char	*file;
{
#define	SEARCH		1
#define	INAMAP		2
#define	SKIPPING	3

	char	buf[BUFSIZ], tofil[BUFSIZ], delim[BUFSIZ];
	int	state = SEARCH, sizdel = 0;
	FILE	*isd, *osd;
	extern FILE *fopen();


	if ((isd = fopen(file, "r")) == NULL) {
#ifdef	LOG
		log("Could not open %s. Skipping...", file);
#endif	LOG
		return;
	}

#ifdef	LOG
	log("Unarchive %d", curn);
#endif	LOG
	while (fgets(buf, sizeof(buf) - 1, isd) != NULL) {
		buf[sizeof(buf)] = NULL;
		switch (state) {
		case SEARCH:
			if (gotcat(buf, tofil, BUFSIZ, delim, BUFSIZ)) {
				state = INAMAP;
				sizdel = strlen(delim);
				if ((osd = fopen(tofil, "w")) == NULL) {
#ifdef	LOG
					log("Could not open %s", tofil);
#endif	LOG
					state = SKIPPING;
				}
			}
			break;
		case SKIPPING:
		case INAMAP:
			if (strncmp(buf, delim, sizdel) == 0) {
				state = SEARCH;
				if (osd != NULL)
					(void )fclose(osd);
			}
			else if (osd != NULL)
				fputs(buf, osd);
			break;
		}
	}

#ifdef	LOG
	if (state != SEARCH)
		log("Read/sync error on %s", file);
#endif	LOG

	(void )fclose(isd);

#undef	SEARCH
#undef	INAMAP
#undef	SKIPPING
}

/*
 * gotcat
 *
 * Use a DFA to recognize
 *	cat << DELIM > OUT
 * or
 *	cat > OUT << DELIM
 *
 */

/* Transition table for the DFA */
int	ttbl[9][4] = {
		1,-1,-1,-1,
		-1,6,2,-1,
		-1,-1,-1,3,
		-1,4,-1,-1,
		-1,-1,-1,5,
		-1,-1,-1,-1,
		-1,-1,-1,7,
		-1,-1,8,-1,
		-1,-1,-1,5,
	};

gotcat(buf, tofil, tofilln, delim, delimln)
	char	*buf,
		*tofil,
		*delim;
	int	tofilln,
		delimln;
{
	int	state;
	char	*ptr;

	state = 0;			/* Start state */
	while (state != -1 && state != 5) {
		/* Eat up white */
		while (*buf != '\n' && (*buf == ' ' || *buf == '\t'))
			buf++;
		if (*buf == '>') {
			buf++;
			state = ttbl[state][1];
			continue;
		}
		if (*buf == '<' && *(buf + 1) == '<') {
			buf += 2;
			state = ttbl[state][2];
			continue;
		}
		if (*buf == 'c' && *(buf + 1) == 'a' && *(buf + 2) == 't') {
			buf += 3;
			state = ttbl[state][0];
			continue;
		}
		ptr = buf;
		while (*buf != '\n' && *buf != ' ' && *buf != '\t')
			buf++;
		if (state == 2 || state == 8)
			getwrd(ptr, buf, delim, delimln);
		else if (state == 6 || state == 4)
			getwrd(ptr, buf, tofil, tofilln);
		state = ttbl[state][3];
	}

	if (state == 5)
		return(1);
	return(0);
}

void
getwrd(fc, lc, buf, maxlen)
	char	*fc,
		*lc,
		*buf;
	int	maxlen;
{
	char	*ptr, *t1ptr, *t2ptr;

	maxlen--;
	maxlen = lc - fc > maxlen ? maxlen : lc - fc;
	ptr = buf;
	t1ptr = fc;
	while (maxlen-- != 0)
		*ptr++ = *t1ptr++;
	*ptr = NULL;

	/* Strip quotes */
	ptr = buf;
	while (*ptr != NULL) {
		if (*ptr == '\\' && (*(ptr + 1) == '\'' || *(ptr + 1) == '"'))
			ptr += 2;
		else if (*ptr == '\'' || *ptr == '"') {
			t1ptr = ptr;
			t2ptr = ptr + 1;
			while ((*t1ptr++ = *t2ptr++) != NULL)
				;
		} else
			ptr++;
	}
}
/*VARARGS1*/
void
myabort(s, a, b, c, d, e, f, g, h, i, j, k, l)
	char	*s;
{

#ifdef	LOG
	if (logsd != NULL) {
		fputs("ABORT - ", logsd);
		fprintf(logsd, s, a, b, c, d, e, f, g, h, i, j, k, l);
		(void )fputc('\n', logsd);
		logtime("End");
	}
#endif	LOG
	exit(-1);
}

#ifdef	LOG
/*VARARGS1*/
void
log(s, a, b, c, d, e, f, g, h, i, j, k, l)
	char	*s;
{

	if (logsd == NULL)
		return;
	fprintf(logsd, s, a, b, c, d, e, f, g, h, i, j, k, l);
	(void )fputc('\n', logsd);
	(void )fflush(logsd);
}

void
logtime(s)
	char	*s;
{
	time_t	clock;
	extern char *ctime();

	if (logsd == NULL)
		return;
	(void )time(&clock);
	fprintf(logsd, "%s %s", s, ctime(&clock));
	(void )fflush(logsd);
}
#endif	LOG

numsort(a, b)
	struct direct **a, **b;
{
	return(atoi((*a)->d_name) - atoi((*b)->d_name));
}

select(d)
	struct direct *d;
{
	char	*cp = d->d_name;
	int	i = 0;

	while (isdigit(*cp))
		i = 10 * i + *(cp++) - '0';
	return(*cp == NULL && i > seqn);
}
