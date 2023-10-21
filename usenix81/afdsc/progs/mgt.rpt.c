/*
Name:
	mgt.rpt User Command

Function:
Print management report or high water mark report from database.

Algorithm:
	Gather command line flags.  Position properly in database.
	If management format, print headers, data for period, objectives,
	and summaries (averages).  If high water format, print headers,
	data for period and high water mark values.

Parameters:
	-d x	change the last reported date to x.  Default is today.
	-n x	change the number of reported days to x.  Default is 30.
	-h	change the rport format to high water mark.  Default is
			daily management report.

Returns:
	ENOERR	Normal exit
	EFLAG	Illegal command line flag
	EACCES	Cannot open database.

Files and Programs:
	MGDATA	database

Installation Instructions:
	COmpile for floating point.
	Put in /bin/mgt.rpt

History:
	May 80, Written by Charles Muir.


*/
#include	<error.h>
#include	<mg.rec.h>
#include	<pr.rec.h>
#include	<sysid.h>

/*	objectives	*/
#define	ORT	4.0
#define	O9RT	7.3
#define	OET	1.1
#define	OUSF	7.0
#define	OUP	90
#define	ODN	1
#define	OCR	0
#define	OCPU	38
#define	OCPUH	6.0

/* summary variables for end of report  */

/*  r t  */
float	trt;
float	wtrt;
float	minrt;
float	maxrt;
int	rtflg;

/*  rt90  */
float	trt9;
float	wtrt9;
float	minrt9;
float	maxrt9;
int	rt9flg;

/* elapsed  */
float	tet;
float	wtet;
float	minet;
float	maxet;
int	etflg;

/*  usf  */
float	tusf;
float	wtusf;
float	minusf;
float	maxusf;
int	usfflg;

/*  %up  */
float	tup;
float	wtup;
float	minup;
float	maxup;
int	upflg;

/*  down  */
float	tdn;
float	wtdn;
float	mindn;
float	maxdn;
int	dnflg;

/*  crash  */
float	tcr;
float	wtcr;
float	mincr;
float	maxcr;
int	crflg;

/*  cpu%  */
float	tcpu;
float	wtcpu;
float	mincpu;
float	maxcpu;
int	cpuflg;

/*  cpuh  */
float	tcpuh;
float	wtcpuh;
float	mincpuh;
float	maxcpuh;
int	cpuhflg;

/*  loghr  */
float	tlogh;
float	wtlogh;
float	minlogh;
float	maxlogh;
int	loghflg;

/*  pusr  */
float	tpusr;
float	wtpusr;
float	minpusr;
float	maxpusr;
int	pusrflg;

/*  max users  */
float	tmu;
float	wtmu;
float	minmu;
float	maxmu;
int	muflg;

/*  commands  */
float	tcmd;
float	wtcmd;
float	mincmd;
float	maxcmd;
int	cmdflg;

/*  procs  */
float	tproc;
float	wtproc;
float	minproc;
float	maxproc;

int	nrec;    /*  number of report records  */
int	nrpt;	/*   number of days to be reported  */
int	nwdrec;   /*  number of weekday report records  */

#define	RPTSIZE	30

struct	mrec	dbuf;
struct	systemid	idbuf;

int	fd;
int	curdate[2];
int	curday;
int	curyear;
int	rptyear;
int	rptday;
int	julian;
float	pup;
float	cphr;
int	oneday[2];
int	l;
int	i;

struct	lng
{
int	high;
int	low;
};

struct	st {
char	str[4];
};

struct	st	day[7]
	"SUN",
	"MON",
	"TUE",
	"WED",
	"THU",
	"FRI",
	"SAT";

struct	st	mon[12]
	"JAN",
	"FEB",
	"MAR",
	"APR",
	"MAY",
	"JUN",
	"JUL",
	"AUG",
	"SEP",
	"OCT",
	"NOV",
	"DEC";

int	*loctime;
char	datestr[11] "xxxxxx0000";
char	*dst;
char	*tstr;
int	ldummy[2];
int	rptdate[2];
int	today[2];
/*	high water mark entries  */

	int	reboot,
		unavail,
		crash,
		ttproc,
		tblk,
		core,
		proccor,
		swapout,
		rproc,
		swappr,
		coreleft,
		cormap,
		swapleft,
		swapmap,
		filemap,
		inmap,
		textmap,
		stkmap,
		callmap,
		ccnt;


#define	nextarg		{++argv; --argc;}		/* function to move to next argument */

int	highflag, dateflag, nflag;
char	*header;

char *prog_id "@(#)mgt.rpt.c Release 9 Level 1 Branch 0 Sequence 0 81/06/02 11:00:58\n";

main(argc, argv)
int	argc;							/* count of command line arguments */
char	**argv;							/* vector to pointer list of command line arguments */
{

	nrpt = RPTSIZE;

	/* COMMAND LINE ARGUMENT GATHERING SECTION */
	nextarg;			/* skip over the arg with command name (arg 0)*/
	while (argc > 0 && **argv == '-')		/* while there are arguments, and */
							/* they start with a dash */
	{
		for(;;) {				/* until end of string is detected */
			switch (*++*argv)		/* interpret each flag letter */
			{
			default:
				exit(EFLAG);

			case 'h':			/* put out high water marks */
				highflag++;
				break;
			case 'd':			/* taking in a report date */
				dateflag++;
				nextarg;
				if ((tstr = getdate(*argv)) == -1) {
					printf("Bad date format\n");
					exit(EFORM);
				}
				dst = datestr;
				for (i = 0; i < 6; i++)
					*dst++ = *tstr++;
				loctime = a2bdate(datestr);
				rptdate[0] = loctime->high;
				rptdate[1] = loctime->low;
				goto ugh_a_goto;
			case 'n':			/* taking in a report number of days */
				nextarg;
				nflag++;
				nrpt = atoi(*argv);
				if (nrpt > 366) {
				    printf("Too many days specified\n");
				    exit(EINVAL);
				}
				goto ugh_a_goto;
			case '\0':			/* end of string, go on to next argument */
				goto ugh_a_goto;
			}
		}
		ugh_a_goto:
		nextarg;
	}

	/* MAIN BODY OF PROGRAM */

	coreleft = 32767;
	swapleft = 32767;
    oneday[0] = 1;		/*  one days time in seconds = 86400 in two words  */
    oneday[1] = 20864;		/* high order is one remaining low order is decimal  */
    dopen();
    posit();
    if (!highflag) {
	pheader();
	prtdata();
	obj();
	sum();
    }
    else {
	highhead();
	highprt();
	highsum();
    }
    exit(ENOERR);

}
/*

Name:
	dopen

Function:
	Open database.

Algorithm:
	Try to open database.  Complain if unable.

Parameters:
	None.

Returns:
	None or EACCES

Files and Programs:
	MGDATA	database


*/
dopen()
{
    if ((fd = open(MGDATA, 0)) < 0) {
	printf("Cannot open %s\n", MGDATA);
	exit(EACCES);
    }
}
/*

Name:
	posit

Function:
	Set position in database.

Algorithm:
	Get report (ending) date.  Calculate beginning Julian date to
	use as index into database.  Seek to that position.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
posit()
{
  if (!dateflag) {
    time(today);
    lsub(rptdate, today, oneday);
  }

    curdate[0] = rptdate[0];
    curdate[1] = rptdate[1];

    loctime = localtime(rptdate);
    rptday = loctime->dyear;
    rptyear = loctime->year;

    for (i=0; i < nrpt; i++) {
	lsub(curdate, curdate, oneday);
    }
    loctime = localtime(curdate);
    julian = loctime->dyear;

    seek (fd, julian * sizeof dbuf, 0);
}
/*

Name:
	highprt

Function:
	Print high water marks for entire report period.

Algorithm:
	Loop through entire period, printing each day's marks (or dots,
	if system was down).  Collect total high water marks.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
highprt()
{
    while((l = read(fd, &dbuf, sizeof dbuf)) > 0) {

	loctime = localtime(curdate);
	curyear = loctime->year;
	curday = loctime->dyear;

	if (rptday == curday) {
	    printf("------------------------------------------------------------------");
	    printf("------------------------------------------------------------------\n");
	}

	printf("%2d %s %s", loctime->day, mon[loctime->month].str, day[loctime->dweek].str);

	localtime(dbuf.mtime);
	if (curyear != loctime->year) {
	    downday();
	}
	else {
 /* print high water marks for day */

	printf ("   %2d   %2d   %2d"
		,dbuf.reboot
		,dbuf.unavail
		,dbuf.crash
		);

	printf ("   %3d   %4d   %4d   %3d"
		,dbuf.tproc
		,dbuf.tblk
		,dbuf.core
		,dbuf.proccor
		);

	printf("   %4d   %3d   %3d   %4d"
		,dbuf.swapout
		,dbuf.rproc
		,dbuf.swappr
		,dbuf.coreleft
		);

	printf("   %2d   %4d   %2d   %3d"
		,dbuf.cormap
		,dbuf.swapleft
		,dbuf.swapmap
		,dbuf.filemap
		);

	printf("   %3d   %3d   %3d   %3d   %4d"
		,dbuf.inmap
		,dbuf.textmap
		,dbuf.stkmap
		,dbuf.callmap
		,dbuf.ccnt
		);
	printf ("\n");
	if (dbuf.reboot > reboot) reboot = dbuf.reboot;
	if (dbuf.unavail > unavail) unavail = dbuf.unavail;
	if (dbuf.crash > crash) crash = dbuf.crash;
	if (dbuf.tproc > ttproc) ttproc = dbuf.tproc;
	if (dbuf.tblk > tblk) tblk = dbuf.tblk;
	if (dbuf.core > core) core = dbuf.core;
	if (dbuf.proccor > proccor) proccor = dbuf.proccor;
	if (dbuf.swapout > swapout) swapout = dbuf.swapout;
	if (dbuf.rproc > rproc) rproc = dbuf.rproc;
	if (dbuf.swappr > swappr) swappr = dbuf.swappr;
	if (dbuf.coreleft < coreleft) coreleft = dbuf.coreleft;
	if (dbuf.cormap > cormap) cormap = dbuf.cormap;
	if (dbuf.swapleft < swapleft) swapleft = dbuf.swapleft;
	if (dbuf.swapmap > swapmap) swapmap = dbuf.swapmap;
	if (dbuf.filemap > filemap) filemap = dbuf.filemap;
	if (dbuf.inmap > inmap) inmap = dbuf.inmap;
	if (dbuf.textmap > textmap) textmap = dbuf.textmap;
	if (dbuf.stkmap > stkmap) stkmap = dbuf.stkmap;
	if (dbuf.callmap > callmap) callmap = dbuf.callmap;
	if (dbuf.ccnt > ccnt) ccnt = dbuf.ccnt;
      }
	    if (rptday == curday) {
		return;
	    }
      incpos();
    }
}
/*

Name:
	downday

Function:
	Print dots for a day when the system was down.

Algorithm:
	Put out dots in correct positions for the specified format.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
downday()
{
  if (highflag) {
	printf ("    .    .    .     .      .");
	printf ("      .     .      .     .     .      .");
	printf ("    .      .    .     .     .     .     .     .      .");
  }
  else {
    printf("       .       .       .       .");
    printf("                .    .  .");
    printf("               .      .");
    printf("       .       .        .       .        .");
  }
    printf("\n");
}
/*

Name:
	incpos

Function:
	Move on to next database record.

Algorithm:
	Find correct Julian date for the next day in report period.
	Seek to this day's record in the database.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
incpos()
{
    ladd(curdate, curdate, oneday);
    loctime = localtime(curdate);
    seek (fd, loctime->dyear * sizeof dbuf, 0);
}
/*

Name:
	highhead

Function:
	Print header for high water format report.

Algorithm:
	Get system name.  Print header lines.
	Note:  For systems without sysid system call, the header
	must be modified to print an appropriate system name.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
highhead()
{
    sysid(&idbuf);
    loctime = localtime(rptdate);
    printf("\n");
    printf("	%s  %d, 19%d", mon[loctime->month].str, loctime->day, loctime->year);
    printf("			DAILY HIGH WATER REPORT		");
    printf("	%s, %c%c%d\n", idbuf.si_title, idbuf.si_type[0], idbuf.si_type[1], idbuf.si_num);
    printf("------------------------------------------------------------------");
    printf("------------------------------------------------------------------\n");

	printf ("            RE-   UN  CRSH  TOT    TOT    IN   # IN   BLKS   RUN   SWAP  CORE  CORE  SWAP");
	printf ("  SWAP  FILE  INO   TEXT  STK   CALL  CHAR\n");

	printf ("            BOOT  AVL       PROC   CORE   CORE CORE   SWAP   PROC  PROC  LEFT   MAP  LEFT");
	printf ("   MAP   TBL  TBL    TBL  ENT    TBL   CNT\n");

	    printf("------------------------------------------------------------------");
	    printf("------------------------------------------------------------------\n");
}
/*

Name:
	pheader

Function:
	Print daily management report headers.

Algorithm:
	Get system name.  Print header lines.
	Note:  for systems without sysid system call, the header
	must be modified to print an appropriate system name.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
pheader()
{
    sysid(&idbuf);
    loctime = localtime(rptdate);
    printf("\n");
    printf("	%s  %d, 19%d", mon[loctime->month].str, loctime->day, loctime->year);
    printf("			DAILY MANAGEMENT REPORT		");
    printf("	%s, %c%c%d\n", idbuf.si_title, idbuf.si_type[0], idbuf.si_type[1], idbuf.si_num);
    printf("------------------------------------------------------------------");
    printf("------------------------------------------------------------------\n");
    printf("                       PERFORMANCE");
    printf("                      AVAILABILITY");
    printf("             UTILIZATION");
    printf("                WORKLOAD");
    printf("\n");
    printf("             ------------------------------");
    printf("             ------------");
    printf("            -------------");
    printf("  -------------------------------------");
    printf("\n");
    printf("             R.T. IN SECONDS  ELAPSED  USER");
    printf("                   NUMBER");
    printf("            %% CPU    CPU");
    printf("   LOGON    PRIME USERS        NUMBER");
    printf("\n");
    printf("             ---------------");
    printf("                                                                           -------------");
    printf("   -------------");
    printf("\n");
    printf("   DATE        AVG     90%%    AVG MIN  SUPT");
    printf("             %%UP   DN  CR");
    printf("            PRIME  HOURS");
    printf("   HOURS    AVG     MAX    CMDS    PROCS");
    printf("\n");
    printf("------------------------------------------------------------------");
    printf("------------------------------------------------------------------\n");
    printf("\n\n");
}
/*

Name:
	obj

Function:
	Print object (goal) values.

Algorithm:
	Print signal flag (asterisks) if objective is not met.
	Print objective values.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
obj()
{
	float	ort, o9rt, oet, ousf, oup, ocpu, ocpuh;
	int	odn, ocr;
	printf( rtflg ? "              *****" : "                   ");
	printf( rt9flg ? "   *****" : "        ");
	printf( etflg ? "    *****" : "         ");
	printf( usfflg ? "   ****" : "       ");
	printf( upflg ? "             ***" : "                ");
	printf( dnflg ? "   **" : "     ");
	printf( crflg ? "  *" : "   ");
	printf( cpuflg ? "             ***" : "                ");
	printf( cpuhflg ? "    ****" : "        ");
	printf("\n");

	ort = ORT;
	o9rt = O9RT;
	oet = OET;
	ousf = OUSF;
	oup = OUP;
	odn = ODN;
	ocr = OCR;
	ocpu = OCPU;
	ocpuh = OCPUH;
	printf("OBJECTIVE");
	printf("     %5.1f   %5.1f    %5.2f   %4.1f", ort, o9rt, oet, ousf);
	printf("             %3.0f   %2d  %1d",  oup, odn, ocr);
	printf("             %3.0f    %4.1f",  ocpu, ocpuh);
	printf("\n");
    printf("------------------------------------------------------------------");
    printf("------------------------------------------------------------------\n");
}
/*

Name:
	sum

Function:
	Print summary lines of management report.

Algorithm:
	Print appropraite totals.  Print average values for all
	seven days of week.  Print average values for only Monday
	through Friday.  Print minimum and maximum values.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
sum()
{
/*  totals  */

	printf("\n\n\n");
	printf("TOTAL(ALL)");
	printf("       .       .       .       .");
	printf("                .");
	printf("  %3.0f %2.0f", tdn, tcr);
	printf("               .      .");
	printf("  %7.1f", tlogh);
	printf("      .        .");
	printf(" %7.0f  %7.0f", tcmd, tproc);
	printf("\n");

/*  average  all  */

	printf(" AVG(ALL)     ");
	printf("%5.1f   %5.1f    %5.2f   %4.1f"
	,trt/nrec
	,trt9/nrec
	,tet/nrec
	,tusf/nrec
	);

	printf("             %3.0f   %2.0f  %1.0f"
	,tup/nrec
	,tdn/nrec
	,tcr/nrec
	);

	printf("             %3.0f    %4.1f"
	,tcpu/nrec
	,tcpuh/nrec
	);

	printf("   %5.1f    %4.1f      %2.0f   %5.0f    %5.0f"
	,tlogh/nrec
	,tpusr/nrec
	,tmu/nrec
	,tcmd/nrec
	,tproc/nrec
	);
	printf("\n");

/*  average weekday  */

	printf(" AVG(W/D)     ");
	printf("%5.1f   %5.1f    %5.2f   %4.1f"
	,wtrt/nwdrec
	,wtrt9/nwdrec
	,wtet/nwdrec
	,wtusf/nwdrec
	);

	printf("             %3.0f   %2.0f  %1.0f"
	,wtup/nwdrec
	,wtdn/nwdrec
	,wtcr/nwdrec
	);

	printf("             %3.0f    %4.1f"
	,wtcpu/nwdrec
	,wtcpuh/nwdrec
	);

	printf("   %5.1f    %4.1f      %2.0f   %5.0f    %5.0f"
	,wtlogh/nwdrec
	,wtpusr/nwdrec
	,wtmu/nwdrec
	,wtcmd/nwdrec
	,wtproc/nwdrec
	);
	printf("\n");

/* minimum */

	printf(" MINIMUM      ");
	printf("%5.1f   %5.1f    %5.2f   %4.1f"
	,minrt
	,minrt9
	,minet
	,minusf
	);

	printf("             %3.0f   %2.0f  %1.0f"
	,minup
	,mindn
	,mincr
	);

	printf("             %3.0f    %4.1f"
	,mincpu
	,mincpuh
	);

	printf("   %5.1f    %4.1f      %2.0f   %5.0f    %5.0f"
	,minlogh
	,minpusr
	,minmu
	,mincmd
	,minproc
	);
	printf("\n");

/* maximum */

	printf(" MAXIMUM      ");

	printf("%5.1f   %5.1f    %5.2f   %4.1f"
	,maxrt
	,maxrt9
	,maxet
	,maxusf
	);

	printf("             %3.0f   %2.0f  %1.0f"
	,maxup
	,maxdn
	,maxcr
	);

	printf("             %3.0f    %4.1f"
	,maxcpu
	,maxcpuh
	);

	printf("   %5.1f    %4.1f      %2.0f   %5.0f    %5.0f"
	,maxlogh
	,maxpusr
	,maxmu
	,maxcmd
	,maxproc
	);
	printf("\n\n");
    printf("------------------------------------------------------------------");
    printf("------------------------------------------------------------------\n");
}
/*

Name:
	prtdata

Function:
	Print individual day's values.

Algorithm:
	Print values.  Gather  totals (for later use in computing averages)
	and maximum and minimum values.  Gather weekday totals.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
prtdata()
{
    while((l = read(fd, &dbuf, sizeof dbuf)) > 0) {

	loctime = localtime(curdate);
	curyear = loctime->year;
	curday = loctime->dyear;


	if (rptday == curday) {
	    printf("------------------------------------------------------------------");
	    printf("------------------------------------------------------------------\n");
	}

	printf("%2d %s %s", loctime->day, mon[loctime->month].str, day[loctime->dweek].str);

	localtime(dbuf.mtime);
	if (curyear != loctime->year) {
	    downday();
	}
	else {
	/* print performance data	*/

	printf("    %5.1f   %5.1f    %5.2f  %5.1f", dbuf.pavrt, dbuf.prt90, dbuf.pavturn, dbuf.usf);

	/* print availability  */

	pup = 100 * (dbuf.ahours / 24);
	printf("             %3.0f   %2d  %1d", pup, dbuf.unavail, dbuf.crash);

	/* utilization  */
	cphr = ((dbuf.cpusr + dbuf.cpsys) / 60);

	printf("             %3.0f    %4.1f", dbuf.pcputil, cphr);

	/* workload */

	printf("   %5.1f    %4.1f      %2d   %5d    %5l", dbuf.loghr, dbuf.pavusr, dbuf.maxu, dbuf.ncmd, dbuf.nproc);

	printf("\n");

	nrec++;
	trt =+ dbuf.pavrt;
	if ((minrt > dbuf.pavrt) || (nrec == 1)) minrt = dbuf.pavrt;
	if (maxrt < dbuf.pavrt) maxrt = dbuf.pavrt;

	trt9 =+ dbuf.prt90;
	if ((minrt9 > dbuf.prt90) || (nrec ==1)) minrt9 = dbuf.prt90;
	if (maxrt9 < dbuf.prt90) maxrt9 = dbuf.prt90;

	tet =+ dbuf.pavturn;
	if ((minet > dbuf.pavturn) || (nrec == 1)) minet = dbuf.pavturn;
	if (maxet < dbuf.pavturn) maxet = dbuf.pavturn;

	tusf =+ dbuf.usf;
	if ((minusf > dbuf.usf ) || (nrec == 1))  minusf = dbuf.usf;
	if (maxusf < dbuf.usf) maxusf =  dbuf.usf;

	tup =+ pup;
	if ((minup > pup) || (nrec ==1)) minup = pup;
	if (maxup < pup) maxup = pup;

	tdn =+ dbuf.unavail;
	if ((mindn > dbuf.unavail) || (nrec ==1)) mindn > dbuf.unavail;
	if (maxdn < dbuf.unavail) maxdn = dbuf.unavail;

	tcr =+ dbuf.crash;
	if ((mincr > dbuf.crash) || (nrec == 1)) mincr = dbuf.crash;
	if (maxcr < dbuf.crash) maxcr = dbuf.crash;

	tcpu =+	dbuf.pcputil;
	if ((mincpu > dbuf.pcputil) || (nrec == 1)) mincpu = dbuf.pcputil;
	if (maxcpu < dbuf.pcputil) maxcpu = dbuf.pcputil;

	tcpuh =+ cphr;
	if ((mincpuh > cphr) || (nrec == 1)) mincpuh = cphr;
	if (maxcpuh < cphr) maxcpuh = cphr;

	tlogh =+ dbuf.loghr;
	if ((minlogh > dbuf.loghr) || (nrec == 1)) minlogh = dbuf.loghr;
	if (maxlogh < dbuf.loghr) maxlogh = dbuf.loghr;

	tpusr =+ dbuf.pavusr;
	if ((minpusr > dbuf.pavusr) || (nrec == 1)) minpusr = dbuf.pavusr;
	if (maxpusr < dbuf.pavusr) maxpusr = dbuf.pavusr;

	tmu =+ dbuf.maxu;
	if ((minmu > dbuf.maxu) || (nrec == 1)) minmu = dbuf.maxu;
	if (maxmu < dbuf.maxu) maxmu = dbuf.maxu;

	tcmd =+ dbuf.ncmd;
	if ((mincmd > dbuf.ncmd) || (nrec == 1)) mincmd = dbuf.ncmd;
	if (maxcmd < dbuf.ncmd) maxcmd = dbuf.ncmd;

	tproc =+ dbuf.nproc;
	if ((minproc > dbuf.nproc) || (nrec == 1)) minproc = dbuf.nproc;
	if (maxproc < dbuf.nproc) maxproc = dbuf.nproc;

	/*  weekday processing for summary  */

	if ((loctime->dweek >= 1) && (loctime->dweek <= 5)) {
	    wtrt =+ dbuf.pavrt;
	    wtrt9 =+ dbuf.prt90;
	    wtet =+ dbuf.pavturn;
	    wtusf =+ dbuf.usf;
	    wtup =+ pup;
	    wtdn =+ dbuf.unavail;
	    wtcr =+ dbuf.crash;
	    wtcpu =+ dbuf.pcputil;
	    wtcpuh =+ cphr;
	    wtlogh =+ dbuf.loghr;
	    wtpusr =+ dbuf.pavusr;
	    wtmu =+ dbuf.maxu;
	    wtcmd =+ dbuf.ncmd;
	    wtproc =+ dbuf.nproc;

	    nwdrec++;
	}
      }
	    if (rptday == curday) {
		if (rptyear != loctime->year)  return;
		if (ORT < dbuf.pavrt)  rtflg++;
		if (O9RT < dbuf.prt90) rt9flg++;
		if (OET < dbuf.pavturn) etflg++;
		if (OUSF > dbuf.usf) usfflg++;
		if (OUP > pup) upflg++;
		if (ODN < dbuf.unavail) dnflg++;
		if (OCR < dbuf.crash) crflg++;
		if (OCPU > dbuf.pcputil) cpuflg++;
		if (OCPUH > cphr) cpuhflg++;
		return;
	    }
      incpos();
    }
}
/*

Name:
	highsum

Function:
	Print summary lines of high water report.

Algorithm:
	Print high water values between delimiting lines.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
highsum()
{
	    printf("------------------------------------------------------------------");
	    printf("------------------------------------------------------------------\n");
	printf ("MAX / MIN ");
	printf ("   %2d   %2d   %2d"
		,reboot
		,unavail
		,crash
		);

	printf ("   %3d   %4d   %4d   %3d"
		,ttproc
		,tblk
		,core
		,proccor
		);

	printf("   %4d   %3d   %3d   %4d"
		,swapout
		,rproc
		,swappr
		,coreleft
		);

	printf("   %2d   %4d   %2d   %3d"
		,cormap
		,swapleft
		,swapmap
		,filemap
		);

	printf("   %3d   %3d   %3d   %3d   %4d"
		,inmap
		,textmap
		,stkmap
		,callmap
		,ccnt
		);
	printf ("\n");
	    printf("------------------------------------------------------------------");
	    printf("------------------------------------------------------------------\n");
}
