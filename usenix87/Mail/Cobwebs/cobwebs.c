/*
 * cobwebs [-agq] [-d Days] [-s Size] [username ...]
 * 
 * -a alphabetic sort
 * -d sort and/or search by age     (default DEF_DAYS)
 * -g all mailboxes
 * -q quiet option -- no top banner
 * -s sort and/or search by size    (default DEF_SIZE)
 *
 *  Default: cobwebs -s
 *
 *  In case of multiple flags:
 *  	Sort priority   -- alpha > days > size
 *  	Search priority -- general > usernames > days > size
 *
 */

#include <sys/param.h>
#include <stdio.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <lastlog.h>
#include <pwd.h>

#define DEF_SIZE 20000			/* default mbox size limit */
#define DEF_DAYS 30			/* default mbox age limit */
#define MAXSTRING 30
#define MAXUSERNAMES 400
#define MAXPACKS 1000
#define TRUE 1
#define FALSE 0

#define NMAX 10

struct packet
{
    char    name[NMAX];
    off_t   size;
    time_t  filetime;
};

struct packet  *calloc ();
struct packet  *packs[MAXPACKS];
struct packet **packp;
struct stat stbuf;

char   *malloc ();
char    usernames[MAXUSERNAMES][NMAX];
int     num_unames = 0;			/* number of usernames		*/
int     now;
int     pack_count = 0;
int     mbox_size = DEF_SIZE;		/* default mbox constraint	*/
int     mbox_days = DEF_DAYS;		/* default mailbox age		*/
long int mbox_clock = 0;		/* convert above to seconds	*/
long int size_accum = 0;
long int age_diff_accum = 0;
int	mbox_total = 0;
int     rep_names = FALSE;		/* name search flag		*/
int     rep_alpha = FALSE;		/* alphabetic sort flag		*/
int     rep_gen = FALSE;		/* all mailboxes flag		*/
int     rep_size = FALSE;		/* sort and/or search by size	*/
int     rep_days = FALSE;		/* sort and/or search by age	*/
int	rep_quiet = FALSE;		/* print the top banner		*/

scmp (p, q)
char *p, *q;
{
    return (strcmp (p, q));
}

main (argc, argv)
int     argc;
char  *argv[];
{
    int i;
    (void) time (&now);
    while ((argc-- > 0) && (*(*++argv) == '-'))
	switch (*(*argv + 1))
	{
	case 's':
	    if (*(*argv + 2) != '\0')
		str_to_int (*argv, 2, &mbox_size);
	    else
	        if ((argc>1) && (**(argv+1) >= '0') && (**(argv+1) <= '9'))
		{
		    str_to_int (*++argv, 0, &mbox_size);
		    argc--;
		}
	    rep_size = TRUE;
	    break;
	case 'd':
	    if (*(*argv + 2) != '\0')
		str_to_int (*argv, 2, &mbox_days);
	    else
	        if ((argc>1) && (**(argv+1) >= '0') && (**(argv+1) <= '9'))
		{
		    str_to_int (*++argv, 0, &mbox_days);
		    argc--;
		}
	    mbox_clock = now - 86400 * mbox_days;
	    rep_days = TRUE;
	    break;
	default :
	    i = 1;
	    do
	        switch (*(*argv + i))
		{
		case 'a' : rep_alpha = TRUE; break;
		case 'g' : rep_gen = TRUE; break;
		case 'q' : rep_quiet = TRUE; break;
		default  : usage ();
		}
	    while (*(*argv + (++i)) != '\0');
	    break;
    	}
    if ((argc > 0) && !rep_gen)
    {
	while ((argc-- > 0) && (num_unames < MAXUSERNAMES))
	    (void) strcpy (usernames[num_unames++], *argv++);
	if (num_unames >= MAXUSERNAMES)
	    fatal ("Error: MAXUSERNAMES %d reached\n", MAXUSERNAMES);
	rep_names = TRUE;
    }
    if (!isatty (0) && !rep_gen)
    {
	while (getname (usernames[num_unames++], NMAX) > 0);
	if (num_unames >= MAXUSERNAMES)
	    fatal ("Error: MAXUSERNAMES %d reached\n", MAXUSERNAMES);
	rep_names = TRUE;
    }
    if (rep_names)
    {
	qsort (usernames, num_unames, sizeof usernames[0], scmp);
	if (!rep_size && !rep_days)
	    rep_alpha = TRUE;
    }
    else 
	if (rep_gen && !rep_days && !rep_size)
	    rep_alpha = TRUE;				/* -g default */
	else
	    if (rep_days)
		rep_size = FALSE;			/* no conflict */
	    else 
		rep_size = TRUE;			/* default */
    check_mailboxes ();
}

size_cmp (p, q)
struct packet **p, **q;
{
    if ((*p)->size == (*q)->size)
	return (0);
    if ((*p)->size > (*q)->size)
	return (-1);
    return (1);
}

alpha_cmp (p, q)
struct packet **p, **q;
{
    return (strcmp ((*p)->name, (*q)->name));
}

filetime_cmp (p, q)
struct packet **p, **q;
{
    if ((*p)->filetime == (*q)->filetime)
	return (0);
    if ((*p)->filetime > (*q)->filetime)
	return (1);
    return (-1);
}

check_mailboxes ()
{
    DIR * etc;
    struct direct  *dp;
    if (chdir ("/usr/spool/mail") < 0) {
	perror ("/usr/spool/mail"); exit (1);
    }
    if ((etc = opendir (".")) == NULL) {
	perror ("/usr/spool/mail"); exit (1);
    }
    packp = packs;
    while (dp = readdir (etc))
    {
	if (dp->d_ino == 0)
	    continue;
	if (stat (dp->d_name, &stbuf) < 0)
	    continue;
	if ((stbuf.st_mode & S_IFMT) == S_IFDIR)
	    continue;
	if (pack_count >= MAXPACKS)
	    fatal ("%d MAXPACKS reached -- report aborted\n", MAXPACKS);
	if (rep_names)
	{
	    if (binsearch (dp->d_name))
	    {
	    take_note (dp->d_name);
	    if (pack_count >= num_unames)
	    	goto sortpacks;			/* all names found */
	    }
	    continue;
	}
	mbox_total++;
	age_diff_accum += (stbuf.st_ctime - now);
	size_accum += stbuf.st_size;
	if ((rep_gen)
		|| (rep_days && (stbuf.st_ctime < mbox_clock))
		|| (rep_size && (stbuf.st_size > mbox_size)))
	    take_note (dp->d_name);
    }
sortpacks: 
    if (rep_alpha)
	qsort (packs, packp - packs, sizeof packs[0], alpha_cmp);
    else
	if (rep_days)
	    qsort (packs, packp - packs, sizeof packs[0], filetime_cmp);
	else
	    qsort (packs, packp - packs, sizeof packs[0], size_cmp);
    printout ();
}

take_note (name)
char   *name;
{
    pack_count++;
    *packp = calloc (1, sizeof (struct packet));
    (void) strcpy ((*packp)->name, name);
    (*packp)->size = stbuf.st_size;
    (*packp)->filetime = stbuf.st_ctime;
    packp++;
}

char   *
age_is (period)
long int    period;
{
    int     days, hours;
    char   *string, temp[MAXSTRING];
    string = malloc (16);
    *string = '\0';
    days = period / 86400;
    hours = (period % 86400) / 3600;
    switch (days)
    {
	case 0: 
	    (void) sprintf (string, "         ");
	    break;
	case 1: 
	    (void) sprintf (string, "%3d Day  ", days);
	    break;
	default: 
	    (void) sprintf (string, "%3d Days ", days);
    }
    switch (hours)
    {
	case 0: 
	    (void) sprintf (temp, "%2d Mins ",
				 (((period % 86400) % 3600) / 60));
	    break;
	case 1: 
	    (void) sprintf (temp, "%2d Hour ", hours);
	    break;
	default: 
	    (void) sprintf (temp, "%2d Hours", hours);
	    break;
    }
    (void) strcat (string, temp);
    return (string);
}

printout ()
{
    register struct packet **p;
    int     f;
    struct passwd  *pwd;
    struct lastlog  ll;
    int     i;
    char   *s, host[10];
    if ((f = open ("/usr/adm/lastlog", 0)) < 0) {
	perror ("/usr/adm/lastlog"); exit (1);
    }
    if (!rep_quiet)
    {
    	if (pack_count == 0)
    	{
	    printf ("No Report.\n");
	    return;
    	}
	for (i = 65; i-- > 0; putchar ('-'));
	putchar ('\n');
	(void) gethostname (host, 10);
	s = (char *) ctime (&now);
	printf ("%10s     %.12s     ", host, (s + 4));
	if (pack_count == 1)
	    printf ("%d Mailbox", pack_count);
	else
	    printf ("%d Mailboxes", pack_count);
	if (rep_names && (num_unames != pack_count))
	     printf (" Found For %d Names", num_unames);
	else
	    if (!rep_gen && !rep_names)
	    {
	    	if (rep_days)
		    printf (" Older Than %d Days", mbox_days);
	    	else
		    if (rep_size)
		    	printf (" Larger Than %d", mbox_size);
	    }
	if (!rep_names)
		printf ("\n\nTotal: %d   Average Size: %d   Average Age:%-16s",
		 mbox_total, (size_accum/mbox_total),
		 age_is ((age_diff_accum + now) / mbox_total));
	printf ("\n\n  NAME    MAILBOX SIZE");
	printf ("      MAILBOX AGE           LAST LOGIN\n");
	for (i = 65; i-- > 0; putchar ('-'));
	putchar ('\n');
    }
    for (p = packs; p < packp; p++)
    {
	printf ("%-8s", (*p)->name);
	printf ("     %6d     ", (*p)->size);
	printf ("%-16s", age_is (now - (*p)->filetime));
	if (((pwd = getpwnam ((*p)->name)) != NULL)
	    && (lseek (f, (long) pwd->pw_uid * sizeof (struct lastlog), 0) >= 0)
	   	&& (read (f, (char *) & ll, sizeof ll) == sizeof ll)
	    	&& (ll.ll_time <= 0))
	    printf ("        no login record\n");
	else
	{
	    s = (char *) ctime (&ll.ll_time);
	    printf ("        %.*s,%.*s\n", 24 - 18, (s + 4), 5, (s + 19));
	}
    }
    (void) close (f);
}

binsearch (target)
char   *target;
{
    int     hi, lo, mid, val;
    lo = 0;
    hi = num_unames - 1;
    while (TRUE)
	if (hi < lo)
	    return (FALSE);
	else
	    if ((val = strcmp (target, usernames[(mid = (lo + hi) / 2)])) == 0)
		return (TRUE);
	    else
		if (val > 0)
		    lo = mid + 1;
		else
		    hi = mid - 1;
}

str_to_int (string, offset, num)
char string[];
int    offset, *num;
{
    int     i, place, sum;
    for (place = 1, sum = 0, i = strlen (string); i-- > offset; place *= 10)
    {
	if ((string [i] < '0') || (string [i] > '9'))
	    usage ();
	else
	    sum = sum + (string [i] - '0') * place;
    }
    *num = sum;
}

getname (line, lengthlim)
char    line[];
int     lengthlim;
{
    int     i;
    char    ch;
    for (i=0; i < lengthlim-1 && ((ch = getchar ()) != EOF)
		&& (ch != '\n') && (ch != ' ') && (ch != '\t');)
	line[i++] = ch;
    line[i] = '\0';
    return (i);
}

usage ()
{
    fprintf (stderr, "Usage: cobwebs [-agq] [-d days] ");
    fprintf (stderr, "[-s size] [username ...]\n");
    exit (1);
}

fatal (format, argument)
char   *format, *argument;
{
    fprintf (stderr, format, argument);
    exit (1);
}
