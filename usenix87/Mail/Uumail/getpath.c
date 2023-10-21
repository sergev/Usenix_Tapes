/*
 * Name: getpath -- return the full usenet path of the given name
 *
 * Paramaters: sysname (input) -- The system name to be expanded
 *	       pathname (output)  The usenet path of the given system name
 *	       pathfile (input) the file to search for the system name
 *
 * Returns: EX_OK     -- path found
 *	    EX_NOHOST -- path not found
 *	    EX_NOINPUT-- unable to open usemap
 *          EX_TEMPFAIL -- database being rebuilt
 *
 * Original Version Author: J. Donnelly   3/82
 *
 */

/*    IF YOU ARE USING A DBM DATABASE, READ THIS!
 *    If the special sentinel value of @@@ is not present in the
 *    database, then it is assumed that the database is being
 *    rebuilt and the requesting process is blocked for TIMEOUT
 *    (default = 180) seconds.  If, after 5 such blocks, the
 *    sentinel is not present, the error code EX_TEMPFAIL is returned.
 *    The same is true if the dbm files cannot be initialized.
 */

/***************************************************************************
This work in its current form is Copyright 1986 Stan Barber
with the exception of opath, gethostname and the original getpath which
as far as I know are in the Public Domain. This software may be distributed
freely as long as no profit is made from such distribution and this notice
is reproducted in whole.
***************************************************************************
This software is provided on an "as is" basis with no guarantee of 
usefulness or correctness of operation for any purpose, intended or
otherwise. The author is in no way liable for this software's performance
or any damage it may cause to any data of any kind anywhere.
***************************************************************************/
/* 22-jun-83 Sheppard
 * modified to rewind path file (if open), rather than open again
 *
 * $Log:	getpath.c,v $
 * Revision 4.0  86/11/17  16:02:15  sob
 * Release version 4.0 -- uumail
 * 
 * Revision 3.11  86/11/06  01:59:48  sob
 * Altered DBM to UUDBM to avoid possible conflicts under 4.3 BSD
 * Thanks to page@ulowell for the report
 * 
 * Revision 3.10  86/10/20  15:05:14  sob
 * Ready for beta test
 * 
 * Revision 3.9  86/10/20  13:23:36  sob
 * Revisions to work more correctly in SORTED and nonSORTED databases
 * 
 * Revision 3.7  86/10/10  18:24:16  sob
 * Moved dbm.h include here from uuconf.h
 * 
 * Revision 3.6  86/10/06  15:03:23  sob
 * Fixed another problem with getting neighbors in getpath.
 * 
 * Revision 3.5  86/10/01  15:49:15  sob
 * Revisions to deal with problems arrising from the neighbors array.
 * Still not completely solved.
 * 
 * Revision 3.4  86/07/11  17:58:07  sob
 * Fixed the alternate case conversion to work right.
 * Stan
 * 
 * Revision 3.3  86/07/10  15:29:21  sob
 * Added modifications that allow getpath to work with hosts that have
 * names that are all uppercase or all lowercase.
 * Stan
 * 
 * Revision 3.2  86/07/06  17:39:57  sob
 * Added changes provided to dynamically allocate enough space for
 * neighbors array. Thanks to rct for the help.
 * Stan
 * 
 * Revision 3.0  86/03/14  12:04:46  sob
 * Release of 3/15/86 --- 3rd Release
 * 
 * Revision 1.19  86/03/14  11:57:23  sob
 * updated copyright
 * 
 * Revision 1.18  86/03/11  11:28:58  sob
 * Added Copyright Notice
 * 
 * Revision 1.17  86/03/03  17:16:59  sob
 * Added fixes provided by desint!geoff.
 * Stan
 * 
 * Revision 1.16  86/02/24  12:45:36  sob
 * Bug fix in scanning the list from uuname.
 * Stan
 * 
 * Revision 1.15  86/02/23  23:01:53  sob
 * This version will use data from the uuname command as well as
 * data from the database
 * 
 * Revision 1.14  85/12/13  15:23:21  sob
 * Added patches from umd-cs!steve
 * 
 * Revision 1.13  85/12/10  20:36:58  sob
 * Added modifications suggested in gatech's version of uumail.
 * Now, the DBM version of the database needs to have a SENTINAL in it
 * to indicate that the DATABASE is not being updated. Also added similiar
 * indicators to the non-DBM version to compare modification times and act
 * accordingly. 
 * 
 * Revision 1.12  85/12/02  15:48:39  sob
 * Combined speed hacks and old way of reading database and
 * added compile flag SORTED. If database is SORTED and not DBM, use
 * -DSORTED in CFLAGS to make it fast. If database is not sorted
 * DO NOT use this flag.
 * 
 * Revision 1.11  85/11/24  15:03:41  sob
 * Added changes suggested by regina!mark
 * 
 * Revision 1.10  85/11/24  04:21:45  sob
 * Added efficiency hacks supplied by meccts!asby (Shane P. McCarron)
 * 
 * Revision 1.9  85/11/14  20:21:49  sob
 * Added #ifdef DEBUG to allow compilation without DEBUG
 * 
 * Revision 1.8  85/11/08  03:04:49  sob
 * release version
 * 
 * Revision 1.7  85/09/30  02:47:40  sob
 * Altered to use path filename from global variable.
 * 
 * Revision 1.6  85/08/03  00:48:57  UUCP
 * Cleaned up with lint.
 * Stan Barber
 * 
 * Revision 1.5  85/07/19  17:45:13  UUCP
 * Added \t as a valid seperation character for the database
 * in the non DBM case. This is what pathalias uses.
 * 
 * Revision 1.4  85/07/19  16:44:07  UUCP
 * revised to return proper things in accordance with sysexits
 * Stan
 * 
 * Revision 1.3  85/07/11  19:30:31  sob
 * added "uuconf.h" include file and deleted duplicated information
 * 
 * Revision 1.2  85/07/10  18:30:59  sob
 * updated to add DBM capabilities
 * Stan Barber, Baylor College of Medicine
 * 
 * Revision 1.1  85/07/10  18:03:28  sob
 * Initial revision
 * 
 */

#include	"uuconf.h"
#ifdef UUDBM
#include <dbm.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif

static char rcsid[] = "$Header: getpath.c,v 4.0 86/11/17 16:02:15 sob Exp $";
extern char * index();

extern FILE * fopen (), *popen();
FILE * in;
bool nghborflag,gotneighbors;
char **neighbors, *n_array;	/* rct */

int getpath (sysname, pathname, pathfile)
char   *sysname, *pathname,*pathfile;
{
    int indx,x;
    char ACsysname[NAMESIZ];			/* alternate case sysname */
#ifdef UUDBM
    datum lhs,rhs;
#else
	struct stat st;
        time_t modtime;
	char name[NAMESIZ];
#ifdef SORTED
	int scomp();

	long lo,hi;
	long cur;
	long last;
	static char buf[256];
#else
    char    * p, * q, t;
#endif
#endif


/* build sysname in the alternate case to conform to methods in SMAIL */
	if (*(sysname) == '.' || isupper(*sysname)) /* a kludge */
		{
			for (x=0;x<strlen(sysname);x++)
				ACsysname[x] = tolower(*(sysname+x));
		}
	else
		{
			for (x=0;x<strlen(sysname);x++)
				ACsysname[x] = toupper(*(sysname+x));
		}

	if (x < NAMESIZ) ACsysname[x] = '\0';
/* end case switch */
#ifdef DEBUG
if (Debug>2) (void) fprintf
	(stderr,"getpath: Sysname = %s, Alternate = %s, Pathfile = %s\n",
						sysname,ACsysname,paths);
#endif
	if(nghborflag == TRUE)
	{
	if (gotneighbors != TRUE) getneighbors();
	indx = 0;
/* is it a neighbor? */
	while(neighbors[indx] != NULL && *(neighbors[indx]) != '\0'){
		if(!strcmp(sysname, neighbors[indx]) 
			|| !strcmp(ACsysname,neighbors[indx])){
				strcpy(pathname, neighbors[indx]);
				strcat(pathname, "!%s");
				return(EX_OK);
			}
			indx++;
		}
	}
/* not a neighbor, let's look in the database */
#ifdef UUDBM
    for (indx = 0; indx < 5; indx++)
    {
	if ((x = dbminit (pathfile)) >= 0)
	    break;
	
#ifdef DEBUG
	if (Debug>2)
	    (void) fprintf (stderr, "Database unavailable.  Sleeping.\n");
#endif
	sleep (TIMEOUT);
    }

    if (x < 0)
	return(EX_OSFILE);

    lhs.dptr = SENTINEL;
    lhs.dsize = strlen (SENTINEL) + 1;
    for (indx = 0; indx < 5; indx++)
    {
	rhs = fetch (lhs);
	if (rhs.dsize > 0)
	    break;
	
#ifdef DEBUG
	if (Debug>2)
	    (void) fprintf (stderr, "Database incomplete.  Sleeping.\n");
#endif
	sleep (TIMEOUT);
    }
    if (rhs.dsize <= 0)
	return(EX_TEMPFAIL);

    	lhs.dptr = sysname;
	lhs.dsize = strlen(sysname)+1;
	rhs = fetch(lhs);
	if (rhs.dptr == NULL){		/* try other case */
		lhs.dptr=ACsysname;
		rhs = fetch(lhs);
		if (rhs.dptr == NULL) return(EX_NOHOST); /* no name found */
	}
	strcpy(pathname,rhs.dptr);
        return(EX_OK);			/* system name found */

#else
if (in == NULL) 
    {
	for (indx = 0; indx < 5; indx++)
	    {
		if ((in = fopen(pathfile, "r")) != NULL)
		    break;
	
#ifdef DEBUG
		if (Debug>2)
		    (void) fprintf (stderr, "Database unavailable.  Sleeping.\n");
#endif
		sleep (TIMEOUT);
	    }
    if (in == NULL)
	return(EX_OSFILE);
    }
    else
	rewind(in);
    indx = 0;
    strcpy(name,sysname);
restart:
	indx++;
	if (indx > 5) return(EX_TEMPFAIL);
	stat(pathfile, &st);
	modtime=st.st_mtime; /* original last modification time */

#ifdef SORTED
	lo = 0;
	hi = st.st_size;
	last = 0;
	cur = hi >> 1;

	while (cur > lo && cur < hi)
	{
		stat(pathfile,&st);
		if (st.st_mtime > modtime) goto restart;
		(void)fseek(in, cur, 0);
		if (fgets(buf, sizeof(buf), in)== NULL) return(EX_TEMPFAIL);
		cur = ftell(in);
		if (fgets(buf, sizeof(buf), in) == NULL) return(EX_TEMPFAIL);

#ifdef	DEBUG
	if (Debug > 4)
		(void) printf("Found site %s\n", buf);
#endif
		if (scomp(name, buf) < 0) hi = cur;
		else
		if (scomp(name, buf) > 0) lo = cur;
		else
		{
			buf[strlen(buf)-1] = '\0';
			strcpy(pathname, (char *)index(buf, '\t') + 1);
			return(EX_OK);
		}
		cur = lo + ((hi - lo)>>1);
		if (last == cur) 
		{
			(void)fseek(in, lo, 0);
			do
			{
				if (fgets(buf, sizeof(buf), in) == NULL)
						return(EX_TEMPFAIL);
				lo = ftell(in);
				if (scomp(name, buf) == 0 )
				{
					buf[strlen(buf)-1] = '\0';
					strcpy(pathname, (char *)index(buf, '\t') + 1);
					return(EX_OK);
				}
			} while (lo <= hi);
			break;
		} /* end if */
		last = cur;
	} /* end while */
	if (!strcmp(name,ACsysname)) return(EX_NOHOST);
	else {
		strcpy(name,ACsysname);
		goto restart;
	}
#else
    p = (char *) malloc(NAMESIZ);
    if (p == NULL) return(EX_OSERR);	 /* can't get space */
    q = p;
    for (;;)
    {
	p = q;
	while ((t = getc(in)) != EOF && (*p++ = t) != ' ' && t != '\t');
					 /* read the system name */
        stat(pathfile,&st);
        if (st.st_mtime > modtime) goto restart;		
					/* database update in progress */
	if( t == EOF ) return(EX_NOHOST);
	*--p = '\0';			/* set end of string */
	p = q;
#ifdef DEBUG
	if (Debug>4) (void) printf("Found %s\n",p);
#endif
	if (!strcmp (p,name) || !strcmp(p,ACsysname))break;
	while (((t = getc (in)) !=EOF && t != '\n'));	/* skip this path */
    }
    p = pathname;			/* save start loc of pathname */
    while ((*pathname++ = getc (in)) != '\n' && *(pathname-1) != EOF);
    *--pathname = '\0';
    pathname = p;
    return(EX_OK);			/* system name found */

#endif
#endif
}

#ifdef	SORTED
#define MAPTAB(c) ((c=='\t')?'\0':c)

int scomp(a,b)
char *a,*b;
{
int r;
	while (!(r=(MAPTAB(*a)-MAPTAB(*b))) && *a && *b && *a!='\t' && *b!='\t')
	{
	       a++; b++;
	}
	return(r);
}
#endif

getneighbors()
{
	FILE *ppntr;
	char * ptr;
	int x = 0;
	char n_neigh[16], *calloc();	/* rct */
	int nelem = 0;			/* rct */

	gotneighbors = TRUE;

	/*
	*  Let's get the number of neighbors we have.
	*
	*  Beginning of added code.	--rct
	*/

	if((ppntr = popen("uuname | wc -l", "r")) != NULL){
#ifdef DEBUG
		if(Debug > 2)
			(void)fprintf(stderr, "Starting uuname | wc -l\n");
#endif
	}
	else{
		(void)fprintf(stderr, "Error: popen\(\"uuname | wc -l\"\)\n");
		exit(1);
	}
	if((fgets(n_neigh, sizeof(n_neigh), ppntr)) != (char *)0){
		if((ptr = index(n_neigh, '\n')) != (char *)0)
			*ptr = '\0';
	}
	else{
		(void)fprintf(stderr, "Error: fgets\(n_neigh\)\n");
		exit(2);
	}
#ifdef DEBUG
	if (Debug > 2)
		(void)fprintf(stderr, "n_neigh = %s\n", n_neigh);
#endif
	(void)pclose(ppntr);

	/*
	*  Allocate storage for neighbors based on n_neigh.
	*  Assumption being made here is that no system has a name
	*  longer than 14 characters.  If this assumption ever turns
	*  out to be wrong, lots of other code will break before this
	*  does!	--rct
	*/

	nelem = atoi(n_neigh) + 2;

	if(((neighbors = (char **)calloc((unsigned)nelem,
		sizeof(char **))) == (char **)0) ||
		((n_array = calloc((unsigned)nelem, 15)) == (char *)0)){
		(void)fprintf(stderr, "Error: getneighbors\(\): calloc\(\)\n");
		exit(3);
	}

	/*
	*  Set up pointers.
	*/

	for(x = 0; x < nelem; x++)
		neighbors[x] = &n_array[x * 15];

	/*
	*  Now, let's read them in!
	*
	*  End of added code.	--rct
	*/

	if ((ppntr = popen("uuname", "r")) != NULL) {
#ifdef DEBUG
		if (Debug>2)
			(void)fprintf(stderr, "Starting uuname\n");
#endif
		x = 0;
		while((fgets(neighbors[x], 15, ppntr)) != NULL){
			if ((ptr = index(neighbors[x], '\n')) != NULL)
				*ptr = '\0';
#ifdef DEBUG
			if (Debug>4)
				(void) fprintf(stderr, "Neighbor # %d: %s\n",
					x + 1, neighbors[x]);
#endif
			x++;
		}	
		(void) pclose(ppntr);
	}
	strcpy(neighbors[x], Myname);
}



