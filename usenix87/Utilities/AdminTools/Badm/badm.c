#ifndef lint
static char rcsid[] = "$Header: badm.c,v 2.0 85/10/31 12:57:16 rick Exp $";
#endif

/* 
 * Utility to do media checking/formating during timesharing
 *
 * $Log:	badm.c,v $
 * Revision 2.0  85/10/31  12:57:16  rick
 * * First publicly-released version *
 * 
 * Rick Ace
 * New York Institute of Technology
 * Computer Graphics Laboratory
 * Old Westbury, New York  11568
 *
 * {seismo,decvax}!philabs!nyit!rick
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/dkbad.h>
#include <vax/dkio.h>
#include <disktab.h>
#include <signal.h>
#include <stdio.h>
#include <sysexits.h>

/*
 * There are a few hp.c driver bugs that can be circumvented by
 * workaround code in badm.  These workarounds appear under the
 * HPBUG conditional.  The `normal' (i.e., driver functions
 * properly) case appears after the `#else'.
 *
 * Of course, the right thing is to fix the driver instead,
 * but who knows... the same bugs might be in 4.Xbsd forever.
 */
#define HPBUG	1	/* hp driver bugs are present */

#define MAXNUMBAD	126	/* max # of bad sectors on one disk */

/*
 * Function codes for drive-specific header-write routines
 */
#define WHDR_VALID	0	/* mark the header as valid */
#define WHDR_BAD	1	/* mark the header as a bad sector */

/*
 * Special values returned by Btsector() 
 */
#define BTEMPTY		((daddr_t) -1)	/* entry is empty */
#define BTINVALID	((daddr_t) -2)	/* entry contains an illegal c/t/s */

int	bflag,iflag;
char	*cmd;		/* command and options, from argv */
char	diskdevice[100] = "/dev/r"; /* disk device name, built from argv */
int	dfd;		/* file descriptor of disk device */
char	*patternbuf;	/* output buffer, contains patterns, 0 if none */
char	*readbackbuf;	/* input buffer, for readback from disk */
unsigned short short_ones = ~0; /* all 1 bits set */
int	wantout;	/* set by SIGINT to request graceful exit */
int	bsi;		/* T/F: bst has been initialized */
struct dkbad bst;	/* bad sector table */
int	whdr_unknown();

/*
 * Drive description table
 */
struct st {
	char	vname[12];	/* vendor's name for disk */
	int	sectsize;	/* # bytes per sector */
	daddr_t	nsect;		/* # sectors/track */
	daddr_t	ntrack;		/* # tracks/surfaces/heads */
	daddr_t	nspc;		/* # sectors/cylinder */
	daddr_t	ncyl;		/* # cylinders */
	int	(*wheader)();	/* routine to write sector header */
	daddr_t	bad0sn;		/* sector # of first bad sector table */
} st =
	{ "default",	  0,  0,  0,     0,   0, whdr_unknown };

int	whdr_rm(),whdr_rp();

struct st disktypes[] = {
	{ "eagle",	512, 48, 20, 48*20, 842, whdr_rm },
	{ "rm05",	512, 32, 19, 32*19, 823, whdr_rm },
	{ "rm03",	512, 32,  5, 32* 5, 823, whdr_rm },
	{ "rp06",	512, 22, 19, 22*19, 815, whdr_rp },
	{ "rp05",	512, 22, 19, 22*19, 411, whdr_rp },
	{ "rp04",	512, 22, 19, 22*19, 411, whdr_rp },
	{ "\0" }		/* LAST ENTRY SENTINEL */
};

/*
 * Bit patterns for media surface testing
 */
unsigned short testpattern[] = {
	0x3333, 0x71C7, 0xB6DB, 0xDB6D, 0xE38E, 0xC71C, 0x83E8, 0x0FF0,
	0xEC6D, 0x1C71, 0x38E3, 0xAAAA, 0x5555, 0xF00F, 0xA5A5, 0x6DEC
};

#define	NPATTERN	(sizeof testpattern / sizeof testpattern[0]) 

/*
 * Torture-seek routine tables
 *
 * Routines take ordinal iteration number, returning
 * a sector number to seek to
 */
daddr_t	tort0(),tort1(),tort2(),tort3();
daddr_t	(*tortvec[])() = {
	tort0, tort1, tort2, tort3
};

/*
 * Command/options table
 *
 * WHEN CHANGING THIS TABLE BE SURE TO CHANGE THE USAGE MESSAGE TOO!
 *
 * Any option characters appearing in co_opt must also appear as
 * cases of the `switch' statement in Main().
 */
int	cmda(),cmdrw(),cmdt();

struct cotab {
	char	co_cmd;		/* command name, 1 letter */
	char	co_opt[7];	/* legal options for the command */
	int	(*co_rtn)();	/* driver */
} cotab[] = {
	'w',	"bi",	cmdrw,
	'r',	"",	cmdrw,
	'a',	"",	cmda,
	't',	"",	cmdt,
	0				/* TERMINATOR */
};

long	atol(),lseek();
char	*ctime(),*index(),*malloc();
int	catchint();
daddr_t	btsector();

main(argc,argv)
	int argc;
	char **argv;
{
	register struct cotab *co;
	register char *p;

	if (signal(SIGINT,SIG_IGN) != SIG_IGN) 
		(void) signal(SIGINT,catchint);
	if (argc < 2) 
		usage();
	/*
	 * Identify the command
	 */
	cmd = argv[1];
	co = &cotab[0];
	co--;
	do if ((++co)->co_cmd == 0) usage();
	 while (cmd[0] != co->co_cmd);
	/*
	 * Inspect and validate the options to the main command
	 */
	for (p = cmd; *++p; ) {
		if (index(co->co_opt,*p) == 0) 
			usage();
		/*
		 * All flag characters specified in the co_opt strings
		 * must have `case' entries in this switch statement.
		 * Arriving at the `should not happen' default case indicates
		 * that at least one such `case' is missing (programmer error).
		 */
		switch (*p) {
		case 'b':	bflag++; break;
		case 'i':	iflag++; break;
		default:	quit(EX_SOFTWARE,"UNEXPECTED FLAG: %c\n",*p);
		}
	}
	/*
	 * Command/options scan is complete, invoke the driver for this command
	 */
	(*co->co_rtn)(argc-2,argv+2);
}

/*
 * Add a sector to the bad sector table and rewrite the header for the sector
 *
 * Returns T/F: sector was added to the bad sector table
 */
addbad(sn)
	daddr_t sn;		/* sector number */
{
	register struct bt_bad *e,*e1;
	register char **bsp;
	daddr_t esn;
	char *bsbuf[MAXNUMBAD];

	/*
	 * Abort now if I don't know how to write headers on this drive
	 */
	if (st.wheader == whdr_unknown) {
		whdr_unknown((daddr_t)0,0,WHDR_BAD); /* only print error msg */
		exit(EX_UNAVAILABLE);	/* should never get here, but... */
	}
	if (!bsi)	/* double-check for programming errors */
		quit(EX_SOFTWARE,"bst NOT SET UP!\n");
	printf("sn%d: ",sn);
	if (btsector(&bst.bt_bad[MAXNUMBAD - 1]) != BTEMPTY) {
		printf("BAD SECTOR TABLE IS FULL, CANNOT ADD\n");
		return 0;
	}
	/*
	 * Locate the position in the table where the new entry will go.
	 */
	e = &bst.bt_bad[-1];
	do {
		esn = btsector(++e);
		if (esn == BTINVALID) {
			/*
			 * This should never happen because the sector
			 * entries in bt_bad have already been validated
			 */
			printf("BAD SECTOR TABLE ENTRY %d CONTAINS AN ILLEGAL ADDRESS cyl=%x trksec=%x\n\
NO ACTION TAKEN\n", e - &bst.bt_bad[0], e->bt_cyl, e->bt_trksec);
			return 0;
		}
		if (sn == esn) {
			printf("ALREADY IN BAD SECTOR TABLE (ALTERNATE IS BAD!)\n");
			return 0;
		}
	} while (esn != BTEMPTY && sn > esn);
	/*
	 * Found the proper position for the new entry.  If there are any
	 * active entries with higher disk addresses, they must be shuffled
	 * down one slot to make room for the new entry, because DEC std 144
	 * dictates that the table must be in ascending order.
	 *
	 * Shuffling the entries also entails shuffling the contents of the
	 * alternate sectors, so the first task is to grab those contents.
	 */
	bsp = &bsbuf[0];
	e1 = e;
	do {
		esn = btsector(e1);
		if (esn == BTEMPTY) 
			break;
		if ((*bsp = malloc((unsigned)st.sectsize)) == 0) {
			printf("Cannot Malloc memory, no action taken\n");
			goto fre0;
		}
		if (bread(esn, *bsp++, st.sectsize) < 0) {
			printf("Error reading sector %d, no action taken\n",esn);
			*bsp = 0;
			goto fre0;
		}
	} while (++e1 < &bst.bt_bad[MAXNUMBAD]);
	*bsp = 0;			/* tie off buffer list */
	/*
	 * Shuffle the table entries to make room, then install the new entry
	 */
	for (e1 = &bst.bt_bad[MAXNUMBAD - 2]; e1 >= e; e1--) 
		e1[1] = e1[0];
	e->bt_cyl = sn / st.nspc;
	e->bt_trksec = sn % st.nspc / st.nsect << 8 | sn % st.nsect;
	writebst();			/* write bad sector table, 5 copies */
	/*
	 * Rewrite the header of the new bad sector to mark it bad
	 */
	(*st.wheader)(sn,1,WHDR_BAD);
	printf("added to bad sector table\n");
	/*
	 * Write the contents of the shuffled sectors back to their
	 * new alternates on disk
	 */
	e1 = e;
	for (bsp = &bsbuf[0]; *bsp; ) {
		/*
		 * For each core buffer there should be a sector number.
		 * Abort if I can't determine the sector number (this
		 * would imply a logic error in this subroutine).
		 */
		if (++e1 >= &bst.bt_bad[MAXNUMBAD] ||
		    (esn = btsector(e1)) == BTEMPTY ||
		    esn == BTINVALID) 
			quit(EX_SOFTWARE,"LOST SECTOR NUMBERS FOR RE-WRITE\n");
		if (bwrite(esn, *bsp, st.sectsize) < 0) 
			printf("FAILED to re-write sector %d\n",esn);
		free(*bsp++);			/* discard buffer */
	}
	return 1;				/* added successfully */

	/*
	 * Free Malloc buffers and error exit
	 */
fre0:	for (bsp = &bsbuf[0]; *bsp; ) 
		free(*bsp++);
	return 0;
}

/*
 * Read from disk
 *
 * Returns 0 OK, -1 error
 */
bread(sn,buf,size)
	daddr_t sn;		/* sector number */
	char *buf;		/* buffer */
	int size;		/* size (bytes) */
{
	(void) lseek(dfd, (long)sn * (long)st.sectsize, 0);
	if (read(dfd,buf,size) == size) 
		return 0;
	return -1;
}

/*
 * Given a bad sector table (bt_bad) entry, convert it to a sector number
 *
 * Returns sector number, or
 *	BTEMPTY if the entry is all 1s (i.e., unoccupied), or
 *	BTINVALID if entry's cylinder, track, or sector number is out of range
 */
daddr_t
btsector(e)
	register struct bt_bad *e;	/* entry to be converted */
{
	register unsigned int etrack,esector;

	if (e->bt_cyl == short_ones && e->bt_trksec == short_ones) 
		return BTEMPTY;
	etrack = e->bt_trksec >> 8;
	esector = e->bt_trksec & 0xFF;
	if (e->bt_cyl >= st.ncyl ||
	    etrack >= st.ntrack ||
	    esector >= st.nsect) 
		return BTINVALID;
	return e->bt_cyl * st.nspc + etrack * st.nsect + esector;
}

/*
 * Write to disk
 *
 * Returns 0 OK, -1 error
 */
bwrite(sn,buf,size)
	daddr_t sn;		/* sector number */
	char *buf;		/* buffer */
	int size;		/* size (bytes) */
{
	(void) lseek(dfd, (long)sn * (long)st.sectsize, 0);
	if (write(dfd,buf,size) == size) 
		return 0;
	return -1;
}

/*
 * SIGINT (Control-C) handler
 */
catchint()
{
	wantout++;
}

/*
 * Driver for `a' command
 */
cmda(ac,av)
	int ac;
	char **av;	/* main() argv sans first two entries */
{
	register int snx;
	char *sbuf;
	daddr_t sn;
	int brv;

	if (ac < 2) 
		usage();
	diskopen(av[0],2);
	get_devdata();		/* get disk geometry, etc. */
	readbst();		/* read bad sector table off pack */
	snx = 1;
	sbuf = malloc((unsigned)st.sectsize);
	do {
		sn = atol(av[snx]);
		if (sn < 0 || sn >= st.bad0sn - MAXNUMBAD) {
			/*
			 * Illegal to specify sectors that are
			 * 1.  beyond the end of the disk
			 * 2.  in the last track (where the bad sector file is) 
			 * 3.  in the alternate-sector region (the 126 sectors
			 *     preceding the last track on the disk) 
			 */
			printf("sn%d: out of range for `%s' disk\n",sn,st.vname);
			continue;
		}
		/*
		 * Read sector contents, mark sector bad, and then
		 * write the old contents out to the alternate.
		 * If the data is so far gone that it cannot be
		 * read, then put something clean (i.e., zeros) 
		 * in the alternate in lieu of the data.
		 */
		if ((brv = bread(sn,sbuf,st.sectsize)) < 0) {
			printf("sn%d: CANNOT READ DATA - substituting zeros\n",sn);
			bzero(sbuf,st.sectsize);	/* substitute zeros */
		}
		if (addbad(sn)) {	
			/* rewrite contents to alternate */
			printf("sn%d: copy ",sn);
			if (bwrite(sn,sbuf,st.sectsize) < 0 || brv < 0) 
				printf("failed\n");
			else
				printf("successful\n");
		}
	} while (++snx < ac);
	free(sbuf);
}

/*
 * Driver for `r' and `w' commands
 */
cmdrw(ac,av)
	int ac;
	char **av;	/* main() argv sans first two entries */
{
	register daddr_t sector;
	int pass,tracksize;
	time_t now;

	if (ac != (iflag ? 2 : 1)) 
		usage();
	(void) time(&now);
	printf(ctime(&now));
	diskopen(av[0], cmd[0]=='w'?2:0);
	get_devdata();		/* get disk geometry, etc. */
	printf("%s: %s  #cylinders=%d, #tracks=%d, #sectors=%d, bytes/sector=%d\n",
		diskdevice, st.vname, st.ncyl, st.ntrack, st.nsect, st.sectsize);
	if (iflag) {
		int serialno;

		printf("\nWriting valid headers on all sectors\n");
		initheaders();		/* writes in last track too */
		serialno = atoi(av[1]);
		printf("\nInitializing empty bad sector table, pack serial # %d\n",serialno);
		initbst(serialno);
		writebst();
	}
	else {
		if (bflag)		/* possibly updating the bad table? */
			readbst();	/* yes, must read it in then */
	}
	printf("\nBeginning surface test\n");
	tracksize = st.sectsize * st.nsect;	/* 1 track's worth of bytes */
	if (*cmd == 'w') 
		patternbuf = malloc((unsigned)tracksize);
	readbackbuf = malloc((unsigned)tracksize);
	/*
	 * Loop forever:
	 *	FOR (all patterns, repeat ad infinitum) 
	 *		FOR (all tracks on disk) 
	 *			write/read all sectors in track
	 *		END
	 *	END
	 */
	for (pass = 0; ; pass = (pass + 1) % NPATTERN) {
		if (patternbuf) 
			initpattern(patternbuf,tracksize,pass);
		for (sector = 0; sector < st.bad0sn; sector += st.nsect) {
			if (wantout) {
				(void) time(&now);
				printf("\n*** badm interrupted at %s",ctime(&now));
				exit(EX_OK);
			}
			if (sector && (sector % (st.nspc * 100)) == 0) 
				printf("cylinder %d\n", sector / st.nspc);
			format_track(sector,tracksize);
		}
	}
}

/*
 * Driver for `t' command (torture disk) 
 */
cmdt(ac,av)
	int ac;
	char **av;	/* main() argv sans first two entries */
{
#define TORTURE_MAX	5000
	int pass,sbint,torture;
	time_t now;
	char *sbuf;

	if (ac != 1) 
		usage();
	(void) time(&now);
	printf("Torture test, %s\n",ctime(&now));
	diskopen(av[0],0);
	get_devdata();		/* get disk geometry, etc. */
	/*
	 * Slightly more optimal page-aligned buffer,
	 * so kernel spends less time in vslock/vsunlock
	 */
	sbint = (int) malloc((unsigned)st.sectsize + 512);
	sbint += 511;		/* round up to next page */
	sbint &= ~511;
	sbuf = (char *)sbint;
	for (pass = 1; ; pass++) {
		printf("Pass %d\n",pass);
		torture = sizeof tortvec / sizeof tortvec[0] - 1;
		do {
			register daddr_t n,(*trou)();

			n = 0;
			trou = tortvec[torture];
			do {
#ifdef TTEST
				printf("%d\t%6d\n",torture,(*trou)(n));
#else
				(void) bread((*trou)(n),sbuf,st.sectsize);
#endif
				if (wantout) {
					printf("\n*** torture interrupted\n");
					exit(EX_OK);
				}
			} while (++n < TORTURE_MAX);
		} while (--torture >= 0);
	}
}

/*
 * Construct pathname of raw disk device and open it
 *
 * Outputs:
 *	dfd		file descriptor on disk device
 *	diskdevice	ASCII name of disk device (/dev/...) 
 */
diskopen(devun,omode)
	char *devun;	/* device/unit specifier, e.g., "hp1" */
	int omode;	/* mode for Open() syscall */
{
	strcat(diskdevice,devun);
	strcat(diskdevice,"c");
	if ((dfd = open(diskdevice,omode)) < 0) {
		perror(diskdevice);
		exit(EX_NOINPUT);
	}
#ifdef HPBUG
	/*
	 * hp.c driver bug.  After spinning up a new disk, the Volume
	 * Valid bit in the RM/RP drive is reset.  hp.c notices this
	 * on the first I/O request following spinup, and sneaks in
	 * a read of the bad sector table prior to doing the requested
	 * disk transfer.  The bug comes when the first transfer
	 * done by badm includes the header; hp.c mistakenly
	 * 1) applies the DKIOCHDR request to the bad sector table
	 *    read operation,
	 * 2) then resets sc_hdr, failing to honor badm's request for
	 *    header I/O.
	 * Bummer # 1 causes hp.c to get a mangled copy of the bad
	 * sector table, and bummer # 2 causes errors when badm tries
	 * to write sector headers during a `badm wbi' operation.
	 *
	 * Workaround:  perform a gratuitous non-header read operation
	 * upon opening the disk to get the driver's bad sector table
	 * read operation over and done with.  My read here will probably
	 * fail with an HCE, but even so, it will have served its purpose.
	 */
	{
		char gbuf[512];

		(void) read(dfd,gbuf,sizeof gbuf);
	}
#endif HPBUG
}

/*
 * Per-track logic for surface testing and bad sector flagging
 */
format_track(startsn,tracksize)
	daddr_t startsn;	/* starting sector number */
	int tracksize;		/* number of bytes in the track */
{
	register int errorval,i,readpass;
	char badmap[256];

	/*
	 * If doing the `w' command, write test patterns
	 */
	if (patternbuf && bwrite(startsn,patternbuf,tracksize) < 0) {
		/*
		 * Problems... slow down to 1 sector at a time
		 * ignoring errors (I'll catch them on readback) 
		 */
		i = 0;
		do (void) bwrite(startsn+i,patternbuf,st.sectsize);
		 while (++i < st.nsect);
	}
	/*
	 * Read test patterns (or whatever) back
	 */
	(void) numecc();		/* clear ECC count */
	if (bread(startsn,readbackbuf,tracksize) < 0 || numecc()) {
		/*
		 * Hard error or ECC, slow down to inspect sectors individually.
		 * The code here makes several passes over the entire track,
		 * because if the error is marginal, it might not show up on
		 * the first try.
		 *
		 * `badmap' limits the number of complaints to one per sector
		 * when I can't mark the sector as bad (i.e., Addbad()==0).
		 */
		readpass = 8;		/* init pass counter */
		bzero(badmap,sizeof badmap); /* no complaints issued yet */
		do {
			i = 0;		/* init sector # within track */
			do {
				if (badmap[i]) /* if previous pass found it */
					continue; /* ... that was enough */
				errorval = 0;
				if (bread(startsn+i,readbackbuf,st.sectsize) < 0
				     || (errorval = numecc())) {
					/*
					 * Report bad sector, then if `b' flag
					 * is set, bad it and retry the read
					 */
					printf("sn%d: ",startsn+i);
					printf(errorval?"soft (%d)":"hard",errorval);
					printf(" read error\n");
					if (bflag && addbad(startsn+i)) 
						i--;	/* retry */
					else
						badmap[i] = 1;
				}
			} while (++i < st.nsect);
		} while (--readpass > 0);
	}
}

/*
 * Obtain device-specific data and set up `st' structure
 */
get_devdata()
{
	register struct st *s;
	register struct disktab *d;
	struct iogstat gs;

	if (ioctl(dfd,DKIOGSTAT,(char *)&gs) < 0) 
		quit(EX_UNAVAILABLE,"cannot get drive status of %s\n",diskdevice);
	if (!gs.iogs_online) 
		quit(EX_IOERR,"%s: not online\n",diskdevice);
	s = &disktypes[0];
	do {
		if (strcmp(gs.iogs_vname,s->vname) == 0) {
			bcopy((char *)s,(char *)&st,sizeof st);
			goto calc0;
		}
	} while ((++s)->vname[0]);
	/*
	 * Cannot find a description of the disk in my tables,
	 * so try Getdiskbyname() to see if it knows anything
	 */
	s = &st;		/* use defaults */
	bcopy(gs.iogs_vname,s->vname,sizeof s->vname - 1);
	if ((d = getdiskbyname(gs.iogs_vname)) == 0) 
		quit(EX_OSFILE,"cannot get `disktab' information for `%s' disk\n",gs.iogs_vname);
	s->sectsize = d->d_secsize;
	s->nsect = d->d_nsectors;
	s->ntrack = d->d_ntracks;
	s->ncyl = d->d_ncylinders;
	s->nspc = s->nsect * s->ntrack;	/* sectors per cylinder */
calc0:	/*
	 * Sector number of first copy of bad sector table, per DEC Std 144.
	 * This is the first sector of the last track
	 */
	s = &st;
	s->bad0sn = s->ncyl * s->nspc - s->nsect;
}

/*
 * Initialize the in-core copy of the bad sector table:
 *	1.  install pack serial number
 *	2.  mark all bad sector slots as unused (all 1's) 
 */
initbst(packsn)
	int packsn;		/* serial number for pack */
{
	register struct bt_bad *be;

	bst.bt_csn = packsn;
	bst.bt_mbz = 0;
	bst.bt_flag = 0;
	be = &bst.bt_bad[0];
	do {
		be->bt_cyl = short_ones;
		be->bt_trksec = short_ones;
	} while (++be < &bst.bt_bad[MAXNUMBAD]);
	bsi++;
}

/*
 * Write good headers to every sector on the disk, a whole track at a time
 * including the last track where the bad sector file lives.
 */
initheaders()
{
	register daddr_t sn,cyli;

	cyli = st.ntrack * 100;	/* keep user entertained while I work */
	sn = 0;
	do {
		if (wantout) {
			printf("\nLast sector written: %d\n",sn - 1);
			exit(EX_OK);
		}
		(*st.wheader)(sn,(int)st.nsect,WHDR_VALID);
		if (--cyli <= 0) {
			printf("cylinder %d\n", sn / st.nspc + 1);
			cyli = st.ntrack * 100;
		}
	} while ((sn += st.nsect) < st.nspc * st.ncyl);
}

/*
 * Initialize a buffer with the requested pattern
 */
initpattern(bp,size,patno)
	char *bp;	/* buffer to initialize */
	int size;	/* size (bytes) */
	int patno;	/* pattern number */
{
	register unsigned short *pp;
	unsigned short patn;

	pp = (unsigned short *)bp;
	patn = testpattern[patno];
	printf("Write pattern %d = 0x%04x\n",patno,patn);
	size /= sizeof patn;
	do {
		*pp++ = patn;
	} while (--size > 0);
}

/*
 * Return the number of ECC errors corrected since the last call here
 */
numecc()
{
	register long lecc;
	static long necc;
	static int eccnotavail;

	if (eccnotavail) 
		return 0;
	lecc = necc;			/* previous count */
	if (ioctl(dfd,DKIOGETECC,(char *)&necc) < 0) {
		printf("WARNING: ECC COUNTS NOT AVAILABLE, LIMPING ALONG\n");
		eccnotavail++;
		return 0;
	}
	return necc - lecc;
}

/*
 * Printf a diagnostic message and exit
 */
/*VARARGS2*/
quit(exitcode,fmt,args)
	int exitcode;
	char *fmt;
{
	_doprnt(fmt,&args,stdout);	/* tacky, tacky */
	if (exitcode == EX_SOFTWARE) 
		abort();		/* make core image for debugging */
	exit(exitcode);
	/* should *NEVER EVER* get here :-) */
}

/*
 * Read the bad sector table off the pack into core, and validate it
 */
readbst()
{
	register int n;
	register struct bt_bad *e;
	int active;
	daddr_t btabsn, cursn, lastsn;

	btabsn = st.bad0sn;	/* where the first copy of the table is */
	n = 5;			/* 5 copies, per DEC std 144 */
	while (bread(btabsn,(char *)&bst,sizeof bst) < 0) {
		printf("WARNING: CANNOT READ BAD SECTOR TABLE (sn%d)\n",btabsn);
		btabsn += 2;	/* try another copy */
		if (--n <= 0) {
			/*
			 * If I can't read the current bad sector table,
			 * I can't augment it now, can I?
			 */
			printf("\n\
You might have to re-format the *WHOLE* disk with `badm wbi'\n");
			exit(EX_IOERR);
		}
	}
	printf("Bad sector table read from sn%d\n",btabsn);
	/*
	 * Validate the contents per these rules:
	 *	o  Active slots, if any, must be clustered at the
	 *	   beginning of the table, and inactive slots at the end
	 *	o  Cylinder, track, and sector numbers must be within
	 *	   the legal ranges for the type of disk I'm working on
	 *	o  Bad sector entries must be in order of ascending
	 *	   disk addresses
	 */
	e = &bst.bt_bad[0];
	active = 1;			/* in active region of table */
#ifdef lint
	lastsn = 0;			/* avoid lint complaint */
#endif
	do {
		cursn = btsector(e);	/* convert entry to sector number */
		if (cursn == BTINVALID) 
			quit(EX_DATAERR,"Entry %d: illegal disk address, cyl=%x trksec=%x\n",
				e - &bst.bt_bad[0], e->bt_cyl, e->bt_trksec);
		if (cursn == BTEMPTY) 
			active = 0;
		else {
			if (!active ||
			    e > &bst.bt_bad[0] && cursn <= lastsn) 
				quit(EX_DATAERR,"Entry %d: out of order\n",
					e - &bst.bt_bad[0]);
			lastsn = cursn;
		}
	} while (++e < &bst.bt_bad[MAXNUMBAD]);
	bsi++;		/* note that bst is set up */
}

/*
 * Random places on disk
 */
/*ARGSUSED*/
daddr_t
tort0(n)
	daddr_t n;
{
	long random();

	return random() % (st.nspc * st.ncyl);
}

/*
 * Start at opposite ends of disk, then oscillate, closing in on center
 */
daddr_t
tort1(n)
	daddr_t n;
{
	register daddr_t cyl,dist;

	dist = n % st.ncyl >> 1;
	cyl = n & 1 ? st.ncyl - dist - 1 : dist;
	return cyl * st.nspc;
}

/*
 * Slam heads from end to end
 */
daddr_t
tort2(n)
	daddr_t n;
{
	/*
	 * 0x301 is about 1/4 as abusive as 0x001
	 */
	if (n & 0x301) 
		return (st.ncyl - 1) * st.nspc;
	return 0;
}

/*
 * 4 close, large seek, 4 close, large seek ...
 */
daddr_t
tort3(n)
	register daddr_t n;
{
	register daddr_t cyl;
	static daddr_t tab4[4] = { 0, 2, 1, 3 };

	cyl = tab4[n & 3] + (n >> 3) % (st.ncyl - 8 >> 1);
	if (n & 4) 
		cyl += st.ncyl >> 1;
	return cyl * st.nspc;
}

usage()
{
	fprintf(stderr,"Usage:\n\
badm r DISKDEVICE\n\
\tread entire disk sequentially, scanning for errors and ECCs\n\
\t(does not alter disk)\n");
	fprintf(stderr,"\n\
badm t DISKDEVICE\n\
\tread-only torture test for disk arm\n");
	fprintf(stderr,"\n\
badm w[bi] DISKDEVICE [ PACKSN ]\n\
\tw    write and readback test patterns (bashes filesystems!)\n\
\twb   same as `w', and augments bad sector table\n\
\twbi  same as `wb', and clears all headers and bad sector tables first\n\
\t     (decimal pack serial number PACKSN required with `wbi')\n\
");
	fprintf(stderr,"\n\
badm a DISKDEVICE SECTORNO [ SECTORNO ... ]\n\
\tadd specified sectors to the bad sector table\n\
");
	fprintf(stderr,"\n\
DISKDEVICE is the device name and unit number, like `hp1'\n\
SECTORNO is a decimal sector number\n\
*** READ THE CAUTIONS IN THE MANUAL ENTRY FIRST!!! ***\n\
");
	exit(EX_USAGE);
}

char	nohmemmsg[] = "cannot get memory for header I/O\n";

/*
 * Sector header I/O for RM-class MASSBUS disks (incl. Eagle) 
 */
whdr_rm(sn,nb,type)
	daddr_t sn;		/* disk sector number */
	int nb;			/* number of sectors */
	int type;		/* WHDR_xxx */
{
	static struct rmhdrbuf {
		unsigned short hdr1;
		char h2sector;
		char h2track;
		char pad[512];
	} *ebuf;
	register struct rmhdrbuf *e;
	register int i;

	if ((e = ebuf) == 0) {
		e = (struct rmhdrbuf *)malloc(sizeof (struct rmhdrbuf) * (unsigned)st.nsect);
		if (e == 0) 
			quit(EX_OSERR,nohmemmsg);
		ebuf = e;
	}
	i = 0;
	do {
		e->hdr1 = 1 << 12;		/* 16-bit format */
		if (type == WHDR_VALID) 
			e->hdr1 |= 0xC000;	/* good (valid) sector */
		e->hdr1 |= (sn + i) / st.nspc;
		e->h2track = (sn + i) % st.nspc / st.nsect;
		e->h2sector = (sn + i) % st.nsect;
		e++;
	} while (++i < nb);
	if (ioctl(dfd,DKIOCHDR,(char *)0) < 0) {
		perror("DKIOCHDR");
		exit(EX_UNAVAILABLE);
	}
#ifdef HPBUG
	/*
	 * Circumvent hp.c driver bug.  When testing to see if I/O will
	 * tread beyond the end of the disk, hpstrategy() does not take
	 * into account the fact that the sector size during header I/O
	 * is 516 (not 512) bytes.  Thus, the driver rejects some
	 * legitimate requests because it erroneously concludes that
	 * they would result in a transfer beyond the end of the disk.
	 *
	 * The workaround here deducts the number of bytes consumed by
	 * the headers (nb * 4) from the `right' (nb * 516) byte count;
	 * this appeases hp.c, but it also results in a partial write to
	 * the last sector of the bunch.  The partial write is harmless
	 * here, since we care only about the headers and not about
	 * anything else in the sectors.  The only possible problem can
	 * occur if more than 512 bytes are deducted (i.e., nb > 128),
	 * because that would eat into the header of the last sector.
	 */
	if (nb > 128)		/* should never happen :-) */
		quit(EX_SOFTWARE,"\nSOFTWARE BUG: nb (%d) > 128\n",nb);
	if (bwrite(sn, (char *)ebuf, st.sectsize * nb) < 0) 
#else HPBUG
	if (bwrite(sn, (char *)ebuf, (char *)e - (char *)ebuf) < 0) 
#endif HPBUG
		printf("sn%d: HEADER WRITE FAILED, CONTINUING\n",sn);
}

/*
 * Sector header I/O for RP04/5/6 MASSBUS disks
 */
whdr_rp(sn,nb,type)
	daddr_t sn;		/* disk sector number */
	int nb;			/* number of sectors */
	int type;		/* WHDR_xxx */
{
	static struct rphdrbuf {
		unsigned short hdr1;
		char h2sector;
		char h2track;
		unsigned short keyfield1,keyfield2;
		char pad[512];
	} *ebuf;
	register struct rphdrbuf *e;
	register int i;

	if ((e = ebuf) == 0) {
		e = (struct rphdrbuf *)malloc(sizeof (struct rphdrbuf) * (unsigned)st.nsect);
		if (e == 0) 
			quit(EX_OSERR,nohmemmsg);
		ebuf = e;
	}
	i = 0;
	do {
		/*
		 * Bad sector handling on RP04/5/6s is a kludge, since there
		 * is no true hardware support (a la RM series drives).
		 * The convention employed in badm and the kernel (hp.c) to
		 * denote a bad sector is to clear 16-bit format (bit 12) 
		 * in the header.  When reading accessing the sector, the
		 * drive will report "format error", which hp.c assumes to
		 * mean "RP0x bad sector".
		 */
		e->hdr1 = 0;			/* invalid */
		if (type == WHDR_VALID) 
			e->hdr1 = 0x1000;	/* good (valid) sector */
		e->hdr1 |= (sn + i) / st.nspc;
		e->h2track = (sn + i) % st.nspc / st.nsect;
		e->h2sector = (sn + i) % st.nsect;
		e->keyfield1 = 0;		/* what are these for? */
		e->keyfield2 = 0;
		e++;
	} while (++i < nb);
	if (ioctl(dfd,DKIOCHDR,(char *)0) < 0) {
		perror("DKIOCHDR");
		exit(EX_UNAVAILABLE);
	}
#ifdef HPBUG
	/*
	 * Circumvent hp.c driver bug.
	 * See comments in whdr_rm() for explanation of this bug.
	 * Sector size during header I/O for RP06 is 520 bytes
	 * (512 sector data + 8 header).
	 */
	if (nb > 512 / 8)		/* should never happen :-) */
		quit(EX_SOFTWARE,"\nSOFTWARE BUG: nb (%d) > 512 / 8\n",nb);
	if (bwrite(sn, (char *)ebuf, st.sectsize * nb) < 0) 
#else HPBUG
	if (bwrite(sn, (char *)ebuf, (char *)e - (char *)ebuf) < 0) 
#endif HPBUG
		printf("sn%d: HEADER WRITE FAILED, CONTINUING\n",sn);
}

/*ARGSUSED*/
whdr_unknown(sn,nb,type)
	daddr_t sn;		/* disk sector number */
	int nb;			/* number of sectors */
	int type;		/* WHDR_xxx */
{
	quit(EX_UNAVAILABLE,"don't know how to read/write headers on `%s' disks\n",st.vname);
}

/*
 * Write the bad sector table out to the disk
 *
 * Writes 5 copies, in the first 5 even-numbered sectors of the
 * last track of the last cylinder of the disk (per DEC Std 144) 
 *
 * Also informs the driver there's a new bad sector table, because some
 * drivers (e.g., hp.c) keep in-core copies, which are read only at bootup
 * or when the drive is spun up (i.e., when Volume-Valid is reset) 
 */
writebst()
{
	register int i;
	static int noioctl;

	/*
	 * Write 5 copies, per DEC std 144
	 */
	i = 0;
	do {
		if (bwrite(st.bad0sn + i * 2, (char *)&bst, sizeof bst) < 0) 
			printf("ERROR WRITING BAD TABLE %d, CONTINUING\n",i);
	} while (++i < 5);
	/*
	 * Now tell the driver that the bad block table has been updated.
	 * This is necessary because hp.c maintains an in-core copy of
	 * this table (in "hpbad"), and, in order to do the job right,
	 * the in-core copy must be accurate.
	 */
	if (!noioctl && ioctl(dfd,DKIONEWBAD,(char *)0) < 0) {
		noioctl++;
		printf("WARNING: Cannot inform driver of new bad sector table\n");
	}
#ifdef HPBUG
	/*
	 * Perform a gratuitous read operation to force driver to read
	 * the bad sector table into the in-core "hpbad" table.
	 * See explanation in Diskopen() for more details.
	 */
	{
		char gbuf[512];

		(void) bread((daddr_t)0,gbuf,sizeof gbuf);
	}
#endif HPBUG
}
