#include	<error.h>
#include	<sysid.h>
#include	<cpuspeed.h>
#include	<acct2.h>
#include	<pr.rec.h>
#include	<mg.rec.h>
#define	ROLOVR	100		/* proc table rolover at max  */
#define	BASE	.44	/*  nominal minumum response time for 100 block per second system	*/
#define	SWSPEED	100	/*   swap speed in blocks per second   */
#define	PRLO	8	/*  prime time for summaries  */
#define	PRHI	17
#define	IOERROR	-1
#define	NSAMP	512	/*  max number of daily samples,  480 = 3 minute intervals  */

struct	mrec	obuf;
struct	logactbuf	lgbuf;
struct	sh2	tbuf2;
struct	systemid	idbuf;

struct	ttylg	{
int	lgtime[2];
}	lgent[128];

int	fcomp();
int	cputype;
int	cpspeed;
float	rtbase;
int	nrec;
int	fd;
int	trl2[2];
int	ptrl2[2];
int	users;
float	timprim;
float	pcptim;
float	plhr;
float	lhr;
float	avert;
int	*tstr;
int	intime[2];
int	outime[2];
int	loprime[2];
int	hiprime[2];
unsigned	int	pid, lpid;
float	rp;
float	sp;
float	sb;
int	l;
int	upflag;
float	hours;
int	nrt;
float	fnrt;
float	rt[NSAMP];
int	i;
int	ldummy[2];
float	cusr, csys, tn, ptn;
float	pcsys, pcusr;
int	*loctime;
int	tsys2[2];
int	tusr2[2];
int	psys2[2];
int	pusr2[2];
int	tprim2[2];
int	lgsec2[2];
int	lgs2[2];
int	plgsec2[2];
int	plgs2[2];
#
/*
 *
 *	mgt.sum
 *
 */

#define	nextarg		{++argv; --argc;}		/* function to move to next argument */

int	copyflag, headerflag, ncopies;
char	*header;

char *prog_id "@(#)mgt.sum.c Release 7 Level 1 Branch 0 Sequence 0 81/06/02 11:12:28\n";

main(argc, argv)
int	argc;							/* count of command line arguments */
char	**argv;							/* vector to pointer list of command line arguments */
{


	/* COMMAND LINE ARGUMENT GATHERING SECTION */
	nextarg;			/* skip over the arg with command name (arg 0)*/
	while (argc > 0 && **argv == '-')		/* while there are arguments, and */
							/* they start with a dash */
	{
		for(;;) {				/* until end of string is detected */
			switch (*++*argv)		/* interpret each flag letter */
			{
			case 'c':			/* example of setting flag only */
				copyflag++;
				break;
			case 'h':			/* example of taking in string */
				headerflag++;
				nextarg;
				header = *argv;
				goto ugh_a_goto;
			case 'm':			/* example of taking in a number */
				nextarg;
				ncopies = atoi(*argv);
				goto ugh_a_goto;
			case '\0':			/* end of string, go on to next argument */
				goto ugh_a_goto;
			}
		}
		ugh_a_goto:
		nextarg;
	}

	/* MAIN BODY OF PROGRAM */

    prtally();
    shtally();
    lgtally();
    if (obuf.usf > 1000) obuf.usf = 0;
    output();
    printf("MGT.SUM: Sumarization completed\n");

}
/*

Name:
	prtally

Function:
	sumarize probe data.

Algorithm:
	read file pr.hold accumulating statistics on
response time, availability, and internal UNIX table values.

Parameters:

Returns:

Globals:
	everything is global.

Calls:
	fcomp - to sort response times.

Called by:
	main

Files and Programs:
	/usr/adm/pr.hold - holding file for 1 day old probe data.

Notes and Memorabilia:

Installation Instructions:
	Compile floating point.

History:
	Written by Charles Muir, AFDSC, May 1980.


*/
prtally()
{

    if ((fd = open(PRHOLD, 0)) < 0) {
	printf("Cannot open %s\n", PRHOLD);
	exit (EACCES);
    }

    sysid (&idbuf);
    cputype = idbuf.si_cpu;
    if (cputype <= 40) cpspeed = BPS40;
    if (cputype >= 44) cpspeed = BPS44;
    if (cputype >= 70) cpspeed = BPS70;
    rtbase = (100 * BASE) / cpspeed;


    nrec = 0;
    while ((l = read(fd, &rbuf, sizeof rbuf)) > 0) {
	nrec++;
	if (rbuf.flags & REBOOT) {
	    if (upflag) obuf.crash++;
	    obuf.reboot++;
	    obuf.unavail++;
	}
	if (rbuf.flags & SYSUP) {
	    upflag++;
	}
	if (rbuf.flags & SYSDN) {
	    if (upflag) obuf.unavail++;
	    upflag = 0;
	}

	if (nrec == 1) {
	    obuf.mtime[0] = rbuf.time[0];
	    obuf.mtime[1] = rbuf.time[1];
	    obuf.coreleft = rbuf.corefr;
	    obuf.swapleft = rbuf.swapfr;
	    lpid = rbuf.procids;
	    if ((!(rbuf.flags & REBOOT))  &&  (!(rbuf.flags & SYSDN))) {
		upflag++;
	    }
	}

	if (rbuf.flags & PIDATA) {
	    if (rbuf.flags & REBOOT) lpid = rbuf.procids;
	    pid = rbuf.procids;
	    if (pid < lpid) {
		obuf.nproc =+ (32768 - lpid);
		obuf.nproc =+ (pid - ROLOVR);	/* proc table rollover from pid 32768  */
	    }
	    else {
		obuf.nproc =+ (pid - lpid);
	    }
	    lpid = pid;
	}

	if (! (rbuf.flags & DATA )) continue;

	if (upflag) {
	    hours = rbuf.cytime;
	    obuf.ahours =+ hours/3600;
	}

	if (obuf.pmaxu < rbuf.ucount) obuf.pmaxu = rbuf.ucount;
	if (obuf.tproc < rbuf.pcount) obuf.tproc = rbuf.pcount;
	if (obuf.tblk < rbuf.scount) obuf.tblk = rbuf.scount;
	if (obuf.ccnt < rbuf.ccount) obuf.ccnt = rbuf.ccount;
	if (obuf.core < rbuf.bcore) obuf.core = rbuf.bcore;
	if (obuf.proccor < rbuf.pcore) obuf.proccor = rbuf.pcore;
	if (obuf.swapout < rbuf.score) obuf.swapout = rbuf.score;
	if (obuf.rproc < rbuf.runproc) obuf.rproc = rbuf.runproc;
	if (obuf.swappr < rbuf.swproc) obuf.swappr = rbuf.swproc;
	if (obuf.coreleft > rbuf.corefr) obuf.coreleft = rbuf.corefr;
	if (obuf.cormap < rbuf.coreu) obuf.cormap = rbuf.coreu;
	if (obuf.swapleft > rbuf.swapfr) obuf.swapleft = rbuf.swapfr;
	if (obuf.swapmap < rbuf.swapu) obuf.swapmap = rbuf.swapu;
	if (obuf.filemap < rbuf.fileu) obuf.filemap = rbuf.fileu;
	if (obuf.inmap < rbuf.inou) obuf.inmap = rbuf.inou;
	if (obuf.textmap < rbuf.textu) obuf.textmap = rbuf.textu;
	if (obuf.stkmap < rbuf.texts) obuf.stkmap = rbuf.texts;
	if (obuf.callmap < rbuf.callu) obuf.callmap = rbuf.callu;

	loctime = localtime (rbuf.time);
	if ((loctime-> hour >= PRLO) && (loctime->hour < PRHI)) {

	    rp = rbuf.runproc;
	    sp = rbuf.swproc;
	    sb = rbuf.score;
	    rt[nrt] = rtbase * (rp - sp);
	    rt[nrt] =+ ((rtbase + (sb / (SWSPEED * sp))) * sp);
	    avert =+ rt[nrt];
	    if (++nrt >= NSAMP) {
		printf("Too many samples\n");
		exit (EFBIG);
	    }
	}
    }

   if (l == IOERROR) {
	printf("I/O error on %s\n", PRHOLD);
	exit (EIO);
    }

    qsort (&rt[0], nrt, 4, fcomp);
    obuf.prt90 = rt[nrt - (nrt/10)];
    fnrt = nrt;

    obuf.pavrt = avert / fnrt;
    close(fd);
}
/*

Name:
	fcomp

Function:
	floating point comparison

Algorithm:
	compare two floating point numbers, and return
depending on their values.

Parameters:
	pointers to the two values (p1, p2) to be compared.

Returns:
	1 if p1 > p2
	0 if p1 = p2
	-1 if p1 < p2

Globals:

Calls:

Called by:
	qsort

Files and Programs:

Notes and Memorabilia:

Installation Instructions:

History:
	Written by Charles Muir, May 1980.


*/
fcomp (p1, p2)
float *p1, *p2;
{
    if(*p1 > *p2) return 1;
    if(*p1 == *p2) return 0;
    return -1;
}
/*

Name:
	shtally

Function:
	Summarize shell command accounting records.

Algorithm:
	Read /usr/adm/sh2.hold file and sumarize system, user, and real
times.  Compute "user support factor", and cpu utilization.

Parameters:

Returns:

Globals:
	everything

Calls:

Called by:
	main

Files and Programs:
	sh2.hold - raw shell acoounting file 1 day old.

Notes and Memorabilia:

Installation Instructions:
	compile floating point

History:
	written by Charles Muir, May 1980.


*/
shtally()
{
float ut, csys, cusr, tn;
    if ((fd = open(SH2HOLD, 0)) < 0) {
	printf("Cannot open %s\n", SH2HOLD);
	exit (EACCES);
    }

    while ((l = read (fd, &tbuf2, sizeof tbuf2)) > 0) {
	obuf.ncmd++;
	if (tbuf2.shf2 == 1) obuf.nsf++;

	ladd(tsys2, tsys2, tbuf2.bsyst2);
	ladd(tusr2, tusr2, tbuf2.bcput2);
	ladd(trl2, trl2, tbuf2.realt2);

	loctime = localtime(tbuf2.date2);
	if ((loctime->hour >= PRLO) && (loctime->hour < PRHI)) {

	    ladd(psys2, psys2, tbuf2.bsyst2);
	    ladd(pusr2, pusr2, tbuf2.bcput2);
	    ladd(ptrl2, ptrl2, tbuf2.realt2);
	}

    }

    if (l == IOERROR) {
	printf ("I/O error on %s\n", SH2HOLD);
	exit (EIO);
    }

    csys = ldiv(tsys2[0], tsys2[1], 360);	/* .1 minute	*/
    cusr = ldiv(tusr2[0], tsys2[1], 360);
    pcsys = ldiv(psys2[0], psys2[1], 360);
    pcusr = ldiv(pusr2[0], pusr2[1], 360);
    tn = ldiv(trl2[0], trl2[1], 6);	/*  .1 minute  */
    ptn = ldiv (ptrl2[0], ptrl2[1], 6);
    obuf.cpsys = csys/( 10 );
    obuf.cpusr = cusr/( 10 );
    obuf.pcpsys = pcsys/( 10 );
    obuf.pcpusr = pcusr/( 10 );
    obuf.turn = tn / 10;
    obuf.avturn = obuf.turn / obuf.ncmd;
    obuf.pturn = ptn / 10;
    obuf.pavturn = obuf.pturn / obuf.ncmd;

    ladd(tprim2, psys2, pusr2);
    timprim = ldiv (tprim2[0], tprim2[1], 2160);	/*  .01 hour  */
    obuf.pcputil = timprim / (PRHI - PRLO);

    pcptim = (obuf.pcpsys + obuf.pcpusr);
	/*  Compute user support factor.  Combination real time to cpu time mix
	    of user commands, and ratio of login time to cpu time.
	    This number indicates how many users doing essentially
	    the same work as the users now using the system can be
	    supported without serious degradation of response
	    time beyond the tss "knee" value.
	*/
    obuf.usf = (obuf.pturn / (2 * pcptim));

    close(fd);
}
/*

Name:
	lgtally

Function:
	Sumarize login time.

Algorithm:
	Use login accounting data to accumulate user
records of number of users, average users, and login hours.

Parameters:

Returns:

Globals:
	everything.

Calls:
	i2bdate

Called by:
	main.

Files and Programs:
	/usr/adm/log.hold - raw login accounting data 1 day old.

Notes and Memorabilia:

Installation Instructions:
	compile floating point.

History:
	Written by Charles Muir, May 1980.


*/
lgtally ()
{

    if ((fd = open(LOGHOLD, 0)) < 0) {
	printf("Cannot open %s\n", LOGHOLD);
	exit (EACCES);
    }

    loctime = localtime (obuf.mtime);
		tstr = i2bdate(0, PRLO, loctime->day, loctime->month + 1, loctime->year);
		lcopy (tstr, loprime);
    loctime = localtime (obuf.mtime);
		tstr = i2bdate(0, PRHI, loctime->day, loctime->month + 1, loctime->year);
		lcopy (tstr, hiprime);
    ldummy[0] = 0;
    ldummy[1] = 0;
    while ((l = read(fd, &lgbuf, sizeof lgbuf)) > 0) {

	if (lgbuf.logname[0] != 0) {		/*  got a login name logging in  */
	    obuf.nlog++;
	    lgent[lgbuf.logtty].lgtime[0] = lgbuf.logtime[0];
	    lgent[lgbuf.logtty].lgtime[1] = lgbuf.logtime[1];
	    if (++users > obuf.maxu) obuf.maxu = users;
	}
	else {
	    if (lgent[lgbuf.logtty].lgtime[1] != 0) {
		lsub (lgs2, lgbuf.logtime, lgent[lgbuf.logtty].lgtime);
		ladd(lgsec2, lgsec2, lgs2);	/*  total login hours  */
		users--;

		lcopy(lgent[lgbuf.logtty].lgtime, intime);
		lcopy(ldummy, lgent[lgbuf.logtty].lgtime);
		lcopy(lgbuf.logtime, outime);
		if (lcomp(intime, hiprime) == 1) continue;	/* login after prime  */
		if (lcomp(outime, loprime) == -1) continue;	/* logout before prime  */

		if (lcomp(outime, hiprime) == 1) lcopy(hiprime, outime);
		if (lcomp(intime, loprime) == -1) lcopy(loprime, intime);

		lsub (plgs2, outime, intime);
		ladd (plgsec2, plgsec2, plgs2);	/*  prime login hours  */
	    }
	}
    }

    if (l == IOERROR) {
	printf("Cannot open %s\n", LOGHOLD);
	exit (EIO);
    }

    plhr = ldiv(plgsec2[0], plgsec2[1], 360);	/* .1 hour  */
    obuf.ploghr = plhr/10;

	/*  Complete computation of user support factor  */

    obuf.usf =+ ((60 * obuf.ploghr) / (2 * pcptim));
    obuf.pavusr = obuf.ploghr/ (PRHI - PRLO);

    lhr = ldiv (lgsec2[0], lgsec2[1], 360);	/* .1 hour	*/
    obuf.loghr = lhr/10;
    close(fd);
}
/*

Name:	i2bdate

Function:	
	Given pointers to integer minutes, hours, day, and year i2bdate will create a
double precision integer to represent the date vector understandable to UNIX.
In addition, a2bdate will check for any bad data, such as a day 32.

Algorithm:	
	Using integer arguments representing
the year, day, hour, month and minute and use these to increment the date vector
(the number of seconds, in GMT, since January 1, 1970).  Then i2bdate will compensate
for the local timezone and subtract an hour if necessary for daylight savings time.

Parameters:	
	min, hour, day, month, year integer pointers;

Returns:
	A pointer to the double precision time integer; or
	-1 for a bad input string.

Files and Programs:	
	None.

Installation Instructions:	
	compile using default libc.a library to pick up 
	date routines needed by 12bdate.

History:	
	Began as a modified copy of system subroutine a2bdate,
but modified by Charles Muir, May 1980 to take integer arguments.


*/

i2bdate (minute, hour, day, month, year)
int	minute, hour, day, month, year;
{
    static int  timbuf[2];
    register int    i;
    extern int *localtime ();
    extern int  timezone;
    extern int	dmsize[];
    int     nt[2];

    if (month < 1 || month > 12)
	return (-1);
    if (day < 1 || day > 31)
	return (-1);
    if (hour == 24)
    {
	hour = 0;
	day++;
    }
    if (hour < 0 || hour > 23)
	return (-1);
    if (minute < 0 || minute > 59)
	return (-1);
    timbuf[0] = 0;
    timbuf[1] = 0;
    year =+ 1900;
    for (i = 1970; i < year; i++)
	gdadd (dysize (i), timbuf);
							/* Leap year */
    if (dysize (year) == 366 && month >= 3)
	gdadd (1, timbuf);
    while (--month)
	gdadd (dmsize[month - 1], timbuf);
    gdadd ((day - 1), timbuf);
    gmdadd (24, hour, timbuf);
    gmdadd (60, minute, timbuf);
    gmdadd (60, 0, timbuf);
    dpadd (timbuf, timezone);
							/* Now fix up to local daylight time. */
    if (localtime (timbuf)[8])
	dpadd (timbuf, -1 * 60 * 60);
    return (timbuf);
}
/*

Name:
	output

Function:
	output summary record from obuf buffer.

Algorithm:
	open summary file, seek to eof, and write 128 byte record.

Parameters:
	only the obuf record buffer.

Returns:

Globals:
	everything

Calls:

Called by:
	main

Files and Programs:
	/usr/adm/mgt.data - cronological summary data.

Notes and Memorabilia:

Installation Instructions:

History:
	Written by Charles Muir, May 1980.


*/
output ()
{
    int	recno;
    if ((fd = open (MGDATA, 2)) < 0) {
	printf("Cannot open %s\n", MGDATA);
	exit(EACCES);
    }

    loctime = localtime (obuf.mtime);
    recno = loctime->dyear * sizeof obuf;

    seek (fd, recno, 0);

    if ((l = write(fd, &obuf, sizeof obuf)) != sizeof obuf) {
	printf("Write error %s\n", MGDATA);
	exit(EIO);
    }

}
/*

Name:
	lcopy

Function:
	Copy one long integer to another.

Algorithm:
	Copy sub0 and sub1 to new long integer.

Parameters:
	integer pointer to old and new value.

Returns:

Globals:

Calls:

Called by:
	lgtally

Files and Programs:

Notes and Memorabilia:

Installation Instructions:

History:
	Written by Charles Muir, May 1980.


*/
lcopy (p1, p2)
int	p1[2], p2[2];
{
    p2[0] = p1[0];
    p2[1] = p1[1];
}
/*

Name:
	lcomp

Function:
	compare two long integers

Algorithm:
	Compare two long integers, and return value depending
on their relative values.

Parameters:
	integer pointers to double precision l1, and l2.

Returns:
	1 if p1 > p2
	0 if p1 = p2
	-1 if p1 < p2

Globals:

Calls:

Called by:
	lgtally

Files and Programs:

Notes and Memorabilia:

Installation Instructions:

History:
	Written by Charles Muir, May 1980.


*/
lcomp (l1, l2)
int	l1[2], l2[2];
{
    unsigned	int	lol1, lol2;

    if (l1[0] > l2[0]) return 1;
    if (l1[0] < l2[0]) return -1;

    lol1 = l1[1];
    lol2 = l2[1];

    if (lol1 > lol2) return 1;
    if (lol1 < lol2) return -1;
    return 0;
}
