#

/*	Search, Greg Ordy, Case Western Reserve Univ.
 *	1978, Copyright (c)
/*	this program is the second half of the search driver
 *	it is called by "search", and is passed the
 *	players terminal letter. if the file "/tmp/slock"
 *	exists, it adds player information to it.
 *	if not, it creates it, and execls off the main game
*/

#define		LOCK		"/tmp/slock"
#define		NEWS		"/nmpc/usr/ordy/bin/snews"
#define		NEW		2
#define		OLD		3

int     lfd,
        fd,
        nfd,
        sfd,
        nsize,
        statflg OLD;

char    ttyname[]
{
    "/dev/tty "
};
char    nbuffer[82];

struct
{
    char    status,
            which,
            terminaltype,
            newline;
}
        ttysform;

struct
{
    char    name,
            type;
    int     pid;
    int     puid;
}
        pl;

main (argc, argv)
int     argc;
char   *argv[];
{
    char    c;

    if (argc != 2)
    {
	write (2, "incorrect invocation\n", 21);
	exit ();
    };
    if ((nfd = open (NEWS, 0)) > 0)
    {
	while ((nsize = read (nfd, nbuffer, 80)) != 0)
	    write (2, nbuffer, nsize);
    };
/* verify();	check for correct time of day */
    if ((lfd = open (LOCK, 2)) < 0)			/* start new game  */
    {
	if ((lfd = creat (LOCK, 0644)) < 0)
	{
	    write (2, "cannot create lock file\n", 25);
	    exit ();
	};
	statflg = NEW;
	write (2, "players: ", 9);
	ttyname[8] = *argv[1];
	sfd = open (ttyname, 2);
	read (sfd, &c, 1);
	while ((ttyname[8] = c) != '\n')
	{
	    write (sfd, &ttyname[8], 1);
	    read (sfd, &c, 1);
	    if ((fd = open (ttyname, 1)) < 0)
		continue;
	    write (fd, "\007SEARCH SOON!!!\n", 16);
	    close (fd);
	};
	write (2, "\n", 1);
	close (sfd);
    };

    pl.name = *argv[1];
    pl.pid = getpid ();
    pl.puid = getuid ();
    if ((fd = open ("/etc/ttys", 0)) < 0)
    {
	write (2, "cannot open /etc/ttys\n", 22);
	exit ();
    };
    while (read (fd, &ttysform, sizeof ttysform) == sizeof ttysform)
	if (ttysform.which == pl.name)
	    break;
    pl.type = ttysform.terminaltype - '0';
    close (fd);
    if (pl.type == 4)
	pl.type = 3;
    seek (lfd, 0, 2);
    write (lfd, &pl, sizeof pl);
    if (statflg == NEW)
    {
	if (fork () == 0)
	{
	    close (1);
	    close (0);
	    for (c = 3; c != 15; c++)
		close (c);
	    execl ("/nmpc/usr/ordy/bin/sgame", "sgame", 0);
	    write (2, "execl of sgame failed\n", 22);
	    kill (pl.pid, 9);
	    unlink (LOCK);
	    exit ();
	};
    };
    for (c = 0; c != 15; c++)
	close (c);
    pause ();
    exit ();

}

verify ()						/* check for valid time of day */
{
    register char  *pnt;
    int     tvec[2];

    time (tvec);
    pnt = ctime (tvec);
    if (*pnt == 'S')
	return;
    pnt =+ 11;
    if (*pnt == '2')
	return;
    if (*pnt == '0' && *(pnt + 1) < '9')
	return;
    if (*pnt == '1' && *(pnt + 1) > '6')
	return;
    if ((getuid () & 0377) == 15)
	return;
    if (*pnt != '1' || *(pnt + 1) != '2')
	sorry ();
    pnt =+ 3;
    if (*pnt == '1' && *(pnt + 1) < '5')
	sorry ();
    if (*pnt == '4' && *(pnt + 1) > '5')
	sorry ();
    if (*pnt == '5')
	sorry ();
    if (*pnt == '0')
	sorry ();
}

sorry ()
{

    write (2, "\n\07\07Search may only be played before 9, after 5\n   ", 47);
    write (2, "and between 12:15 and 12:45, at lunch.\n      ", 39);
    exit ();
}
