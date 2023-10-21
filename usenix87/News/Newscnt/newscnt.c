/* newscnt -- a program to enumerate unread net.news articles     *
 *                                                                *
 *   Glen A. Taylor (AT&T-IS)         May 1986                    *
 *                                                                *
 * Added the code inside the BSD and USE_GETOPT #ifdef's	  *
 *   Rich $alz (Mirror Systems)       June 1986                   *
 *                                                                */


static char sccsid[] = "@(#)newscnt.c	1.2";

#define BSD
#define DEBUG

#include <stdio.h>
#ifdef BSD
#include <strings.h>
char *strtok();
#else
#include <string.h>
#endif
#include <pwd.h>
#include <ctype.h>

#define FALSE	0
#define TRUE	1
#define SILENT  4
#define TERSE   2
#define ALL     1
#define ERROR   -1
#define NEWS    0
#define NONEWS  1
#define MAXLENG 512
#define MAXGRPS 128
#define ACTIVE  "/usr/lib/news/active"
#define NEWSRC  ".newsrc"
#ifdef BSD
#define USER_NAME "USER"
#else
#define USER_NAME	"LOGNAME"
#endif

struct grp {
    char *name;   /* news group name */
    long  lowest; /* lowest article number */
    long  highest; /* highest article number */
    int   done; /* to indicate this element has been processed */
    struct grp *ptr; /* ptr. to next element */
};

typedef struct grp *GRPTR;

char *namevec[MAXGRPS]; /* array of newsgroup name strings */

main(argc,argv)
int argc;
char **argv;
{

    char *alt_newsrc,  **newsgrps, *malloc();
    int silent, terse, all, flags, fflag, nflag, xflag, nr_newsgrps;
    FILE *fpnewsrc, *fpactive;
    int count = 0;
    GRPTR listhead = NULL;

    /* Parse arguments and establish working parameters */
#ifdef	USE_GETOPT
    {
	char c;
	extern char *optarg;
	extern int optind;

	nr_newsgrps = silent = terse = all = xflag = fflag = nflag = flags = 0;
	while ((c = getopt(argc, argv, "staxf:n:")) != EOF)
	    switch (c) {
		default:
Usage:
		    fprintf(stderr,
		       "usage: %s [-{sta}] [-x] [-f alt_.newsrc] -n newsgrp...",
		       argv[0]);
		    exit(1);
		case 's': silent = 1; break;
		case 't': terse = 1; break;
		case 'a': all = 1; break;
		case 'x': xflag = 1; break;
		case 'f': fflag = 1; alt_newsrc = optarg; break;
		case 'n': nflag = 1; nr_newsgrps += addgrp(optarg); break;
	    }
	if (silent + terse + all > 1)
	    goto Usage;
	for (argv += optind; *argv; argv++)
	    nr_newsgrps += addgrp(argv);
    }
#else	/* !USE_GETOPT */
    if (!scanargs(argc,argv, "% sta%- x%- f%-alt_.newsrc!s n%-newsgrp!*s",
	 &flags, &xflag, &fflag, &alt_newsrc, &nflag, &nr_newsgrps, &newsgrps))
	 exit(1);
    silent = flags & SILENT;
    terse  = flags & TERSE;
    all    = flags & ALL;
#endif	/* USE_GETOPT */

    /* Open the .newsrc file */
    if (fflag) { /* use the file name the user supplied */
	fpnewsrc = fopen(alt_newsrc,"r");
    }
    else { /* determine the user's .newsrc file and open */
	{
	    char *fname[MAXLENG];
	    char *uname, *getenv();
	    struct passwd *passptr, *getpwnam();

	    uname = getenv(USER_NAME); /* get user's login name */
	    passptr = getpwnam(uname); /* get user's home dir */
	    /* construct .newsrc path */
	    strcpy(fname, passptr -> pw_dir);
	    strcat(fname, "/");
	    strcat(fname, NEWSRC);

	    fpnewsrc = fopen(fname,"r");
	}
    }

    /* Derive array of newsgroup names to search for */
    if (!nflag) getgrps(fpnewsrc, &nr_newsgrps, &newsgrps);

#ifdef DEBUG
    { int i;
      printf("Number of newsgroups = %d\n", nr_newsgrps);
      for (i = 0; i < nr_newsgrps; i++) printf("%s\n", newsgrps[i]);
    }
#endif

    if (nr_newsgrps == 0) exit (ERROR); /* can't do anything! */

    /* Open the ACTIVE newsgroup file */
    if ((fpactive = fopen(ACTIVE, "r"))==NULL)
	exit(ERROR); /* cannot proceed w/o ACTIVE file */

    /* Go through the ACTIVE file and make linked list of all 
       newsgroups matching elements of the "newsgrps" array   */
    {
	char gname[MAXLENG];   /* tmp to hold active group name */
	long low, high;  /* tmps to hold active group low & high numbers */
	int i, j;
	char *sp, junk[10];
	GRPTR current, next;

	while (fscanf(fpactive,"%s %ld %ld %s\n", gname, &high, &low, junk)
	    != EOF) { /* read the entire ACTIVE file */

	    /* Compare gname to every name in newsgrps */
	    for (i=0; i < nr_newsgrps; i++) {
		sp = newsgrps[i];
		j = strlen(sp);
		/* check for a group to be explicitly skipped */
		if (*sp == '!') { 
		    sp++;
		    if (strncmp(gname,sp,j-1) == 0) break;
		}
		/* otherwise check for a positive match */
		else if (strncmp(gname,sp,j) == 0) {
		/* got a matching group so allocate new list element */
		    next = (struct grp *) malloc(sizeof (struct grp));

		    if (!listhead) {/* first element */
			listhead = next;
			current = next;
		    }
		    else {
			current -> ptr = next; /* link on this element */
			current = next;
		    }
		    /* fill in the list element structure */
		    j = strlen(gname) + 1; /* get length of group name */
		    current -> name = strcpy(malloc(j), gname);
		    current -> lowest = low;
		    current -> highest = high;
		    current -> done = FALSE;
		    current -> ptr = NULL;

		    /* terminate inner loop -- proceed to next group */
		    break;
		}
	    }
	}
    }

#ifdef DEBUG
    {
	GRPTR lptr;
	printf("\nList of relevant active newsgroups:\n");
	lptr = listhead;
	while (lptr != NULL) {
	    printf("%s %d %d\n",lptr->name,lptr->highest,lptr->lowest);
	    lptr = lptr->ptr;
	}
    }
#endif

    /* Work down the linked list and compare to .newsrc entries */
    rewind(fpnewsrc);
    {
	GRPTR next;
	char line[MAXLENG], *gname, *range;
	int ignore, cnt;

	while(fgets(line, MAXLENG, fpnewsrc) != NULL){
	    gname = line; /* name starts in first char position */
	    range = line; /* find the "range" list */
	    while (*range != ':' && *range != '!' && *range != '\0')
		range++; /* : and ! are the two legal name delimiters */
	    ignore = *range == '!' ? TRUE : FALSE; /* do we ignore? */
	    *range++ = '\0'; /* delimit gname from range */

	    /* Search the list for this newsgroup */
	    next = listhead;
	    while (next != NULL) {
		if (strcmp(gname,next -> name) == 0) {/* Found it */
		    if (!ignore) {
		        cnt = ckrange(next, range, xflag);
			count = count + cnt;
			if (all && cnt) printf("%s: %d articles\n",gname,cnt);
		    }
		    next -> done = TRUE;
		    break;
		}
		next = next -> ptr;
	    }
	}
	/* go through list one more time and get those that aren't "done" */
	next = listhead;
	while (next != NULL) {
	    if (! next -> done) {
		cnt = next -> highest - next -> lowest;
		if (cnt < 0) cnt = 0;
		count = count + cnt;
		/* print the nonzero entries if "all" were requested */
		if (all && cnt) printf("%s: %d articles\n",next->name,cnt);
	    }
	    next = next -> ptr;
	}
    }

    /* Print final total */
    if (silent) exit( count? NEWS : NONEWS);
    if (terse) printf("%d\n", count);
    else if (count) printf("%d news articles.\n",count);
    else printf("No news.\n");
    exit(count? NEWS : NONEWS);
}

#ifdef	USE_GETOPT
/* addgrp -- add a group list to the namevec global array.
	     returns count of groups added.			*/
addgrp(p)
char *p;
{
    static char SEPARATORS[] = " \t\n,";
    static int cnt;
    char *ptr;
    int i;

    ptr = strtok(p, SEPARATORS);
    for (i = 0; ptr = strtok(NULL, SEPARATORS); i++)
	namevec[cnt++] = strcpy(malloc(strlen(ptr) + 1), ptr);
    return(i);
}
#endif	/* USE_GETOPT */


/* getgrps -- reads the .newsrc file, finds the "option -n ... " news
	      group list, and copies news groups into an array of
	      strings.                                                */
getgrps(fp, num, names)
FILE *fp;
int *num;
char ***names;
{
    char line[MAXLENG], *ptr;
    int optflag = FALSE, nflag = FALSE;

    *names = namevec;
    *num = 0;
    while (fgets(line,MAXLENG,fp) != NULL ) {
	if (!optflag && strncmp(line,"option",6) == 0) { 
	    /* found an option line */
	    optflag = TRUE;
	    nflag = FALSE;
	    ptr = strtok(line," \t\n");
	    ptr = strtok(NULL," \t\n"); /* skip "option" */
	}
	else if (optflag && isspace(line[0])) {
	    /* found a continuation line */
	    ptr = strtok(line," \t\n");
	}
	else {
	    optflag = FALSE;
	    nflag = FALSE;
	}

	/* Parse the options line */
	if (optflag) {
	    while (!nflag && ptr != NULL) {
		if (strcmp(ptr,"-n") == 0) {
		    nflag = TRUE;
		    ptr = strtok(NULL, " \t\n"); /* consume "-n" */
		    break;
		}
		ptr = strtok(NULL, " \t\n"); /* get first news group */
	    }
	    while (nflag && ptr != NULL) {
		/* copy this news group into "newsgrps" */
		namevec[*num] = strcpy(malloc(strlen(ptr)+1), ptr);
		*num += 1;
		ptr = strtok(NULL, " \t\n");
	    }
	}
    }
}

/* ckrange -- compare a range specification against a high count / low
	      count and return the number of unread articles          */
int
ckrange(ptr, range, flag)
GRPTR ptr;
char *range;
int flag;
{
    long low, high, nxthi, nxtlo;
    int i, extent, count;
    char *bitmap, *nextseg(), *rp;

#ifdef DEBUG
    printf("<*> ckrange: \n\tname -- %s\n\thigh = %d\n\tlow = %d\n",
	ptr->name, ptr->highest, ptr->lowest);
    printf("\trange -- %s\n", range);
#endif

    /* get low and high values for the newsgroup */
    low = ptr -> lowest;
    high = ptr -> highest;
    if (low > high) return (0); /* this one is screwy! */


    if (flag) { /* ignore range info */
	count = high - low;
	if (count < 0 ) count = 0;
	return (count);
    }

    /* set up a bitmap to do the computation */
    extent = high - low + 1;
    if (extent <= 0) return (0);
    bitmap = malloc(extent);
    if (bitmap == NULL) exit(ERROR);
    for (i=0; i < extent; i++) bitmap[i] = '*'; /* init. bitmap */

    /* set bits (bytes, actually) for each read article */
    rp = range;
    while ((rp = nextseg(&nxthi,&nxtlo,rp)) != NULL) {
#ifdef DEBUG
    printf("<*> nexthi = %d, nextlo = %d\nnext seg = `%s'\n",
	nxthi, nxtlo, rp);
#endif

	/* check that the ones read intersect the ones avail */
	if (nxthi < low || nxtlo > high) continue;

	/* adjust the range markers to the intersection */
	nxthi = (high > nxthi) ? nxthi : high;
	nxtlo = (low < nxtlo) ? nxtlo : low;

	/* mark the articles that were read */
	if (nxtlo == nxthi) bitmap[nxtlo-low] = ' ';
	else for (i=nxtlo-low; i <= nxthi-low; i++) bitmap[i] = ' ';
    }

    /* count the set bits & go home */
    count = 0;
    for (i=0; i < extent; i++) if (bitmap[i] == '*') count++;
    free(bitmap);
    return (count);
}

char *
nextseg(high, low, str)
long *high, *low;
char *str;
{
    char *p1, *retval;
    long val = 0L;
    int hyphen = FALSE, more = TRUE;

    if (*str == '\0') return(NULL);
    p1 = str;
    while (more) {
	switch (*p1) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		    val = val * 10 + (*p1 - '0');
		    break;

	case '-':
		    *low = val;
		    val = 0L;
		    hyphen = TRUE;
		    break;

	case ' ':
	case ',':
	case '\n':
		    more = FALSE;
		    retval = ++p1;
		    *high = val;
		    if (!hyphen) *low = val;
		    break;

	case '\0':
		    more = FALSE;
		    retval = NULL;
		    *high = val;
		    if (!hyphen) *low = val;
		    return (retval);
	}
	p1++;
    }
    return (retval);
}
