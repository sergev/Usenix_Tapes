#include "uutty.h"
/* 
** The following routine may have to be changed for different Unices,
** and probably will have to be totally rewritten for other systems.
**
** Fill in fields of the utmp structure *up, which may be our global
** utmp or may be another one returned by some library routine.
**
** Here that we define our own cute copy macro, which copies <siz> 
** bytes or to the first null, whichever comes first.  Note the test
** to verify that the src address is nonnull.
*/
#define CP(dst,src,siz) {p = dst; if (q = src) for (i=0; i<siz; i++) if (*q) *p++ = *q++; else *p++ = '\0';}

fillutmp(name,idp,line,type)
	char *name;	/* User's login id */
	char *idp;	/* 4-byte line id for utmp entry */
	char *line;	/* Device (port) name */
	int   type;
{	char *p, *q;
	int   i;

	D6("fillutmp(%lX,%lX,%lX,%d)",name,idp,line,type);
	if (name) D5("fillutmp: name=\"%s\"",name);
	if ( idp) D5("fillutmp:  idp=\"%s\"", idp);
	if (line) D5("fillutmp: line=\"%s\"",line);
	if (up == 0) {		/* No ttyslot!!! */
		D5("utmp:before setutent()");
		setutent();			/* Make pututline rescan the file */
		up = &utmp;
	}
#ifdef SYS5
	D5("up=%06lX",up);
	D5("utmp: Copy name \"%s\"",up->ut_user);
	CP(up->ut_user,name,8);			/* Fill in the user field with the login id */
	D5("utmp: Copy   id \"%s\"",up->ut_id);
	CP(up->ut_id,  idp, 8);			/* The "id" field is from the /etc/inittab entry */
	D5("utmp: Copy line \"%s\"",up->ut_line);
	CP(up->ut_line,line,12);		/* Copy the device name to the line field */
	up->ut_pid  = getpid();			/* This is probably already correct, but... */
	up->ut_type = type;
	up->ut_exit.e_termination = 0;		/* Default exit codes are all zero */
	up->ut_exit.e_exit = 0;
	D5("utmp: Note time...");
	time(&currtime);
	up->ut_time = currtime;			/* Tell the OS when (s)he logged in */
	if (debug >= 5) Hexdnm(up,sizeof(struct utmp),"utmp");
#endif
/*
** No return value; the caller trusts us to do it right.
*/
}
