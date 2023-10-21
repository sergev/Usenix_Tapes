#include "rcheck.h"
#include "/usr/src/etc/rwhod/rwhod.h"
/* The location of the above file may differ. */
int rsize = 1024 / sizeof (struct whoent);
struct whod rold[MAXHOSTS], rnew[MAXHOSTS];
rload (mode)
{
    	DIR *dirp;
    	struct direct *dp;
	int fd, x, nr=0;
	dirp = opendir (".");
	while (dp = readdir (dirp)) {
 		if (dp->d_ino == 0)
		    continue;
	    	if (strncmp (dp->d_name, "whod.", 5))
		    continue;
		if (strcmp (dp->d_name+5, hostname) == 0)
		    continue;
	    	fd = open (dp->d_name, 0);
		for (x=0;  x<rsize;  ++x) {
		    if (mode)
		    strcpy (rnew[nr].wd_we[x].we_utmp.out_line, "\0");
		    else strcpy (rold[nr].wd_we[x].we_utmp.out_line, "\0");
		}
		if (mode)
		    read (fd, &rnew[nr], sizeof (struct whod));
		else read (fd, &rold[nr], sizeof (struct whod));
		close (fd);
		++nr;
	}
	closedir (dirp);
}

rscan ()
{
	int x, u;
	for (x=0; x<MAXHOSTS; ++x)
		for (u = 0; strlen (rold[x].wd_we[u].we_utmp.out_line) && 
			u < rsize;  ++u)
			if (find (&rold[x].wd_we[u].we_utmp, &rnew[x]))
	printout(rold[x].wd_we[u].we_utmp.out_name, rold[x].wd_we[u].we_utmp.out_line,
	rold[x].wd_hostname, "\0", 0);
	for (x=0; x <MAXHOSTS; ++x)
		for (u = 0; strlen (rnew[x].wd_we[u].we_utmp.out_line) &&
			u < rsize;  ++u)
			if (find (&rnew[x].wd_we[u].we_utmp, &rold[x]))
	printout(rnew[x].wd_we[u].we_utmp.out_name, rnew[x].wd_we[u].we_utmp.out_line,
	rnew[x].wd_hostname, "\0", 1);
	for (x=0;  x<MAXHOSTS;  ++x)
	    for (u = 0;  u < rsize;  ++u) {
		strcpy (rold[x].wd_we[u].we_utmp.out_name, 
			rnew[x].wd_we[u].we_utmp.out_name);
		strcpy (rold[x].wd_we[u].we_utmp.out_line, 
			rnew[x].wd_we[u].we_utmp.out_line);
	    }
}
find (a, b)
struct outmp *a;
struct whod *b;
{
	int x;
	char cname[NAMELEN+1];
	strncpy (cname, a->out_name, NAMELEN);
	cname[NAMELEN] = '\0';
	for (x = 0; x < rsize; ++x)
		if (strcmp(a->out_line, b->wd_we[x].we_utmp.out_line) == 0
		&& strncmp(cname, b->wd_we[x].we_utmp.out_name, NAMELEN)==0)
			return (0);
	return (1);
}
