#

/*
 *	Search game driver.  Greg Ordy, 8/27/78
 *	Case Western Reserve University, 1978 Copyright (c)
 *	driver program for the intergallactic adventure game - search
 *	this program conditions a players terminal for assignment as
 *	a star ship console. information relating to the player is
 *	passed on to a setuid program that makes an entry in the
 *	file "/tmp/slock", where the main game can get at it
*/

int     status,
        wpid,
        tfildes,
        mode,
        arg[3];
char    term,
        ttyname[]
{
    "/dev/tty "
};

main ()
{
    if ((term = ttyn (0)) == 'x')
    {
	write (2, "Cannot play from a file!!!\007\n", 28);
	exit ();
    };
    ttyname[8] = term;
    chmod (ttyname, 0666);
    signal (2, 1);
    signal (3, 1);
    tfildes = open (ttyname, 2);
    gtty (tfildes, arg);
    mode = arg[2];
    arg[2] = 0162;
    stty (tfildes, arg);
    if ((wpid = fork ()) == 0)
    {
	execl ("/nmpc/usr/ordy/bin/sslave", "sslave", &ttyname[8], 0);
	write (2, "execl of sslave failed\n", 23);
	arg[2] = mode;
	chmod (ttyname, 0622);
	stty (tfildes, arg);
	exit ();
    };
    close (tfildes);
    close (0);
    while (wait (&status) != wpid);
    chmod (ttyname, 0622);
    tfildes = open (ttyname, 2);
    arg[2] = mode;
    stty (tfildes, arg);
    exit ();

}
