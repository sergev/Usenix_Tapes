/*
 * mdump -- put daily/weekly backups onto a single tape
 *
 * Mdump simplifies the tedious process of daily and weekly backups by
 * putting all daily backups onto a single tape.  Weekly backups are
 * performed the same way.  The /etc/dumptab file defines the file
 * position on the tape each filesystem is written at and the day of
 * the week the weekly backup is done.  Filesystems that are scheduled
 * for a weekly backup are marked with a dummy file on the daily tape
 * so that file numbering remains constant.  The same is done for dailies
 * on a weekly tape.
 *
 * usage: mdump [-d] [-w] [-dl n] [-wl n] [-l] [-e] [-t day] [-f [host:]/dev/rmtx] [-db]
 *
 * -d	write daily backups with default dump level of DAILY
 *
 * -w	write weekly backups with default dump level of WEEKLY
 *
 * -dl n	write daily backups at dump level n
 *
 * -wl n	write weekly backups at dump level n
 *
 * -l	suppress generation of the listing file
 *
 * -e	run listing code after an error exit
 *
 * -t day	run mdump for day (sun - sat) instead of today
 *
 * -f [host:]/dev/rmtx	tape device name.  If the host: prefix
 *	is used, the backup tape will be written over the network to
 *	the tape drive on the named host.
 *
 * -db	Write debug information to stderr
 *
 * NOTE to installers:  Site dependent commands have been flagged in the
 * code with the comment "EDIT THIS".
 *
 * Written by Paul Pomes, University of Illinois, Computing Services Office.
 * Copyright 1985 by Paul Pomes and University of Illinois Board of Trustees
 *
 * This program may be freely reproduced and distributed provided the
 * copyright notice above is included in all copies.  It may not be
 * included in any software product or distribution sold for profit
 * without the permission of the author.
 *
 * UUCP:	{ihnp4,pur-ee,convex}!uiucdcs!uiucuxc!paul
 * ARPANET:	paul%uiucuxc@uiuc.arpa
 * CSNET:	paul%uiucuxc@uiuc.csnet
 * ICBM:	40 07 N / 88 13 W
 * US Mail:	Univ of Illinois, CSO, 1304 W Springfield Ave, Urbana, IL  61801
 *
 * $Log:	mdump.c,v $
 * Revision 1.10  85/09/11  14:06:08  root
 * Changed pr command to use two column printing instead of three.  -pbp
 * 
 * Revision 1.9  85/08/14  16:25:52  root
 * Threats of mayhem from operators induced me to re-insert -d and -w flags.
 * -pbp
 * 
 * Revision 1.8  85/08/14  13:45:47  paul
 * Revamped dump level selection with inclusion of wl and dl switches,
 * removed w and d as superfluous.  Moved commands back to the sprintfs
 * that use them.
 * 
 * Revision 1.7  85/08/07  16:30:08  paul
 * Added bare-bones System 5 defines (index -> strchr, etc).  Large hacks
 * needed to do the same for the mag tape ioctls.  Another day, far away.
 * 
 * Revision 1.6  85/08/07  16:03:57  root
 * Added n switch to dump command so that operators are notified.
 * Extremely effective annoyance feature.   -pbp
 * 
 * Revision 1.5  85/07/11  14:44:07  paul
 * Added recovery question for restore listings of filesystems with no files.
 * pbp
 * 
 * Revision 1.4  85/07/09  21:18:09  root
 * *** empty log message ***
 * 
 * Revision 1.3  85/07/05  17:13:39  root
 * Added -t day-of-week flag for enhanced error recovery.   -pbp
 * 
 * Revision 1.2  85/07/03  23:23:04  root
 * Added -e flag to force listings after "rsh mt" errors.   -pbp
 * 
 * Revision 1.1  85/07/02  13:47:46  root
 * Initial revision
 */

#ifndef lint
static char	RcsId[] = "$Header: mdump.c,v 1.10 85/09/11 14:06:08 root Exp $";
#endif

#include	<stdio.h>
#ifdef	SYS5
#include	<string.h>
#define		index		strchr
#define		rindex		strrchr
#else
#include	<strings.h>
#endif
#include	<ctype.h>
#include	<sys/time.h>
#include	<sys/file.h>
#include	<sys/types.h>
#include	<sys/mtio.h>
#include	<sys/ioctl.h>
#include	<dumptab.h>

#define		equal(s1, s2)	(strcmp(s1, s2) == 0)

/* default dump levels */
#define		DAILY		5
#define		WEEKLY		4

/* default output devices */
#define		DEF_DEV		"/dev/rmt0"
#define		DEF_NRDEV	"/dev/nrmt0"

/* dumptab file.  must be the same as referenced by getdtent() functions */
#define		DUMPTAB		"/etc/dumptab"

/* temp file for listings */
#define		DLIST		"/tmp/mdumpXXXXXX"

/* maximum number of filesystems */
#define		MAX_FS		30

/* size of shorter strings, e.g., rhost[] */
#define		SSTR_SIZE	12

/* size of longer strings, e.g., command[] */
#define		LSTR_SIZE	160

/* copy of argv[0] for error messages */
char		*self;

/* resident host name */
char		myname[SSTR_SIZE];

/* debug messages printed if set */
int		debug;

/* run listing code only */
int		elist;

/* state table of tape files.  if dump_state[0] is set, then there's a dump
 * file there.  If zero, there's a dummy file on the tape instead.  If -1
 * then EOT has been reached or some other dump(8) error.  Typical usage
 * would be dump_state[dt->dt_position].
 */
int		dump_state[MAX_FS];

/* mag tape stuff */
struct mt_cmds {
	char	*c_name;
	int	c_code;
	int	c_ronly;
} com[] = {
	{ "eof",	MTWEOF,	0 },
	{ "fsf",	MTFSF,	1 },
	{ "bsf",	MTBSF,	1 },
	{ "rew",	MTREW,	1 },
	{ 0 }
};

extern time_t	time();

main(argc, argv)
int	argc;
char	**argv;
{
	/* a useful counter */
	int		i;

	/* flags set for default dump levels and types */
	int		daily = 0;
	int		weekly = 0;

	/* dump level (0 to 9).  -1 indicates not selected */
	int		dlevel = -1;	/* daily */
	int		wlevel = -1;	/* weekly */

	/* create a compact listing if set */
	int		dlist = 1;

	/* run mdump for day argument instead of today of set */
	char		newday[4];

	/* listing file */
	char		*list_file = DLIST;

	/* backup device */
	char		*device, *nrdevice;

	/* remote host name */
	char		rhost[SSTR_SIZE];

	/* time stuff */
	struct tm	*tm;
	time_t		clock;

	/* dumptab info */
	struct dumptab	*dt;

	/* library routines */
	char		*malloc(), *mktemp();

	/* routine to set new tm->tm_wday value for -t argument */
	struct tm	*setday();

	/* squirrel a copy of *argv[0] away for use in error messages */
	self = malloc((unsigned) (strlen(*argv) + 1));
	(void) strcpy(self, *argv);

	newday[0] = rhost[0] = '\0';
	device = DEF_DEV;
	nrdevice = DEF_NRDEV;
	(void) gethostname(myname, SSTR_SIZE-1);
	/*
	 * parse arguments
	 */
	i = 1;
	while (i < argc && *argv[i] == '-') {
		if (equal(argv[i]+1, "db")) {
			/* db - set debug level */
			debug++;
			i++;
			fprintf(stderr, "%s: debug option enabled\n", self);
		}
		else if (equal(argv[i]+1, "d")) {
			/* d - dailies of default dump level */
			daily++;
			i++;
		}
		else if (equal(argv[i]+1, "w")) {
			/* w- weeklies of default dump level */
			weekly++;
			i++;
		}
		else if (equal(argv[i]+1, "dl")) {
			/* dl - read daily dump level from next argument */
			i++;
			if ((dlevel = atoi(argv[i])) > 9) {
				fprintf(stderr, "%s: invalid daily dump level (%d)\n", self, dlevel);
				exit(1);
			}
			i++;
		}
		else if (equal(argv[i]+1, "wl")) {
			/* wl - read weekly dump level from next argument */
			i++;
			if ((wlevel = atoi(argv[i])) > 9) {
				fprintf(stderr, "%s: invalid weekly dump level (%d)\n", self, wlevel);
				exit(1);
			}
			i++;
		}
		else if (equal(argv[i]+1, "l")) {
			/* l - suppress listings of files */
			dlist = 0;
			i++;
		}
		else if (equal(argv[i]+1, "e")) {
			/* e - run listing code only */
			printf("Listings only - nothing will be written to tape\n");
			elist++;
			i++;
		}
		else if (equal(argv[i]+1, "t")) {
			/* t - read new dump day-of-week from next argument */
			i++;
			(void) strncpy(newday, argv[i], 3);
			i++;
		}
		else if (equal(argv[i]+1, "f"))
		{
			/*
			 * f - read backup device from next argument
			 * turn /dev/rmt names into /dev/nrmt
			 */
			char	temp[40], *p1, *p2;

			i++;
			p2 = (char *) NULL;
			if ((p1 = index(argv[i], ':')) != 0) {
				(void) strncpy(rhost, argv[i], p1-argv[i]+1);
				*(index(rhost, ':')) = '\0';
				p1++;
			}
			if (p1 == 0)
				p1 = argv[i];
			device = malloc((unsigned) (strlen(p1) + 1));
			(void) strcpy(device, p1);
			if ((p2 = rindex(argv[i], '/')) == 0) {
				fprintf(stderr, "%s: Output device must be a tape drive, e.g., /dev/rmt1, uxc:/dev/rmt2\n", self);
				exit(1);
			}
			p2++;
			(void) strncpy(temp, p1, p2-p1);
			*(temp + (p2-p1)) = '\0';
			nrdevice = malloc((unsigned) (strlen(argv[i])+2));
			if (*p2 == 'n')
				(void) sprintf(nrdevice, "%s%s", temp, p2);
			else
				(void) sprintf(nrdevice, "%sn%s", temp, p2);
			if (debug)
				fprintf(stderr, "rhost %s, device %s, nrdevice %s\n", rhost, device, nrdevice);
			i++;
		}
		else
		{
			/* command line errors */
			fprintf(stderr, "%s: %s - bad flag\n", self, argv[i]+1);
			fprintf(stderr, "Usage: %s [-d] [-w] [-dl n] [-wl n] [-l] [-e] [-t day] [-f [host:]/dev/rmtx] [-db]\n", self);
			exit(1);
		}
	}
	if (daily && dlevel < 0)
		dlevel = DAILY;
	if (weekly && wlevel < 0)
		wlevel = WEEKLY;
	if (wlevel < 0 && dlevel < 0) {
		fprintf(stderr, "%s: No action specified.\n", self);
		fprintf(stderr, "Invoke %s with at least one of the following: -d, -w, -dl n, -wl n\n", self);
		exit(1);
	}
	clock = time((time_t *) 0);
	if (*newday)
		tm = setday(newday);
	else
		tm = localtime(&clock);
	mtio(rhost, device, "rew");
	wrt_dtinfo(rhost, nrdevice);
	setdtent();
	for (i = 1; (dt = getdtpos(i)); i++) {
		if (dt->dt_weekly == tm->tm_wday) {
			if (wlevel > -1) {
				if (wrt_dmp(rhost, nrdevice, dt, wlevel))
					break;
			}
			else
				wrt_dummy(rhost, nrdevice, dt);
		}
		else {
			if (dlevel > -1) {
				if (wrt_dmp(rhost, nrdevice, dt, dlevel))
					break;
			}
			else
				wrt_dummy(rhost, nrdevice, dt);
		}
	}
	/* write the last EOF to form EOT and rewind the tape
	 * by using the rewind-on-close device.
	 */
	if (! elist) {
		printf("Really rewinding tape\n");
		mtio(rhost, device, "eof");
	}
	if (dlist) {
		char	command[LSTR_SIZE];

		(void) mktemp(list_file);
		mtio(rhost, device, "rew");

		/* skip over the zero'th file of /etc/dumptab */
		mtio(rhost, nrdevice, "fsf");
		setdtent();
		for (i = 1; (dt = getdtpos(i)); i++)
			if (list_tape(rhost, nrdevice, dt, list_file))
				break;
		/* EDIT THIS for preferred print command */
		(void) sprintf(command, 
#ifdef vax
			"ibmprint -f -h \"%s Dlist\" %s",   /* uiucuxc */
#else
			"lpr -r -s -J %s_Dlist %s",	/* osiris */
#endif
			myname, list_file);
		if (debug)
			fprintf(stderr, "print command: %s\n", command);
		if (i = (system(command) >> 8))
			fprintf(stderr, "%s: %s exitted abnormally (%d).\n", self, command, i);
		printf("Last rewind of tape\n");
		mtio(rhost, device, "rew");
	}
	exit(0);
}

/*
 * mtio -- perform a local/remote tape operation
 *
 *	for local tapes (host[0] == '\0') use the ioctl calls.
 *	remote tapes use an rsh command.
 *
 *	parameters:
 *		host (IN) -- name of remote host, if host[0] != '\0'
 *		dev (IN) -- tape drive name
 *		cp (IN) -- tape opcode
 *	returns:
 *		none
 *	side effects:
 *		none
 *	deficiencies:
 *		any error causes an error exit from the program
 */
mtio(host, dev, cp)
char	*host, *dev, *cp;
{
	struct mt_cmds	*comp;

	/* tape file descriptor */
	int		mtfd;

	/* return code from system() call */
	int		status;

	/* from sys/mtio.h */
	struct mtop	mt_com;

	/* buffer for rsh command */
	char		command[LSTR_SIZE];

	if (*host == '\0') {
		for (comp = com; comp->c_name != NULL; comp++)
			if (strncmp(cp, comp->c_name, strlen(cp)) == 0)
				break;
		if (comp->c_name == NULL) {
			fprintf(stderr, "%s: mtio: don't grok \"%s\"\n", self, cp);
			exit(1);
		}
		if ((mtfd = open(dev, comp->c_ronly ? 0 : 2)) < 0) {
			fprintf(stderr, "%s: mtio: open of ", self);
			perror(dev);
			exit(1);
		}
		mt_com.mt_op = comp->c_code;
		mt_com.mt_count = 1;
		if (ioctl(mtfd, MTIOCTOP, &mt_com) < 0) {
			fprintf(stderr, "%s: mtio: %s %s %d ", self, dev,
				comp->c_name, mt_com.mt_count);
			perror("failed");
			exit(1);
		}
		(void) close(mtfd);
	}
	else {
		(void) sprintf(command, "rsh %s mt -t %s %s", host, dev, cp);
		if (debug)
			fprintf(stderr, "tape command %s\n", command);
		if (status = (system(command) >> 8)) {
			fprintf(stderr, "%s: %s exitted abnormally (%d).  Restart %s\n", self, command, status, self);
			exit(1);
		}
	}
}

/*
 * wrt_dmp -- write a filesystem dump on the dump tape
 *
 *	for local tapes (host[0] == '\0') use dump
 *	remote tapes use rdump
 *
 *	parameters:
 *		host (IN) -- name of remote host if host[0] != '\0'
 *		dev (IN) -- tape drive name
 *		dt (IN) -- dumptab pointer
 *		level (IN) -- dump level (0-9)
 *	returns:
 *		0 if dump command successful, 1 if not
 *	side effects:
 *		alters dump_state table
 *	deficiencies:
 *		any error causes an error exit from the program
 */
wrt_dmp(host, dev, dt, level)
char		*host, *dev;
int		level;
struct dumptab	*dt;
{
	char	command[LSTR_SIZE];
	int	status;

	if (elist) {
		dump_state[dt->dt_position] = 1;
		return(0);
	}
	/* EDIT THIS for preferred dump command */
	(void) sprintf(command, " dump nuf%d %s%c%s %s",
			level, host, (*host ? ':' : ' '), dev, dt->dt_name);
	if (*host)
		command[0] = 'r';
	if (debug)
		fprintf(stderr, "dump command %s\n", command);
	printf("Dumping %s\n", dt->dt_name);

	/* dump(8) exits with 1 if all is well */
	if ((status = (system(command) >> 8)) != 1) {
		fprintf(stderr, "%s: %s exitted abnormally (%d).\n", self, command, status);
		fprintf(stderr, "\tTape will rewind and create listings of the successful dumps.\n");
		dump_state[dt->dt_position] = -1;
		mtio(host, dev, "bsf");
		return(1);
	}
	dump_state[dt->dt_position] = 1;
	return(0);
}

/*
 * list_tape -- generate a "restore tf" listing for the current tape file
 *
 *	list_tape creates a command line to be executed by system() and
 *	re-directs the output to a listing file.
 *
 *	parameters:
 *		host (IN) -- name of remote host if host[0] != '\0'
 *		dev (IN) -- tape drive name
 *		dt (IN) -- dumptab pointer
 *		lfile (IN) -- name of listing file
 *	returns:
 *		0 for normal results, 1 if at end of tape
 *	side effects:
 *		none
 *	deficiencies:
 *		any error causes an error exit from the program
 */
list_tape(host, dev, dt, lfile)
char		*host, *dev, *lfile;
struct dumptab	*dt;
{
	FILE		*fp;
	int		status;
	char		command[LSTR_SIZE];

	if (dump_state[dt->dt_position] == 0) {
		/* no file here so write a message on the dump listing */
		if ((fp = fopen(lfile, "a")) == NULL) {
			fprintf(stderr, "%s: list_tape ", self);
			perror(lfile);
			exit(1);
		}
		fprintf(fp, "\n\n\tNo dump file for %s on this tape.\n\f",
			dt->dt_name);
		(void) fclose(fp);
		/* advance the tape by one tape mark */
		printf("Skipping dummy file for %s\n", dt->dt_name);
		mtio(host, dev, "fsf");
		return(0);
	}
	else if (dump_state[dt->dt_position] == 1) {
		/* build the appropriate restore tf command and execute it */
		/* EDIT THIS for preferred listing command */
		(void) sprintf(command, " restore tf %s%c%s | sort +1 | pr -2 -w132 -f -h \"%s Dump Listing of %s\" >> %s",
			host, (*host ? ':' : ' '), dev, myname, dt->dt_name, lfile);
		if (*host)
			command[0] = 'r';
		if (debug)
			fprintf(stderr, "list command %s\n", command);
		printf("Listing %s\n", dt->dt_name);
		if (status = (system(command) >> 8)) {
			char	c,d;

			fprintf(stderr, "%s: %s exitted abnormally (%d).  Continue? [yn] ", self, command, status);
			while ((d = getc(stdin)) != '\n')
				c = d;
			if (c != 'y')
				exit(1);
		}
		/* advance the tape to end of file */
		mtio(host, dev, "fsf");
		return(0);
	}
	return(1);
}

/*
 * wrt_dtinfo -- write /etc/dumptab onto the tape
 *
 *	writing a copy of /etc/dumptab onto the tape as file #0 provides
 *	a catalogue of the tape files and insulates backup volumes from
 *	changes in /etc/dumptab.  Elsewhere on the tape it acts as a 
 *	place holder for a non-selected dump, e.g., daily on a weekly-only
 *	tape.
 *
 *	parameters:
 *		host (IN) -- name of remote host
 *		dev (IN) -- tape drive name
 *	returns:
 *		none
 *	side effects:
 *		none
 *	deficiencies:
 *		any error causes an error exit from the program
 */
wrt_dtinfo(host, dev)
char	*host, *dev;
{
	char	command[LSTR_SIZE];
	int	status;

	if (elist)
		return;
	if (*host)
		/* EDIT THIS for preferred method */
		(void) sprintf(command, "rcp %s %s:/tmp/xyzzy.mdump; rsh %s \"dd if=/tmp/xyzzy.mdump of=%s bs=512; rm /tmp/xyzzy.mdump\"",
			DUMPTAB, host, host, dev);
	else
		(void) sprintf(command, "dd if=%s of=%s bs=512",
			DUMPTAB, dev);
	printf("Copying %s to tape\n", DUMPTAB);
	if (status = (system(command) >> 8)) {
		fprintf(stderr, "%s: %s exitted abnormally (%d).  Restart %s\n", self, command, status, self);
		exit(1);
	}
}

/*
 * wrt_dummy -- write a dummy file onto the tape
 *
 *	write a dummy file onto the tape to take the place of a non-
 *	scheduled backup.  this preserves tape order.
 *
 *	parameters:
 *		host (IN) -- name of remote host
 *		dev (IN) -- tape drive name
 *		dt (IN) -- dumptab pointer
 *	returns:
 *		none
 *	side effects:
 *		alters dump_state table
 *	deficiencies:
 *		any error causes an error exit from the program
 */
wrt_dummy(host, dev, dt)
char		*host, *dev;
struct dumptab	*dt;
{
	char	command[LSTR_SIZE];
	int	status;

	if (elist)
		return;
	if (*host)
		/* EDIT THIS for preferred method */
		(void) sprintf(command, "rcp %s %s:/tmp/xyzzy.mdump; rsh %s \"dd if=/tmp/xyzzy.mdump of=%s bs=512; rm /tmp/xyzzy.mdump\"",
			DUMPTAB, host, host, dev);
	else
		(void) sprintf(command, "dd if=%s of=%s bs=512",
			DUMPTAB, dev);
	printf("Writing place holder file for %s to tape\n", dt->dt_name);
	if (status = (system(command) >> 8)) {
		fprintf(stderr, "%s: %s exitted abnormally (%d).  Restart %s\n", self, command, status, self);
		exit(1);
	}
}

/*
 * setday -- set tm.tm_wday to value consistent with argument
 *
 *	tm.tm_wday has a legal range of 0 (for Sunday) to 6 (for Saturday).
 *	day is a three character string ("sun", "mon", etc) used to determine
 *	the value of tm.tm_wday.
 *
 *	parameters:
 *		day (IN/OUT) -- day of week string
 *	returns:
 *		pointer to static tm struct
 *	side effects:
 *		changes day string to all lower case
 *	deficiencies:
 *		any error causes an error exit from the program
 */
struct tm *
setday(day)
char	*day;
{
	static struct tm	mtm;
	char			*p;

	for (p = day; *p; p++)
		if (isupper(*p))
			*p += ('a' - 'A');
	if (equal(day, "sun"))
		mtm.tm_wday = 0;
	else if (equal(day, "mon"))
		mtm.tm_wday = 1;
	else if (equal(day, "tue"))
		mtm.tm_wday = 2;
	else if (equal(day, "wed"))
		mtm.tm_wday = 3;
	else if (equal(day, "thu"))
		mtm.tm_wday = 4;
	else if (equal(day, "fri"))
		mtm.tm_wday = 5;
	else if (equal(day, "sat"))
		mtm.tm_wday = 6;
	else {
		fprintf(stderr, "%s: setday: illegal day of week (%s)\n", self, day);
		fprintf(stderr, "\tLegal values are sun, mon, tue, etc.\n");
		exit(1);
	}
	return(&mtm);
}
