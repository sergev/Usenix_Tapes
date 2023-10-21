#include "hd.h"

/* This records each usage of Vsh in the file LOGFILE */
/* Each entry is in the form "User Time(minutes) Date" */

long time (), ent_time [1];
char *ctime ();

comein () 
{
    time (ent_time);
}

goout () 
{

	long lv_time [1];
	register char *namep;
	register char *cp;
	FILE *logfile;

	if (access(LOGFILE, 2))
		return;
	time (lv_time);
/*
	for (namep = cp = HOME; *cp; cp++)
		if (*cp == '/') namep = cp;
	namep++;
 */	namep = username;
	logfile = fopen (LOGFILE, "a");
	if (logfile == NULL)
		return;
	fprintf (logfile, "%8.8s %4d %10.10s\n",
		namep, (int)((*lv_time - *ent_time)/60L), ctime (ent_time));
	fclose (logfile);
}

leave () 
{			/* exit after resetting tty */
    int putch();

    tty_cooked ();
    ewin();
    window = LI;
    vwin();		/* reset tty scrolling */
    atxy(1, LI);
    if (TE)
	tputs(TE, 0, putch);
    goout ();
    exit (0);
}
