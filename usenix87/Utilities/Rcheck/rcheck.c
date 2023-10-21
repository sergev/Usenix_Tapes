/*
 * CHECK for users logging in or out on the local and Remote machines.
 * Idea stolen from jr by bs
 * Idea stolen from bs by klemets@ttds.UUCP
 * It should be started with "rcheck &".
 * The program is written for 4.2 BSD UNIX. If you don't use 4.2 BSD
 * you must change a little in the makefile and then it will work,
 * I hope... (It will then ignore the local network.)
 */

#include "rcheck.h"
int users, fid;
struct utmp old[MAXUSERS], new[MAXUSERS];

main(argc, argv)
char *argv[];
{
	int x, h=0, p=0, m=0, t=0;
	for (x=1; x<argc; ++x)
		if (strcmp ("-h", argv[x]) == 0)
			m = 1;
		else if (strcmp ("-t", argv[x]) == 0)
			m = 2;
		else {
			if (m == 0) {
				if (p == MAXUSERS) {
					fprintf (stderr, "Too many users specified.\n");
					exit (1);
				}
				strcpy (allperson[p++], argv[x]);
			}
			else if (m == 1) {
				if (h == MAXHOSTS) {
					fprintf (stderr, "Too many hosts specified.\n");
					exit (1);
				}
				strcpy (allhost[h++], argv[x]);
			}
			else {
				if (t == MAXUSERS) {
					fprintf (stderr, "Too many ttys specified.\n");
					exit (1);
				}
				strcpy (alltty[t++], argv[x]);
			}
	}
#ifdef BSD
	chdir ("/usr/spool/rwho");
#endif BSD
	gethostname (hostname, sizeof hostname);
	if ((fid = open("/etc/utmp", 0)) == NULL) {
		fprintf (stderr, "Can't open /etc/utmp\n");
		exit (-1);
	}
	load(old);
#ifdef BSD
	rload (0);/* rold */
#endif BSD
	while (getppid() > 1) {
	    sleep(30);
	    load(new);
	    localscan ();
	    sleep (30);
	    load (new);
	    localscan ();
#ifdef BSD
	    rload (1);
	    rscan ();
#endif BSD
	}
}

localscan ()
{
	    for (users--; users>=0; users--)
		if (strcmp(old[users].ut_name,new[users].ut_name))
		    if (new[users].ut_name[0]=='\0') {
			printout (old[users].ut_name, old[users].ut_line, hostname, old[users].ut_host, 0);
			old[users].ut_name[0]='\0';
		    }
		    else {
			printout (new[users].ut_name, new[users].ut_line, hostname, new[users].ut_host, 1);
			strcpy(old[users].ut_name,new[users].ut_name);
			strcpy (old[users].ut_line, new[users].ut_line);
			strcpy (old[users].ut_host, new[users].ut_host);
		    }
}
printout (name, tty, host, remote, mode)
char *name, *tty, *host, *remote;
{
	char *givename(), rtime[10], cname[NAMELEN+1];
	long time (), tim;
	int x=0;
	strncpy (cname, name, NAMELEN);
	cname[NAMELEN] = '\0';
	if (strlen (allperson[0])) {
		while (x < MAXUSERS)
			if (strncmp (allperson[x], cname, NAMELEN) == 0)
				break;
			else ++x;
		if (x == MAXUSERS)
			return (0);
	}
	x = 0;
	if (strlen (allhost[0])) {
		while (x < MAXHOSTS)
			if (strcmp (allhost[x], host) == 0)
				break;
			else ++x;
		if (x == MAXHOSTS)
			return (0);
	}
	x = 0;
	if (strlen (alltty[0])) {
		while (x < MAXUSERS)
			if (strcmp (alltty[x], tty) == 0)
				break;
			else ++x;
		if (x == MAXUSERS)
			return (0);
	}
	tim = time (0);
	strncpy (rtime, ctime (&tim)+11, 5);
	rtime[5] = '\0';
	printf ("\r\n*** %s %s@%s%s logged %s %s%s%s ***\7\r\n", rtime, cname,
		host, givename (cname), (mode) ? "in on" : "out from", tty,
		(strlen (remote)) ? " via " : "\0", (strlen (remote)) ? remote : "\0");
	return (1);
}
	    
char *givename (s)
char *s;
{
	struct passwd *getpwname (), *psw;
	char *namefix (), *r;
	static char str[100];
	if (psw = getpwnam (s))
		if (strlen (r = namefix (psw->pw_gecos, s))) {
			strcpy (str, " (");
			strcat (str, r);
			strcat (str, ")");
			return (str);
		}
	return ("");
}
load(f)
struct utmp f[MAXUSERS];
{
	int i;
	users = 0;
	lseek (fid, 0, 0);
	while (read (fid, (char *) &f[users], sizeof (struct utmp)) > 0)
	    ++users;
}
char *namefix (s, name)
char s[], name[];
{   
    int x, i;
    char tmp[200];
    for (x=i=0; x<strlen (s); ++x, ++i) {
	if (s[x] == ',') {
	    tmp[i] = '\0';
	    return (tmp);
	}
	if (s[x] == '&') {
	    strcpy (&tmp[i], name);
	    if (tmp[i] < 'A' || tmp[i] > 'Z')
		tmp[i] -= ' ';
	    i += strlen (name)-1;
	}
	else tmp[i] = s[x];
    }
    tmp[i] = '\0';
    return (tmp);
}
