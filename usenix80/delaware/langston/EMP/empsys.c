      These are miscellaneous routines from the archive "empsub.a"
      which may be system dependent.  If any of these won't work
      on your system then:
      1) make a routine of the same name that will work
      2) compile it (-c)
      3) replace the standard .o version in "empsub.a" with yours.  Make
	 sure the name of the .o file is as described in "Compile:".
      All these routines are Copyright (c) by P. Langston
      (for whatever that's worth).

-----------------------------------------------------------------------

/*
** Compile: cc -O -c -q myuid.c; ar r empsub.a myuid.o
*/

myruid()  /* get the current user's "real" uid */
{
	return(getuid() & 0377);
}

myeuid()  /* get the current user's "effective" uid */
{
	return(getuid() >> 8 & 0377);
}

-----------------------------------------------------------------------

/*
** Compile: cc -O -c -q getlogn.c; ar r empsub.a getlogn.o
*/

char *
getlogn(uid)                                      /* return logname of uid */
{
	register char *cp;
	static char buf[128] 0;
	static int lastuid 0;

	if (*buf != '\0' && lastuid == uid && uid != 0)
	    return(buf);
	if (getpw(0377 & uid, buf) != 0)
	    return("????");
	for (cp = buf; *cp && *cp != ':'; cp++);
	*cp = '\0';
	return(buf);
}

-----------------------------------------------------------------------

#include        "empdef.h"
/*
** Compile: cc -O -c -q erlog.c; ar r empsub.a erlog.o
*/

extern  int thisprog;

erlog(string)               /* mail "string" to the priveleged Empire user */
char *string;
{
	char buf[256];

	switch (fork()) {
	case -1:
	    prfmt("Unable to fork()!\n");
	    break;
	case 0:
	    close(0);           /* these two are because of "to" prog bugs */
	    close(1);
	    copy(string, buf);    /* to avoid collisions in the fmt buffer */
	    copy(fmt("-TPROG:%d, COM:%s\n%s\n", thisprog, combuf, buf), buf);
	    execl("/bin/to", "to" , privlog, "-SEmpire Error", buf, 0);
	    prfmt("Unable to execl the mail program!\n");
	    exit(2);
	default:
	    wait0();
	}
}

-----------------------------------------------------------------------

/*
** Compile: cc -O -c -q resetuid.c; ar r empsub.a resetuid.o
*/

resetuid()                            /* set "effective" uid to "real" uid */
{
	setuid(getuid() & 0377);
}

-----------------------------------------------------------------------

/*
** Compile: cc -O -c -q superet.c; ar r empsub.a superet.o
**
** C routine to do multi-level returns  psl 4/76
**	Usage:	superet(n, arg0, arg1)
**		if n=0 it's a nop
**		   n=1 it's a normal return
**		   n>1 it's SUPER!
**		arg0 is returned in r0
**		arg1 is returned in r1
*/
long
superet(n, args)
long	args;
{
	register int *ip, *myp;

	ip = &n;
	myp = &ip[-2];
	ip = *myp;
	while (--n > 0)
	    ip = *ip;
	*myp++ = *ip++;
	*myp = *ip;
	return(args);
}

-----------------------------------------------------------------------

/       WAIT0 -- Wait with 0 args -- Does not use "nargs"
/       (this one is not presently in empsub.a, but if you don't
/        have it you can add it)
/  Compile: as wait0.s; mv a.out wait0.o; ar r empsub.a wait0.o
/

/ C library -- wait0

/ Usage: pid = wait0();
/
/ pid == -1 iff error

.globl	_wait0, cerror

_wait0:
	mov	r5,-(sp)
	mov	sp,r5
	sys	wait
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
