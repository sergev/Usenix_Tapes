

/*	-*-update-version-*-
 *	VERSION=Mon Jun  2 08:05:36 1986
 *	
 *	program tel
 *
 *	information on employees of apt and at&t is retreived
 *	from a large database, which can normally be entered
 *	by the 'postda' command. The information is stored in a
 *	personal database $HOME/.tel in order to speed up the
 *	search for regular used entries. Furthermore new entries,
 *	not available in the postda database, can be made in
 *	the personal database.
 *
 *		G. Langhout	860515
 */

#include	<ctype.h>
#include	<stdio.h>
#include	<string.h>
#include	<curses.h>
#include	<term.h>

#define		MAXLINES	100
#define		LINELEN		80
#define		MAXSTORE	9
#define		HT		'\011'

typedef	enum
{
	IS_FALSE = 0, IS_TRUE = 1
}
BOOL;

extern	char	*malloc();
extern	void 	gettuple();
extern	void	parsoptions();
extern 	void	usage();
extern	void	version();
extern	void	gettuple();
extern	void	addtodb();
extern	void	dbread();
extern	void	dbwrite();
extern	void	parser();
extern	BOOL	strinline();
extern	BOOL	lnfound();
extern	BOOL	mnfound();
extern	BOOL	fnfound();
extern	BOOL	locfound();
extern	int	searchnext();
extern	void	showtuple();
extern	void	reartuple();
extern	void	makenewentry();
extern	BOOL	removetuple();
extern	BOOL	pushtuple();
extern	BOOL	foundinpost();
extern	void	tmppush();
extern	BOOL	askwhichentry();
extern	BOOL	legalstring();
extern	int	luncmp();
extern	char	lcase();
extern	BOOL	setuppipe();
extern	BOOL	legalpostda();
extern	BOOL	readfrompipe();
extern	void	delete();
extern	void	update();
extern	BOOL	privaterecord();
extern	void	copytonewdb();
extern	void	refreshtuple();
extern	void	targetname();

FILE    *fp, *dbfp;
int     nlines, ornlines, options;
char    s;
char    line[MAXLINES][LINELEN];
char	*lineptr[MAXLINES];
char	*tmpptr[MAXSTORE];

BOOL	aflg = IS_FALSE, dflg = IS_FALSE, hflg = IS_FALSE;
BOOL	tflg = IS_FALSE, uflg = IS_FALSE, vflg = IS_FALSE;

int	lastentry, curentry;
int	c;

extern  char    *optarg;
extern  int	optind;

/*
 *	main evaluates the options, and takes subsequent action
 */
main(argc, argv)
int	argc;
char    *argv[];

{
	parsoptions(argc, argv);
	switch(options)
	{
	case 0:
		if (argc == 1)
			usage();
		else
			if (argc == 2)
				gettuple(*++argv);
			else
				addtodb(argc, argv);
		break;
	case 1:
		if (aflg) addtodb(argc, argv);
		if (dflg) delete(optarg);
		if (hflg) usage();
		if (tflg) gettuple(*(argv+2));
		if (uflg) update();
		if (vflg) version();
		break;
	default:
		usage();
		break;
	}
}

/* find options and arguments */
void
parsoptions(argc, argv)
int	argc;
char	*argv[];
{
	options = 0;

	while((c = getopt(argc, argv, "a:d:ht:uv")) != EOF)
		switch(c)
		{
		case 'a':
			options++;
			aflg = IS_TRUE;
			break;
		case 'd' :
			dflg = IS_TRUE;
			options++;
			break;
		case 'h':
			hflg = IS_TRUE;
			options++;
			break;
		case 'u':
			uflg = IS_TRUE;
			options++;
			break;
		case 't':
			tflg = IS_TRUE;
			options++;
			break;
		case 'v':
			vflg = IS_TRUE;
			options++;
			break;
		default :
			hflg = IS_TRUE;
			options++;
			break;
		}
}


/* print help function on stdio */
void
usage()
{
	printf("tel name		; search name, add to pdb if new\n");
	printf("tel name data [data]	; add to private database (pdb)\n");
	printf("tel -a name [data]	; add to private database\n");
	printf("tel -d name		; query delete from pdb\n");
	printf("tel -h			; help function\n");
	printf("tel -t name		; search but do not add to pdb\n");
	printf("tel -u			; update pdb if postda is changed\n");
	printf("tel -v			; version number\n");
}

/* print version number */
void
version()
{
	printf("Version 1.1     G. Langhout\n");
}

/*
 *	search name in personal db and display if succeeded,
 *	otherwise search in postda db and add to personal db.
 */
void
gettuple(name)
char *name;
{
	char line[LINELEN], *fn, *mn, *ln, *loc;
	char shortname[LINELEN];

	strncpy(line, name, LINELEN);
	strncpy(shortname, line, 32);
	parser(line, &fn, &mn, &ln, &loc);
	dbread();
	lastentry = -1;
	while((curentry = searchnext(lastentry, fn, mn, ln, loc)) >= 0)
	{
		showtuple(lineptr[curentry]);
		reartuple(curentry);
		lastentry = curentry;
	}
	if (lastentry != -1)	/* entry found in own db */
		dbwrite();
	else
		if (foundinpost(shortname))	/* entry found in postda db */
			dbwrite();
		else
			printf("No record found : %s \n", name);
}

/* add own entry (not from postda) to private db */
void
addtodb(argc, argv)
int    argc;
char    *argv[];
{
	int     i;
	char    s[LINELEN];

	/* construct new entry from commandline arguments */
	s[0] = HT;
	s[1] = NULL;

	if (options != 0)
	{
		strncat(s, optarg, LINELEN - strlen(s) - 1);
		strncat(s, " ", LINELEN - strlen(s) - 1);
	}
	for (i = optind; i < argc; i++)
	{
		strncat(s, argv[i], LINELEN - strlen(s) - 1);
		strncat(s, " ", LINELEN - strlen(s) - 1);
	}
	strcat(s, "\012");

	/* retreive old db and add info if there is enough room */
	dbread();
	if (pushtuple(s))
		printf("tel: information added\n");
	dbwrite();
}

/* read pdb from file $HOME/.tel */
void
dbread()
{
	char pathname[LINELEN];

	strcpy(pathname, getenv("TEL"));
	if (*pathname == NULL)
	{
		strcpy(pathname, getenv("HOME"));
		if (*pathname != NULL)
			strcat(pathname, "/.tel");
		else
			strcpy(pathname, ".tel");
	}
	dbfp = fopen(pathname, "r");
	nlines = 0;
	while(fgets(line[nlines], LINELEN, dbfp) != NULL)
	{
		lineptr[nlines] = line[nlines];
		nlines++;
	}
	ornlines = nlines;
	fclose(dbfp);
}

/* write new private db, provided the -t option is not used */
void
dbwrite()
{
	int    i;
	char pathname[LINELEN];

	if (!tflg)
	{
		strcpy(pathname, getenv("TEL"));
		if (*pathname == NULL)
		{
			strcpy(pathname, getenv("HOME"));
			if (*pathname != NULL)
				strcat(pathname, "/.tel");
			else
				strcpy(pathname, ".tel");
		}
		dbfp = fopen(pathname, "w");
		for (i=0; i<nlines; i++)
			fputs(lineptr[i], dbfp);
		fclose(dbfp);
	}
}

/*
 * split the searchname in functional parts:
 * firstname, middlename, lastname and location.
 */
void
parser(line, pfn, pmn, pln, ploc)
char *line, **pfn, **pmn, **pln, **ploc;
{

	int i,n;
	char *ptr[10],s[LINELEN],*pt, t[LINELEN];
	unsigned size;

	if (pt = strchr(line, ':'))
	{
		*pt = 0;
		*ploc = pt+1;
	}
	else
		*ploc = 0;

	*pfn = *pmn = *pln = 0;

	ptr[0] = strtok(line, ".");
	i = 1;
	while(ptr[i] = strtok(0, "."))
		i++;
	*pln = ptr[i-1];
	if (i > 1)
	{
		*pfn = ptr[0];
		if (i > 2)
			*pmn = ptr[1];
	}
}

/* is the searchname in the line ? */
BOOL
strinline(s, fn, mn, ln, loc)
char    *s, *fn, *mn, *ln, *loc;

{
	int    i, j;
	char t[LINELEN];
	char    *p;

	strcpy(t, s);

	if (!(ln && lnfound(t, ln)))
		return(IS_FALSE);
	if (fn)
		if (!fnfound(t, fn))
			return(IS_FALSE);
		else
			if (mn)
				if (!mnfound(t, mn))
					return(IS_FALSE);
	if (loc)
		if (!locfound(s, loc))
			return(IS_FALSE);
	return(IS_TRUE);
}

/* is the lastname in the line ? */
BOOL
lnfound(t, ln)
char	*t, *ln;
{
	char	*p;
	if (t = strchr(t, HT))
		if (p = strtok(++t, ","))
		{
			while((*ln != NULL) && (lcase(*ln) == lcase(*p)) && 
                              (*p != NULL))
			{
				while(*(++p) == ' ') ;
				ln++;
			}
			if (*ln == NULL)
				return(IS_TRUE);
		}
return(IS_FALSE);

}

/* is the firstname in the line ? */
BOOL
fnfound(t, fn)
char *t, *fn;
{
	char *p;

	if (p = strtok(0, ", ."))
		if (!luncmp(p, fn, strlen(fn)))
			return(IS_TRUE);
	return(IS_FALSE);
}

/* is the middlename in the line ? */
BOOL
mnfound(t, mn)
char *t, *mn;
{
	char *p;
	if (p = strtok(0, ", ."))
		if (!luncmp(p, mn, strlen(mn)))
			return(IS_TRUE);
	return(IS_FALSE);
}

/* is the location in the line ? */
BOOL
locfound(name, loc)
char *name, *loc;
{
	char *p;

	if (luncmp(name, loc, strlen(loc)) == 0)
		return(IS_TRUE);
	else
		return(IS_FALSE);
}

/*
 * return the linenumber of the next entry in which the
 * searchname is found.
 */
int
searchnext(lnum, fn, mn, ln, loc)
int    lnum;
char	*fn, *mn, *ln, *loc;

{
	BOOL    found;
	lnum++;
	while (!(found = strinline(lineptr[lnum], fn, mn, ln, loc)) && 
	    lnum < ornlines)
		lnum++;
	if (!found)
		return(-1);
	else
		return(lnum);
}

/* show the found entry */
void
showtuple(s)
char    *s;
{
	printf("%s", s);
}

/* position the entry, found in the pdb, on top */
void
reartuple(curentry)
int    curentry;
{
	char    *tptr;

	tptr = lineptr[curentry];
	for ( ; curentry > 0; curentry--)
		lineptr[curentry] = lineptr[curentry-1];
	lineptr[0] = tptr;
}

/* push a new entry on the pdb */
void
makenewentry(s)
char *s;
{
	int i;
	for (i = nlines; i > 0; i--)
		lineptr[i] = lineptr[i-1];
	lineptr[0] = malloc(LINELEN);
	strcpy(lineptr[0], s);
	nlines++;
}

/* remove oldest postda entry */
BOOL
removetuple()
{
	int i, j;

	i = nlines - 1;
	while((*lineptr[i] == HT) && (i > 0))
		i--;
	if (*lineptr[i] != HT)
	{
		for (j = i; j < nlines - 1; j++)
			lineptr[j] = lineptr[j+1];
		nlines--;
		return(IS_TRUE);
	}
	else
		return(IS_FALSE);
}

/*
 * push new entry if private db is not full,
 * if there are MAXSTORE entries, the oldest postda
 * entry will be removed. If there are no postda entries
 * left, the db is full.
 */
BOOL
pushtuple(s)
char    *s;
{
	showtuple(s);
	if ((nlines >= MAXLINES) && !removetuple())
	{
		printf("private database full, entry not added %c\n", '\007');
		return(IS_FALSE);
	}
	else
	{
		makenewentry(s);
		return(IS_TRUE);
	}
}

/* pass searchname to postda command and push a found entry */
BOOL
foundinpost(name)
char    *name;
{
	int        newentry, no_stored, curentry;
	char    s[LINELEN];

	printf("Searching.");
	fflush(stdout);
	newentry = 0;
	no_stored = 0;
	if (setuppipe(name))
	{
		while(readfrompipe(s))
		{
			if (newentry == 0)
				printf(".\n");
			if (!tflg)
			{
				tmppush(s, &no_stored);
				newentry++;
			}
			else
			{
				newentry = 1;
				showtuple(s);
			}
		}
		pclose(fp);
	}
	if (newentry == 0)
		printf(".\n");
	else
		if (newentry == 1)
			pushtuple(tmpptr[0]);
		else
			if (askwhichentry(&curentry, no_stored, newentry))
				pushtuple(tmpptr[curentry-1]);

	if (newentry == 0)
		return(IS_FALSE);
	else
		return(IS_TRUE);
}

/* store data from postda temporarily */
void
tmppush(s, pno_stored)
char *s;
int *pno_stored;

{
	if (*pno_stored < MAXSTORE)
	{
		tmpptr[*pno_stored] = malloc(LINELEN);
		strcpy(tmpptr[*pno_stored], s);
		(*pno_stored)++;
	}
}

/* ask which entry of the postda entries is to be stored */
BOOL
askwhichentry(pentry, no_stored, newentry)
int *pentry;
int no_stored, newentry;
{
	int	i;
	setupterm(0, 1, 0);
	for (i = 0; i < no_stored; i++)
	{
		vidattr(A_REVERSE);
		printf("%1d ", i+1);
		vidattr(!A_REVERSE);
		printf("%s", tmpptr[i]);
	}
	resetterm();
	if (newentry > no_stored)
	{
		printf("\n%d records were not displayed,",newentry-no_stored);
		printf(" please be more specific or use -t option\n");
	}
	printf("\nSelect record for private database (0 = no record) : ");
	scanf("%d", pentry);
	if ((*pentry > no_stored)  || (*pentry < 0))
		*pentry = 0;
	if (*pentry == 0)
		return(IS_FALSE);
	else
		return(IS_TRUE);
}

/* strip postda output from error lines */
BOOL
legalstring(s)
char    *s;
{
	if (strncmp("No record", s+1, 9) == 0)
		return(IS_FALSE);
	else
		return(IS_TRUE);
}

/* strncmp disregarding upper-lowercase */
int
luncmp(s, t, n)
char	*s, *t;
int	n;
{
int	i;
for (i = 0; i < n; i++)
if (lcase(*s++) != lcase(*t++))
	return(1);
return(0);
}

/* return lowercase character of character */
char
lcase(c)
char    c;
{
	if (isupper(c) == 0)
		return(c);
	else
		return(c + 32);
}

/* open pipe to postda process, if the searchname is legal */
BOOL
setuppipe(name)
char    *name;
{
	char	search[LINELEN];
	if (legalpostda(name))
	{
		strcpy(search, "postda ");
		strcat(search, name);

		if(fp = popen(search, "r"))
			return(IS_TRUE);
		else
			return(IS_FALSE);
	}
	else return(IS_FALSE);
}

/* check the input to postda */
BOOL
legalpostda(name)
char *name;
{
	int i;
	i = 0;
	while(name[i] != 0)
		if ((isalpha(name[i]) == 0) && 
		    (name[i] != ':') && (name[i] != '.'))
			return(IS_FALSE);
		else
			i++;
	return(IS_TRUE);
}

/* read from pipe, the error messages are skipped */
BOOL
readfrompipe(s)
char    *s;
{
	while (fgets(s, LINELEN, fp))
		if (legalstring(s))
			return(IS_TRUE);
	return(IS_FALSE);
}

void
delete(name)
char *name;
{
	char line[LINELEN], *fn, *mn, *ln, *loc;
	int	no_entry, ex_entry;
	int	entryfound[MAXLINES];
	int	i;

	setupterm(0, 1, 0);
	strcpy(line, name);
	parser(line, &fn, &mn, &ln, &loc);
	dbread();
	lastentry = -1;
	while((curentry = searchnext(lastentry, fn, mn, ln, loc)) >= 0)
	{
		entryfound[no_entry] = curentry;
		no_entry++;
		lastentry = curentry;
	}
	if (no_entry == 0)
		printf("No record found to be deleted\n");
	else
	{
		for (i = 0; i < no_entry; i++)
		{
			vidattr(A_REVERSE);
			printf("%1d ", i + 1);
			vidattr(!A_REVERSE);
			showtuple(lineptr[entryfound[i]]);
		}
		resetterm();
		printf("\nSelect record to be deleted (0 = no record) : ");
		scanf("%d", &ex_entry);
		if ((ex_entry > 0) && (ex_entry <= no_entry))
		{
			for (i = entryfound[ex_entry - 1]; i < nlines - 1; i++)
				lineptr[i] = lineptr[i + 1];
			nlines--;
			printf("Record is removed\n");
			dbwrite();
		}
		else
			printf("No record removed\n");
	}
}

/*
 * update personal database by running postda on the entries
 * in the db. The non postda entries are not changed.
 */
void
update()
{
	char    newdb[MAXLINES][LINELEN];
	int    i, newlines;
	int	*nl = &newlines;

	dbread();
	newlines = 0;
	for (i = 0; i < nlines; i++)
	{
		if (privaterecord(lineptr[i]))
			copytonewdb(newdb, lineptr[i], nl);
		else
			refreshtuple(newdb, lineptr[i], nl);
	}


	for (i = 0; i < newlines; i++)
	{
		lineptr[i] = newdb[i];

	}
	nlines = newlines;
	dbwrite();
}

/* your own entry ? */
BOOL
privaterecord(s)
char *s;
{
	if (*s == HT)
		return(IS_TRUE);
	else
		return(IS_FALSE);
}

/* copy string in temporal db */
void
copytonewdb(newdb, s, nl)
char newdb[][LINELEN];
char *s;
int	*nl;
{
	strcpy(newdb[(*nl)++], s);
}

/* place output from postda in temporal db */
void
refreshtuple(newdb, s, nl)
char newdb[][LINELEN];
char *s;
int    *nl;
{
	char t[LINELEN];
	targetname(t, s);
	if (setuppipe(t))
	{
		while (readfrompipe(t))
			copytonewdb(newdb, t, nl);
		pclose(fp);
	}
}


/* construct searchname from entry in private db, for postda */
void
targetname(t, s)
char *t, *s;
{
	char *fn, *mn, *p,*pe,full[LINELEN];

	t[0] = full[0] = '\0';
	if (p = strchr(s, HT))
	{
		strncpy(full, p+1, 25);
		strtok(full, ",");
		if (fn = strtok(0, ", ."))
		{
			strcat(t, fn);
			strcat(t, ".");
			if (mn = strtok(0,", ."))
			{
				strcat(t, mn);
				strcat(t, ".");
			}
		}
	}
pe = full;

while(*pe != NULL)
{
if (*pe != ' ')
strncat(t, pe, 1);
pe++;
}
	strcat(t, ":");
	strncat(t, s, p-s);
}
