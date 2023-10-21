/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"
# include <utmp.h>
# include <sys/stat.h>
# include <signal.h>

# define PATHSIZ        150
# define MAXDUPS        100
# define TIMEOUT        120

# define fd1 chand[0]
# define fd2 chand[1]

int     nulluser;
char    ply0[100],
       *ply1;

term2init ()
{
    char   *t1[PATHSIZ],
           *t2;
    char   *getuser ();
    char   *getlogin ();

    ply1 = getlogin ();
    if (ply1 == 0) {
        fprintf (stderr, "Don't know who you are.\n");
        exit (1);
    }

    for (;;) {
        printf ("Who would you like to play with ? ");
        gets (ply0);
        if (*ply0 != '\0')
            break;
    }
    iswiz[0] = iswizard (ply0);
    iswiz[1] = iswizard (ply1);

    if (strcmp (ply0, "-null") == 0)
                /* enter only if change_player command is to be
                  reserved for wizards only. */
    /*
     if(!iswiz[1])
     {
     fprintf(stderr, "You are not a wizard.\n") ;
     exit(1) ;
     } else
     */
        nulluser = 1;
    if (nulluser)
        (void) strcpy (t1, "/dev/null");
    else
        (void) sprintf (t1, "/dev/%s", getuser (ply0));
    t2 = "/dev/tty";

    if (strcmp (t1, t2) == 0) {
        fprintf (stderr, "You can't expect to play with yourself!\n");
        exit (1);
    }

    page (t1, ply0, ply1);
    printf ("Response....\n\r");
    do2term (t1, t2);
    geterm (t1);
}

geterm (t)
char   *t;
{
    char   *s,
            c,
            termbuf[100],
            pscmd[100];
    FILE * psfp, *popen ();

    if (strcmp (t, "/dev/null") == 0) {
        char *getenv () ;
        fillterm (getenv ("TERM"), &ttycs[0]);
        return;
    }
    if (!strcmp (t, "/dev/console"))
        s = "co";
    else                        /* /dev/ttyxx */
        if (strncmp (t, "/dev/tty", 8) || strlen (t) != 10) {
            fprintf (stderr, "unrecognized tty name %s\n", t);
            exit (1);
        }
        else
            s = &t[8];

    (void) sprintf (pscmd, "ps wwet%s", s);
    psfp = popen (pscmd, "r");
    if (psfp == NULL) {
        fprintf (stderr, "can't execute %s\n", pscmd);
        (void) kill (getpid (), 9);
    }
    if (strfind (psfp, "okgalaxy", 0)) {
        fprintf (stderr, "can't find okgalaxy in %s\n", pscmd);
        (void) kill (getpid (), 9);
    }
    if (strfind (psfp, " TERM=", 1)) {
        fprintf (stderr, "can't find TERM type\n");
        (void) kill (getpid (), 9);
    }
    s = termbuf;
    while ((c = getc (psfp)) != ' ' && c != '\n')
        if (c == EOF) {
            fprintf (stderr, "ps output format error\n");
            (void) kill (getpid (), 9);
        }
        else
            *s++ = c;

    *s = '\0';
    while (getc (psfp) != EOF);
    (void) fclose (psfp);
    fillterm (termbuf, &ttycs[0]);
}

strfind (Xfp, s, Xflag)
FILE * Xfp;
char   *s;
{
    char   *p,
           *e,
            c;

    p = s;
    for (;;) {
        if ((c = getc (Xfp)) == EOF || c == '\n' && Xflag)
            return - 1;
        if (c == *p) {
            if (*++p == '\0')
                return 0;
        }
        else
            if (p != s) {

                for (e = s; e < p; e++)
                    if (c == s[p - e] && !strncmp (e, s, p - e))
                        break;
                if (e < p || c == *s)
                    p = &s[p - e + 1];
                else
                    p = s;
            }
    }
}


struct utmp dups[MAXDUPS];
int     lastdup = 0;

char   *
        getuser (s)
char   *s;
{
    static int  utmpf = -1;
    static struct utmp  utmp;
    char    p[100];
    int     i;
    lastdup = 0;

    if (utmpf == -1)
        utmpf = open ("/etc/utmp", 0);
    else
        (void) lseek (utmpf, 0L, 0);
    if (utmpf == -1)
        return (0);

    while (read (utmpf, (char *) & utmp, sizeof utmp) == sizeof utmp)
        if (strncmp (utmp.ut_name, s, 8) == 0)
            dups[lastdup++] = utmp;

    if (lastdup == 1)
        return (dups[0].ut_line);

    if (lastdup == 0) {
        writes (2, "not logged in.\n\r");
        exit (1);
    }

    printf ("%s logged in more then once.\n", s);
    for (i = 0; i < lastdup; i++) {
        printf ("%s on %s ? ", s, dups[i].ut_line);
        gets (p);
        if (p[0] == 'y')
            return (dups[i].ut_line);
        if (p[0] == 'q')
            return (0);
    }

    writes (2, "no more ttys\n\r");
    exit (1);

    /* NOTREACHED */
}

int     xpipe[2];
int     chand[MAXCHAN];


do2term (t1, t2)
char   *t1,
       *t2;
{
    fd1 = open (t1, 2);
    fd2 = open (t2, 2);
    if (fd1 == -1)
        pout (t1);
    if (fd2 == -1)
        pout (t2);

    if (pipe (xpipe) == -1)
        pout ("pipe");

    (void) dochan (fd1);
    (void) dochan (fd2);
}

pout (s)
char   *s;
{
    perror (s);
    exit (1);
}

dochan (fd)
int     fd;
{
    static  lastchan = 0;
    char    s[50];

    (void) sprintf (s, "%d", lastchan++);
    switch (fork ()) {
        case -1:
            pout ("fork");
        default:                /* parent */
            return (lastchan - 1);

        case 0:
            if (DUP2 (fd, 0) == -1)
                pout ("dup2 1");

            if (DUP2 (xpipe[1], 1) == -1)
                pout ("dup2 2");

            execl (LOCAL, "local", s, 0);
            pout ("local");
    }
 /* NOTREACHED */
}

readc (x)
chan * x;
{
    return (read (xpipe[0], (char *) x, sizeof (*x)));
}

page (s, nm, iam)
char   *nm;
char   *s;
char   *iam;
{
    int     onalrm ();
    if (nulluser)
        return;

    if (!okterm (s)) {
        fprintf (stderr,
                "%s's terminal, %s, mode is bad for galaxy\n",
                nm, s);
        exit (1);
    }
    if (access (s, 2) == -1) {
        fprintf (stderr, "Can't write %s.\n", nm);
        exit (1);
    }

    if (VFORK () == 0) {
        execl (PAGER, "pager", s,
                iam, 0);
        exit (1);
    }

    writes (1, "Please wait.....\n");
    (void) alarm (TIMEOUT);

    signal (SIGALRM, onalrm);
    printf ("Sleeping on %s\n", s);
    while (okterm (s));

    signal (SIGALRM, SIG_IGN);
    return;
}

okterm (s)
char   *s;
{
    struct stat stbuf;

    (void) stat (s, &stbuf);
printf("%s.st_mode = %#o\n", s, stbuf.st_mode);
    if ((stbuf.st_mode & (S_IREAD >> 3)) || (stbuf.st_mode & (S_IREAD >> 6)))
        return (0);
    else
        return (1);
}

onalrm () {
    writes (1, "Timeout.\r\n");
    exit (1);
}
