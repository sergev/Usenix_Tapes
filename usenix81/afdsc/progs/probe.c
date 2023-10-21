#
/*

Name:
	probe

Function:
	Gather internal dynamic data.

Algorithm:
	Using a technique similar to PS, probe reads the
namelist for desired internal variables, and then
seeks and reads memory to locate the value.
Probe can work on a cycling basis to gather information
for later summary purposes.
Information will be in the form of a printout, or
a binary file.  Some options of probe allow it to
also keep information about wather the system
is configured for users or down.

Parameters:
	-c cycletime  Allow probe to gether information
		every cycletime seconds.

	-f	probe will not print the output, but put it in a binary
		file  /usr/adm/pr.data in a format found in
		probe FORM.

	-u	If another invocation of probe is running
		(to gather daily data) probe will signal
		the running probe that the system is up
		and exit.

	-d	If another invocation of probe is running
		(to gather daily data) probe will signal
		the running probe that the system is now
		unavailable for users, and exit.

	-k	If another invocation of probe is running,
		this option will cause probe to kill the other one
		off.  This is used when taking the system
		down via a shell file without doing a ps, and
		finding the probe process.

	-i	Cause probe to print the in-core inode table
		only if the -f option is not in effect.
		This is useful when debugging system problems
		with bad filesystems, and to get  a feel for
		what inodes are "locked" in core on a
		permanent basis for system efficiency.

Returns:
	Standard error flags.

Globals:

Calls:

Called by:
	operate, kickoff, drain, leton.

Files and Programs:
		/usr/adm/pr.data - probe output file in binary format.

		/usr/adm/pr.hold - probe holding file for yesterday's
			data.  File is copied by probe at midnight.

	Related summary program is mgt.sum, which takes probe data
(/usr/adm/pr.hold), shell accounting data (/usr/adm/sh2.hold),
and login acounting (/usr/adm/log.hold) and creates a daily summary file
containing daily accounts of the last year's data.
See mg.data FORM for contents.

Notes and Memorabilia:

Installation Instructions:
	compile without floating point, or special libraries.

History:
	Written by Charles Muir, AFDSC/SFA, in May 1980.
This program is part of a daily management reproting system
designed to show the availability, performance, and workload
on the system.  Data captured by probe can also be used to
size system tables for maximum performance without running
out of table space.


*/
#
#include	<error.h>
/*
 *
 *	probe
 *
 */

#define	nextarg		{++argv; --argc;}		/* function to move to next argument */

int	copyflag, headerflag, ncopies;
char	*header;

char *prog_id "%Z%%M% Release %R% Level %L% Branch %B% Sequence %S% %D% %T%\n";



#include <param.h>
#include <proc.h>
#include <ibmtty.h>
#include <user.h>
#include	<file.h>
#include	<text.h>
#include	<inode.h>
#include	<pr.rec.h>
#define	DEVICES	"/dev"
#define	MEMORY	"/dev/mem"
#define	CORE	"/usr/sys/core"
#define	PRACT	"/usr/adm/pr.act"
#define	PRHOLD	"/usr/adm/pr.hold"
#define	LOCK	"/usr/lock/probe"
#define BIN	"/bin"
#define	TMAX	10
#define	SIGUP	10
#define	SIGDN	11
#define	SIGKILL	9
#define	ARGLEN	32
#define	SIGHUP	1
#define	SIGINT	2
#define	SIGQUIT	3
#define	SIGILL	4
#define	SIGTRAP	5

/*	system.h in edited form	*/
/*
 * Random set of variables
 * used by more than one
 * routine.
 */
int	coremap[CMAPSIZ];	/* space for core allocation */
int	swapmap[SMAPSIZ];	/* space for swap allocation */
int	execnt;			/* number of processes in exec */
/*
 * The callout structure is for
 * a routine arranging
 * to be called by the clock interrupt
 * (clock.c) with a specified argument,
 * in a specified amount of time.
 * Used, for example, to time tab
 * delays on teletypes.
 */
struct	callo
{
	int	c_time;		/* incremental time */
	int	c_arg;		/* argument to routine */
	int	(*c_func)();	/* routine */
} callout[NCALL];
/*
 * Mount structure.
 * One allocated on every mount.
 * Used to find the super block.
 */
struct	mount
{
	int	m_dev;		/* device mounted */
	int	*m_bufp;	/* pointer to superblock */
	int	*m_inodp;	/* pointer to mounted on inode */
} mount[NMOUNT];
int	maxmem;			/* actual max memory per process */
int	nswap;			/* size of swap space */
int	thrash;			/* count of thrashes */

struct syment
{
    char    name[8];
    int     type;
    char   *value;
}               nl[10];

struct	map
{
	char	*m_size;
	char	*m_addr;
};

struct tty  tty;
struct user u;
int	corblk	64;
int	swpblk	512;
int     kflg;
int     otflg 1 ;
int     tvec[2];
int     cflg;
int     fflg;
int	iflg;
int	uflg;
int	kflg;
int	pid;
int	dflg;
int	toggle;
int	cleanup();
int	up();
int	down();
int	eod();
int	oldday;
int	filflg;
int     mem;
int	fd;
int	fdl;
int	l;
int     swap;
int     stbuf[257];
int     ndev;
char   *tstr;
int	*loctime;
char    astr[ARGLEN];
char    devc[65];
char   *symref "/unix";
char   *fstr;
int     devl[65];
int     devt[65];
char   *coref MEMORY ;
struct ibuf
{
    char    idevmin,
            idevmaj;
    int     inum;
    int     iflags;
    char    inl;
    char    iuid;
    char    igid;
    char    isize0;
    int     isize;
    int     iaddr[8];
    char   *ictime[2];
    char   *imtime[2];
    int     fill;
};



main(argc, argv)
int	argc;							/* count of command line arguments */
char	**argv;							/* vector to pointer list of command line arguments */
{

    struct proc *p;
    int     n,
            b;
    int     i,
            c,
            mtty;
    char   *ap;
    int     uid,
            puid;


	/* COMMAND LINE ARGUMENT GATHERING SECTION */
	nextarg;			/* skip over the arg with command name (arg 0)*/
	while (argc > 0 && **argv == '-')		/* while there are arguments, and */
							/* they start with a dash */
	{
		for(;;) {				/* until end of string is detected */
			switch (*++*argv)		/* interpret each flag letter */
			{
			default:
				exit (EFLAG);		/* exit for usage print */


		    case 'u':		/* write system up record, and exit */
			uflg++;
			break;

		    case 'k':
			kflg++;
			break;

		    case 'd':		/* write system down record, and exit */
			dflg++;
			break;

		    case 'f':		/* Use accounting file for output */
			filflg++;
			signal(SIGHUP, 1);
			signal(SIGINT, 1);
			signal(SIGQUIT, 1);
			    if (( fd = open (PRACT, 1)) < 0) {
				printf("Cannot open %s\n", PRACT);
				exit(EACCES);
			    }
			    seek (fd, 0, 2);
			break;

		    case 'i':		/* Print out inode table */
			iflg++;
			break;

		    case 'c':		/* cycle every rbuf.cytime interval */ 
			nextarg;
			cflg++;
			rbuf.cytime = atoi (*argv);
			goto ugh_a_goto;

			case '\0':			/* end of string, go on to next argument */
				goto ugh_a_goto;
			}
		}
		ugh_a_goto:
		nextarg;
	}

    signal(SIGILL, eod);
    signal(SIGTRAP, cleanup);
    signal (SIGUP, up);
    signal (SIGDN, down);

    if (filflg) {
	if (!uflg && !dflg && !kflg) {
	    rbuf.flags =+ REBOOT;
	    if ((fdl = open(LOCK, 0)) > 0) {
		if (read(fdl, &pid, 2) > 0) {
		    kill (pid, SIGKILL);
		}
		close (fdl);
	    }
	    if ((fdl = creat(LOCK, 0600)) < 0) {
		printf("Cannot creat %s\n", LOCK);
		exit (EACCES);
	    }
	    pid = getpid();
	    write(fdl, &pid, 2);
	    close(fdl);
	    rbuf.procids = pid;
	    rbuf.flags =+ PIDATA;
	}
	else {
	    if (( fdl = open (LOCK, 0)) < 0) {
		printf("Cannot open %s\n", LOCK);
		exit (EACCES);
	    }
	    read (fdl, &pid, 2);
	    close (fdl);
	    if (uflg) {
		kill (pid, SIGUP);
		done();
	    }
	    if (dflg) {
		kill (pid, SIGDN);
		done();
	    }
	    if (kflg) {
		kill (pid, SIGKILL);
		done ();
	    }
	}
    }
    else {
    printf ("U  EB  CB  PR EP ET G S B I FC  CA  C FS   SA   S  FI IN TE T C       DATE\n");
    }
    if (chdir (DEVICES) < 0)
	{
	printf ("cannot change to /dev\n");
	exit (EACCES);
	}
    setup (&nl[0], "_proc");
    setup (&nl[1], "_swapdev");
    setup (&nl[2], "_coremap");
    setup (&nl[3], "_swapmap");
    setup (&nl[4], "_nswap");
    setup (&nl[5], "_file");
    setup (&nl[6], "_inode");
    setup (&nl[7], "_text");
    setup (&nl[8], "_callout");
    nlist (fflg ? fstr : symref, nl);
    if (nl[0].type == 0)
	{
	printf ("No namelist\n");
	return;
	}
    if (kflg)
	coref = CORE;
    if ((mem = open (coref, 0)) < 0)
	{
	printf ("No mem\n");
	exit (ENOENT);
	}
    seek (mem, nl[1].value, 0);
    read (mem, &nl[1].value, 2);
    getdev ();
    mtty = ttyn (0);
    uid = getuid () & 0377;
    if (chdir (BIN) < 0) {
	printf("Cannot change to /bin\n");
	exit (EACCES);
    }
    while (cflg || otflg) {
	prtot(&proc[0]);
	cotot(&coremap[0]);
	swtot(&swapmap[0]);
	fitot(&file[0]);
	intot(&inode[0]);
	tetot (&text[0]);
	catot(&callout[0]);
	if (filflg) {
	rbuf.flags =+ DATA;
	if (++toggle == TMAX) {
	    toggle = 0;
	    rbuf.procids = gnpid ();
	    rbuf.flags =+ PIDATA;
	}
	output();
	}
	else {
	time (tvec);
	tstr = ctime (tvec);
	printf(" %s", tstr);
	}
	if (cflg)
	    {
	    sleep (rbuf.cytime);
	    }
	otflg = 0;
	}
    done ();
}

/*

Name:
	getdev

Function:
	get tty file major minor from /dev directory.
	also gets major minor of swap device.

Algorithm:
	open /dev and read directory, stat all the files
	(a real dog), looking for tty names or swap device.
	once swap device is found, only stat tty files.

Parameters:

Returns:

Globals:
	lots

Calls:

Called by:
	main

Files and Programs:
	/dev and below.

Notes and Memorabilia:

Installation Instructions:

History:
	originally part of the PS command.


*/
getdev ()
{
    register struct
    {
	int     dir_ino;
	char    dir_n[14];
    }              *p;
    register    i,
                c;
    int     f;
    char    dbuf[512];
    int     sbuf[20];

    f = open ("/dev");
    if (f < 0)
	{
	printf ("cannot open /dev\n");
	exit (EACCES);
	}
    swap = -1;
    c = 0;

loop: 
    i = read (f, dbuf, 512);
    if (i <= 0)
	{
	close (f);
	if (swap < 0)
	    {
	    printf ("no swap device\n");
	    exit (ENOENT);
	    }
	ndev = c;
	return;
	}
    while (i < 512)
	dbuf[i++] = 0;
    for (p = dbuf; p < dbuf + 512; p++)
	{
	if (p -> dir_ino == 0)
	    continue;
	if (p -> dir_n[0] == 't' &&
		p -> dir_n[1] == 't' &&
		p -> dir_n[2] == 'y' &&
		p -> dir_n[4] == 0 &&
		p -> dir_n[3] != 0)
	    {
	    if (stat (p -> dir_n, sbuf) < 0)
		continue;
	    devc[c] = p -> dir_n[3];
	    devl[c] = sbuf -> iaddr[0];
	    c++;
	    continue;
	    }
	if (swap >= 0)
	    continue;
	if (stat (p -> dir_n, sbuf) < 0)
	    continue;
	if ((sbuf -> iflags & 060000) != 060000)
	    continue;
	if (sbuf -> iaddr[0] == nl[1].value)
	    swap = open (p -> dir_n, 0);
	}
    goto loop;
}

/*

Name:
	setup

Function:
	copy one string to another

Algorithm:
	using pointers copy target of one pointer to target of another.

Parameters:
	p = output pointer, s = input pointer.

Returns:

Globals:

Calls:

Called by:
	main

Files and Programs:

Notes and Memorabilia:

Installation Instructions:
	originally part of PS.

History:


*/
setup (p, s)
char   *p,
       *s;
{
    while (*p++ = *s++);
}


/*

Name:
	done

Function:
	No error exit point.

Algorithm:
	simply exit with no errors.

Parameters:

Returns:

Globals:

Calls:
	exit()

Called by:
	lots of places.

Files and Programs:

Notes and Memorabilia:

Installation Instructions:

History:
	part of the "GREAT Command Re-write" technique
	incorporated by Charles Muir into PS in may 1979.


*/
done ()
{
    exit (ENOERR);
}

/*

Name:
	prtot

Function:
	gather process table statistics about running processes.

Algorithm:
	Seek to process table location in /dev/mem.
read process table into buffer, and locate all sorts of
information about what is running, swapped, and waiting.
Also use proc information about location of tty structure
to track down tty buffer headers for each tty and accumulate
outstanding tty character count (c_cc) to be used
to size kernel tables for clist sizes.

Parameters:
	pp = pointer to process table buffer in main.

Returns:
	prints data or puts it in binary output buffer for later
	write to a file.

Globals:
	many

Calls:
	seek, read.

Called by:
	main

Files and Programs:

Notes and Memorabilia:

Installation Instructions:
	Written by Charles Muir, AFDSC/SFA, may 1980

History:


*/
prtot(pp)
struct	proc	*pp;
{
    register	int	i;
    register	int	c;
	seek (mem, nl[0].value, 0);
	read (mem, pp, sizeof proc);
	rbuf.ucount = 0;
	rbuf.swproc = 0;
	rbuf.ccount = 0;
	for (c = 0; c < ndev; c++) devt[c] = 0;
	rbuf.pcount = 0;
	rbuf.scount = 0;
	rbuf.bcore = 0;
	rbuf.pcore = 0;
	rbuf.score = 0;
	rbuf.runproc = 0;
	rbuf.cpuproc = 0;
	for (i = 0; i < NPROC; i++)
	    {
	    if (proc[i].p_stat == 0)
		continue;
	    rbuf.pcount++;
	    if (proc[i].p_ttyp) {
		if (proc[i].p_ppid == 1) rbuf.ucount++;
		for (c = 0; c < ndev; c++)
		    if (devt[c] == proc[i].p_ttyp) {
			goto out;
		    }
		seek (mem, proc[i].p_ttyp, 0);
		read (mem, &tty, sizeof tty);
		for (c = 0; c < ndev; c++)
		    if (devl[c] == tty.t_dev) {
			devt[c] = proc[i].p_ttyp;
			rbuf.ccount =+ tty.t_rawq.c_cc;
			rbuf.ccount =+ tty.t_canq.c_cc;
			rbuf.ccount =+ tty.t_outq.c_cc;
		    }
	    out: ;
	    }
	    if (proc[i].p_ppid == 1 && proc[i].p_ttyp)
		rbuf.ucount++;
	    rbuf.scount =+ (((proc[i].p_size) + 7) >> 3);
	    if (proc[i].p_flag > 0) {
		rbuf.bcore =+ (((proc[i].p_size) + 7) >> 3);
		rbuf.pcore++;
		if (proc[i].p_flag > 19)
		    rbuf.cpuproc++;
	    }
	    if (proc[i].p_stat == 3) {
		rbuf.runproc++;
		if (proc[i].p_flag == 0) {
		    rbuf.score =+ (((proc[i].p_size) + 7) >> 3);
		    rbuf.swproc++;
		}
	    }
	}
    if (!filflg) {
	printf("%d %d %d %d %d %d %d %d %d %d"
		,rbuf.ucount
		,rbuf.scount
		,rbuf.bcore
		,rbuf.pcount
		,rbuf.pcore
		,rbuf.cpuproc
		,rbuf.runproc
		,rbuf.swproc
		,rbuf.score
		,rbuf.ccount
		);
	}
}
/*

Name:
	cotot

Function:
	access cormap, and free space info.

Algorithm:
	seek to and read coremap area into buffer.
	determine available free space, and
	number of map entries used.

Parameters:
	pointer to map buffer.

Returns:
	prints output, or puts it in a binary buffer to be
	written to a file later.

Globals:
	many

Calls:
	seek, read printf.

Called by:
	main

Files and Programs:

Notes and Memorabilia:

Installation Instructions:

History:
	Written by Charles Muir, May 1980.


*/
cotot(mp)
struct	map	*mp;
{
    register	struct	map	*bp;

    rbuf.corefr = 0;
    rbuf.coreu = 0;

    seek (mem, nl[2].value, 0);
    read (mem, mp, (CMAPSIZ * 2));

    for (bp = mp; bp->m_size != 0; bp++) {
	rbuf.corefr =+ (((bp->m_size) + 7) >>3);
	rbuf.coreu++;
    }

    if (!filflg) {
    printf(" %l %l %l"
	,rbuf.corefr
	,rbuf.maxcore
	,rbuf.coreu
	);
    }
}
/*

Name:
	swtot

Function:
	access swapmap, and gather info about free swap space
	and entries used.

Algorithm:
	seek to and read swapmap kernel area into buffer.
	determine amount of free swap space, and number
	of entries used.

Parameters:
	pointer to internal swapmap buffer.

Returns:
	prints or puts data into binary buffer for later write to file.

Globals:
	many

Calls:
	seek, read, printf

Called by:
	main.

Files and Programs:

Notes and Memorabilia:

Installation Instructions:

History:
	written by Charles Muir, may 1980.


*/
swtot (mp)
struct	map	*mp;
{
    register	struct	map	*bp;

    seek (mem, nl[3].value, 0);
    read (mem, mp, (SMAPSIZ * 2));

    seek (mem, nl[4].value, 0);
    read (mem, &nswap, 2);

    rbuf.maxswap = nswap;
    rbuf.swapu = 0;
    rbuf.swapfr = 0;

    for (bp = mp; bp->m_size != 0; bp++) {
	rbuf.swapfr =+ bp->m_size;
	rbuf.swapu++;
    }

    if (!filflg) {
    printf(" %l %l %l"
	,rbuf.swapfr
	,rbuf.maxswap
	,rbuf.swapu
	);
    }
}
/*

Name:
	fitot

Function:
	gather information about used file table
entries.

Algorithm:
	seek to and read the file table into a buffer.
	access used entries and tabulate number.

Parameters:
	pointer to internal file table buffer.

Returns:

Globals:
	many

Calls:
	seek, read, printf

Called by:
	main

Files and Programs:

Notes and Memorabilia:

Installation Instructions:

History:
	written by charles Muir, may 1980.


*/
fitot (fp)
struct	file	*fp;
{
    register	struct	file	*bp;

    seek (mem, nl[5].value, 0);
    read (mem, fp, sizeof file);

    rbuf.fileu = 0;
    for (bp = fp; bp <= &file[NFILE]; bp++) {
	if (bp->f_count) rbuf.fileu++;
    }

    if (!filflg) {
    printf(" %l"
	,rbuf.fileu
	);
    }
}
/*

Name:
	intot

Function:
	Gather usage of the in-core inode table.

Algorithm:
	seek to and read the in-core inode table into 
	a buffer.
	gather info on table usage, and if -i command flag
	is present, print information about what inodes
	are currently in the table.

Parameters:
	pointer to internal inode table buffer.

Returns:
	prints info, or puts binary data in buffer for later write.

Globals:
	many

Calls:
	seek, read, printf.

Called by:
	main

Files and Programs:

Notes and Memorabilia:

Installation Instructions:

History:
	Written by Charles Muir, may 1980


*/
intot (ip)
struct	inode	*ip;
{
    register	struct	inode	*bp;

    seek (mem, nl[6].value, 0);
    read (mem, ip, sizeof inode);

    rbuf.inou = 0;
    if (iflg && !filflg)  {
	    printf("\nINODE TABLE\n");
	    printf("FLAG	COUNT	DEV	I_NUM	MODE	LINK	UID	GID\n");
    }
    for (bp = ip; bp <= &inode[NINODE]; bp++) {
	if (bp->i_count)  {
	    rbuf.inou++;
	if (iflg && !filflg) {
	    printf("%o	%d	%d	%d	%o	%d	%d	%d\n"
		,bp->i_flag
		,bp->i_count
		,bp->i_dev
		,bp->i_number
		,bp->i_mode
		,bp->i_nlink
		,bp->i_uid
		,bp->i_gid
		);
	}
    }
    }

    if (!filflg) {
    printf(" %l"
	,rbuf.inou
	);
    }
}
/*

Name:
	tetot

Function:
	gather information on text table.

Algorithm:
	seek to and read text table from kernel core into
	internal buffer.
	tally number of entries used, and maximum available.

Parameters:
	pointer to internal text buffer.

Returns:
	prints or puts data into binary buffer for later write to file.

Globals:
	many

Calls:
	seek, read, printf.

Called by:
	main

Files and Programs:

Notes and Memorabilia:

Installation Instructions:

History:
	Written By Charles Muir, may 1980.


*/
tetot (tp)
struct	text	*tp;
{
    register	struct	text	*bp;

    seek (mem, nl[7].value, 0);
    read (mem, tp, sizeof text);

    rbuf.textu = 0;
    rbuf.texts = 0;
    for (bp = tp; bp <= &text[NTEXT]; bp++) {
	if (bp->x_iptr) {
	    rbuf.textu++;
	    if (bp->x_ccount) rbuf.texts++;
	}
    }

    if (!filflg) {
    printf(" %l %l"
	,rbuf.textu
	,rbuf.texts
	);
    }
}
/*

Name:
	catot

Function:
	Gather info on callout table.

Algorithm:
	seek to and read in-core kernel callout table.
	accumulate table usage, and maximum size.

Parameters:
	pointer to internal callout buffer.

Returns:
	print, or data to a buffer for later write.

Globals:
	many

Calls:
	seek, read, printf.

Called by:
	main

Files and Programs:

Notes and Memorabilia:

Installation Instructions:

History:
	Written by Charles Muir, may 1980.


*/
catot (cp)
struct	callo	*cp;
{
    register	struct	callo	*bp;

    seek (mem, nl[8].value, 0);
    read (mem, cp, sizeof callout);

    rbuf.callu = 0;
    for (bp = cp; bp <= &callout[NCALL]; bp++) {
	if (bp->c_func) rbuf.callu++;
    }

    if (!filflg) {
    printf(" %l"
	,rbuf.callu
	);
    }
}

/*

Name:
	output

Function:
	output binary data buffer to file.

Algorithm:
	Write data accumulated by other "tot" routines
	to data file if the -f flag is in effect.
	if the day has changed since last write, call
	for end of day (eod) processing.

Parameters:
	none.

Returns:

Globals:
	many

Calls:
	loctime, time, write, eod, printf, done.

Called by:
	main

Files and Programs:
	/usr/adm/pr.data - binary data file.

Notes and Memorabilia:

Installation Instructions:

History:
	written by Charles Muir, may 1980.


*/
output()
{
	    time(rbuf.time);
	    loctime = localtime (rbuf.time);

	    if ((!otflg) && (oldday != loctime->day)) {
		eod();
	    }
	    oldday = loctime->day;

	    if ((l = write (fd, &rbuf, sizeof rbuf)) != sizeof rbuf) {
		printf ("Write error %s\n", PRACT);
		done ();
	    }
	    rbuf.flags = 0;
}
/*

Name:
	eod

Function:
	end of day processing.

Algorithm:
	Close data file, fork off a process to
	copy data file (/usr/adm/pr.data) to holding file
	(/usr/adm/pr.hold).  Create the data file again,
	emptying it, and return.

Parameters:

Returns:
	exits with EACCES if anything is wrong with the files.

Globals:
	many

Calls:
	close, fork, signal, execl, printf, exit, wait, creat.

Called by:
	output.

Files and Programs:
	/usr/adm/pr.data - data file.
	/uar/adm/pr.hold - data hold file for one day.

Notes and Memorabilia:

Installation Instructions:

History:
Written by Charles Muir, may 1980.


*/
eod ()
{
    register    savint,
                pid,
                rpid;
    int     retcode;
    int	onhup;
    int	onquit;
    onhup = 1;
    onquit = 1;
printf("Eod probe processing\n");
    close (fd);
    if ((pid = fork ()) == 0)
	{
	signal (SIGHUP, onhup);
	signal (SIGQIT, onquit);
	execl ("/bin/mv", "pr-move", PRACT, PRHOLD, 0);
	printf ("Cannot execute: pr-move\n");
	exit (EAGAIN);
	}
    savint = signal (SIGINT, 1);
    while ((rpid = wait (&retcode)) != pid && rpid != -1);
    signal (SIGINT, savint);
    if ((fd = creat(PRACT, 0600)) < 0) {
	printf("Cannot creat %s\n", PRACT);
	exit (EACCES);
    }
    rbuf.procids = pid;
    rbuf.flags =+ PIDATA;
    toggle = 0;
}
/*

Name:
	cleanup

Function:
	cleanup probe processing upon signal.

Algorithm:
	set flags such that probe will exit on next full cycle.

Parameters:

Returns:
	prints the message cleanup on tty.

Globals:
	cflg, otflg.

Calls:
	printf

Called by:
	signal

Files and Programs:

Notes and Memorabilia:

Installation Instructions:

History:
	written by Charles Muir, May 1980


*/
cleanup()
{
    cflg = 0;
    otflg = 0;
printf("Cleanup\n");
}
/*

Name:
	up

Function:
	record system is up.

Algorithm:
	catch signal from other probe process executed with
	a -u flag.  Set flag, and genrate output record
	indicating the time the system came up.

Parameters:

Returns:

Globals:
	rbuf.flags

Calls:
	signal, output.

Called by:
	signal

Files and Programs:

Notes and Memorabilia:

Installation Instructions:

History:
	Written by Charles Muir, may 1980.


*/
up()
{
		signal (SIGUP, up);
		rbuf.flags =+ SYSUP;
		output();
}
/*

Name:
	down

Function:
	Indicate system is down for some reason

Algorithm:
	Catch a signal from another probe process executed
	with the -d flag.  generate an output record indicating the
	system is down.

Parameters:

Returns:

Globals:
	rbuf.flags

Calls:
	output, signal.

Called by:
	signal

Files and Programs:

Notes and Memorabilia:

Installation Instructions:

History:
	Written by Charles Muir, May 1980.


*/
down()
{
		signal (SIGDN, down);
		rbuf.flags =+ SYSDN;
		output();
}
/*

Name:
	gnpid

Function:
	 	get new process id number.

Algorithm:
		fork a process just to see what the process id is.
	This value is used by a later summary program (mgt.sum).
	to compute  the number of daily process id's generated.

Parameters:

Returns:
	pid - current process id.

Globals:

Calls:

Called by:
	main

Files and Programs:

Notes and Memorabilia:

Installation Instructions:

History:
	Written by Charles Muir, May 1980.


*/
gnpid ()
{
    register    savint,
                pid,
                rpid;
    int     retcode;
    int	onhup;
    int	onquit;
    onhup = 1;
    onquit = 1;
    if ((pid = fork ()) == 0)
	{
	exit (0);
	}
    savint = signal (SIGINT, 1);
    while ((rpid = wait (&retcode)) != pid && rpid != -1);
    signal (SIGINT, savint);
    return (pid);
}
