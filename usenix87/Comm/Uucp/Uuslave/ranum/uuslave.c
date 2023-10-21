/* uuslave.c - client uucp(R) connection */
/* 	(uucp is copyright AT&T)	 */
/* this is designed to run on IBM PCs w/ */
/* a modem or direct connection to UNIX  */
/* no guarantees of functionality...     */

/* original author ?   */
/* very extensive and brutal hacks by Marcus J Ranum */

/* I take no responsibility for the working code here. */
/* all the comments (for what they're worth) are mine) */
/* I added all the incremental debug information stuff */

#include "stdio.h"
#include "signal.h"
/* if your system does not support signal, comment all */
/* references to signal out and it may still work.     */

#define	O_RDONLY	000	/* open for reading */
#define	O_WRONLY	001	/* open for writing */
#define	O_RDWR		002	/* open for read & write */

/* mysterious numbers that are possibly CP/M specific. */
/* they are used as masks for file I/O */
#define	CLOSE	1
#define	CTRL	0
#define LNGDAT	2
#define SHTDAT	3
#define	MAGIC	0125252
#define	EOT	4
#define	RR	4
#define	INITC	5
#define	INITB	6
#define	INITA	7

int     sigint ();
int     abort ();
char   *strcat ();
char   *strcpy ();
long    lseek ();
extern  errno;


int     wndsiz = 1;
int     segsiz = 1;

/* these are used later as buffers for ttynames, etc, etc, etc. */
char    msgi[256],
        msgo[256],
        cmnd[8],
        srcnam[32];
char    dstnam[32],
        dskbuf[256],
        msgbld[256];

/* fdtty is terminal file descriptor */
/* fddsk is disk file descriptor */
int     fdtty,
        fddsk;

/* I believe these are the message sequence numbers */
int     tt,
        xxx,
        yyy,
        rseq,
        wseq;

/* this is the communications device/port/whatever */
char   *ttynam;

/* error log file name, debug flags, etc.*/
FILE * errfile;
char   *errlog;
int     errflg;
int     debugmode = 0;

/* these next few strings are strings uucp expects to see. */
/* they shouldn't need changing */
/* messages that get sent out */
char   *msgo0;
char    msgo1[] = "Password:";
char    msgo2[] = "\20Shere\0";
char    msgo3[] = "\20ROK\0\20Pg\0";
char    msgo4[] = "\20OOOOOO\0";
char    msgo5[] = "...abort...";

/* messages that are read in */
char    msgi0[] = "uucp\n";
char    msgi1[] = "s8000\n";
char    msgi2[] = "\20S*\0";
char   *msgi3;

main (argc, argv)
int     argc;
char   *argv[];
{
    char   *p;
    int     data,
            ttisflg,
            unamflg,
            themflg,
            count;

    errflg = ttisflg = 0;
    unamflg = themflg = 0;
    errlog = "uuerr.log";

 /* mjr - this part completely mine. original version did not */
 /* give a damn about arguments */
/* we try to remove compiled-in dependencies (ugh) */

    for (count = 1; count <= argc; count++) {
	if (argv[count][0] == '-') {
	    switch (argv[count][1]) {
		/* line name */
		case 'l': 
		case 'L': 
		    if (strlen (argv[count]) > 2) {
			ttynam = &argv[count][2];
			ttisflg++;
		    }
		    break;

		/* error log file (default is uuerr.log) */
		case 'e': 
		case 'E': 
		    if (strlen (argv[count]) > 2) {
			errlog = &argv[count][2];
		    }
		    break;

		/* this system's name */
		case 'h': 
		case 'H': 
		    if (strlen (argv[count]) > 2) {
			unamflg++;
			msgo0 = &argv[count][2];
		    }
		    break;

		/* the other systems name */
		case 's': 
		case 'S': 
		    if (strlen (argv[count]) > 2) {
			themflg++;
			msgi3 = &argv[count][2];
		    }
		    break;

		/* turn on debug */
		case 'x': 
		case 'X': 
		    debugmode = atoi (&argv[count][2]);
		    if (!debugmode)
			debugmode++;
		    break;

		/* options */
		case 'o': 
		case 'O': 
		    fprintf (stderr, "uuslave options: \n");
		    fprintf (stderr, "(mandatory) -ldevicename\n");
		    fprintf (stderr, "(mandatory) -hhostname\n");
		    fprintf (stderr, "(mandatory) -stheirname\n");
		    fprintf (stderr, "errlog      -efilename\n");
		    fprintf (stderr, "debug       -x#\n");
		    fprintf (stderr, "options     -o\n");
		    exit (-9);

		default: 
		    fprintf (stderr, "uuslave:invalid option %s\n", &argv[count][1]);
		    exit (-1);
	    }
	}
    }

    if (!ttisflg) {
	fprintf (stderr, "uuslave: ");
	fprintf (stderr, "MUST specify line with -l<line> flag\n");
	exit (-1);
    }

    if (!themflg) {
	fprintf (stderr, "uuslave: ");
	fprintf (stderr, "MUST name other system with -s<name> flag\n");
	fprintf (stderr, "(use the name they will try to login as)\n");
	exit (-1);
    }

    if (!unamflg) {
	fprintf (stderr, "uuslave: ");
	fprintf (stderr, "MUST specify this system name with -h<name> flag\n");
	fprintf (stderr, "(use the name they are trying to log in to)\n");
	exit (-1);
    }
    if (debugmode) {
	fprintf (stderr, "using line %s to communicate.\n", ttynam);
	fprintf (stderr, "expecting them to log in as %s.\n", msgi3);
	fprintf (stderr, "this system name is expected to be %s.\n", msgo0);
	fprintf (stderr, "error log file is %s.\n", errlog);
    }

 /* open the communications device/tty for read/write */
    if ((fdtty = open (ttynam, O_RDWR)) < 0) {
	printf ("Cannot open %s for read/write %d\n", ttynam, errno);
	exit (1);
    }
    if (debugmode > 3) {
	printf ("opened %s read/write\n", ttynam);
    }

 /* trap interrupt to close communications line */
    signal (SIGINT, sigint);

    while (1) {
	if (debugmode) {
	    puts ("restarting\n");
	}
	rseq = 0;
	wseq = 1;

    /* wait for EOT */
	while ((data = xgetc ()) == EOF || (data &= 0x7F) != EOT);
	if (debugmode > 6) {
	    puts ("got EOT\n");
	}

    /* output login request, verify uucp */
	write (fdtty, msgo0, sizeof (msgo0) - 1);
	if (instr (msgi0, sizeof (msgi0) - 1))
	    abort ();
	if (debugmode > 3) {
	    puts ("requested and got login\n");
	}

    /* output password request */
	write (fdtty, msgo1, sizeof (msgo1) - 1);
	if (instr (msgi1, sizeof (msgi1) - 1))
	    abort ();
	if (debugmode > 4) {
	    puts ("requested and got password\n");
	}

    /* output system here message, wait for response */
	write (fdtty, msgo2, sizeof (msgo2) - 1);
	if (instr (msgi2, sizeof (msgi2) - 1))
	    abort ();
	if (debugmode > 6) {
	    puts ("sent and got system here name\n");
	}

    /* output ok message, protocol request, wait for response */
	write (fdtty, msgo3, sizeof (msgo3) - 1);
	if (instr (msgi3, sizeof (msgi3) - 1))
	    abort ();
	if (debugmode > 6) {
	    puts ("sent OK\n");
	}

    /* output inital message, wait for response */
	ctlmsg ((INITA << 3) | wndsiz);
	if (inpkt () || tt != CTRL || xxx != INITA)
	    abort ();
	if (debugmode > 6) {
	    puts ("sent initial message\n");
	}

    /* output initb message, wait for response */
	ctlmsg ((INITB << 3) | segsiz);
	if (inpkt () || tt != CTRL || xxx != INITB)
	    abort ();
	if (debugmode > 6) {
	    puts ("sent initb message\n");
	}

    /* output initc message, wait for response */
	ctlmsg ((INITC << 3) | wndsiz);
	if (inpkt () || tt != CTRL || xxx != INITC)
	    abort ();
	if (debugmode > 6) {
	    puts ("sent initc message\n");
	}

    /* output initial acknowledge, wait for command */
	ackmsg ();
	while (1) {
	    if (inpkt () || tt != LNGDAT) {
		intf ("OVER EIGHT");
		abort ();
	    }
	    strcpy (msgbld, &msgi[6]);
	    while (strlen (&msgi[6]) == (segsiz + 1) * 32) {
		ackmsg ();
		if (inpkt () || tt != LNGDAT) {
		    intf ("OVER ABORT SEVEN");
		    abort ();
		}
		strcat (msgbld, &msgi[6]);
	    }
	    switch (msgbld[0]) {
		case 'S': 
		    sscanf (msgbld, "%s %s %s", cmnd, srcnam, dstnam);
		    p = dstnam;
		    if ((fddsk = creat (p, 0644)) >= 0) {
			ackmsg ();
			if (lngput ("SY", 2)) {
			    intf ("OVER NINE");
			    abort ();
			}
			do
			    if (inpkt ()) {
				intf ("OVER TEN");
				abort ();
			    }
			    else
				switch (tt) {
				    case LNGDAT: 
					write (fddsk, &msgi[6], ((segsiz + 1) * 32));
					ackmsg ();
					break;
				    case SHTDAT: 
					if (msgi[6] & 0x80) {
					    intf ("OVER ELEVEN");
					    if (debugmode) {
						puts ("short packet error");
					    }
					    abort ();
					}
					else {
					    if (msgi[6] != (segsiz + 1) * 32)
						write (fddsk, &msgi[7], (segsiz + 1) * 32 - msgi[6]);
					    ackmsg ();
					}
					break;
				    default: 
					intf ("OVER TWELVE");
					abort ();
				}
			while (tt != SHTDAT || msgi[6] != (segsiz + 1) * 32);
			close (fddsk);
			if (lngput ("CY", 2))
			    abort ();
		    }
		    else {
			ackmsg ();
			if (errflg) {
			    if (errfile = fopen (errlog, "a+")) {
				fprintf (errfile, "Cannot write %s errno%d\n", p, errno);
				fclose (errfile);
			    }
			}
			sprintf (dskbuf, "SN%d", errno);
			if (lngput (dskbuf, strlen (dskbuf)))
			    abort ();
		    }
		    break;
		case 'R': 
		    sscanf (msgbld, "%s %s %s", cmnd, srcnam, dstnam);
		    p = srcnam;
		    if ((fddsk = open (p, O_RDONLY)) >= 0) {
			ackmsg ();
			if (lngput ("RY", 2))
			    abort ();
			do
			    if ((count = read (fddsk, dskbuf, ((segsiz + 1) * 32))) == (segsiz + 1) * 32)
				if (lngput (dskbuf, (segsiz + 1) * 32))
				    abort ();
				else;
			    else
				if (shtput (dskbuf, count))
				    abort ();
			while (count);
			close (fddsk);
			do
			    if (inpkt ())
				abort ();
			while (tt != LNGDAT);
			ackmsg ();
		    }
		    else {
			ackmsg ();
			if (errflg) {
			    if (errfile = fopen (errlog, "a+")) {
				fprintf (errfile, "Cannot read file %s errno:%d\n", p, errno);
				fclose (errfile);
			    }
			}
			sprintf (dskbuf, "RN%d", errno);
			if (lngput (dskbuf, strlen (dskbuf)))
			    abort ();
		    }
		    break;
		case 'H': 
		    intf ("IN H CASE");
		    if (lngput ("HY", 2)) {
			intf ("OVER ABORT ONE");
			abort ();
		    }
		    if (inpkt () || tt != LNGDAT) {
			intf ("OVER ABORT TWO");
			abort ();
		    }
		    if (!strcmp (&msgi[6], "HY")) {
			ctlmsg (CLOSE << 3);
			do
			    if (inpkt ()) {
				intf ("OVER ABORT THREE");
				abort ();
			    }
			while (tt != CTRL && xxx != CLOSE);
			write (fdtty, msgo4, sizeof (msgo4) - 1);
			instr (msgo4, sizeof (msgo4) - 1);
		    }
		    intf ("OVER ABORT FIVE");
		    break;
	    }
	}
    }
}

intf (buffer)
register char  *buffer;
{
    int     fd;

 /* make entry in UUCP.DAT file */
    fd = open ("UUCP.DAT", O_RDWR);
    lseek (fd, (long) 0, 2);
    write (fd, buffer, strlen (buffer));
    close (fd);
}

xgetc () {

    char    data;

 /* get some stuff from acu/tty line */
    if (read (fdtty, &data, 1) > 0)
	return (data & 0xFF);
    return (EOF);
}

sigint () {
 /* interrupt - close acu/tty line */
    if (debugmode) {
	fprintf (stderr, "...interrupt...\n");
    }
    close (fdtty);
    exit (0);
}

zero (p, c)
char   *p;
int     c;
{
 /* zero a string */
    while (c--)
	*p++ = 0;
}

ackmsg () {
    int     cksm,
            index;

 /* looks like this writes a UUCp format ack message */
    msgo[0] = 020;
    msgo[1] = 9;
    msgo[4] = (CTRL << 6) | (RR << 3) | rseq;
    cksm = MAGIC - msgo[4];
    msgo[2] = cksm;
    msgo[3] = cksm >> 8;
    msgo[5] = msgo[1] ^ msgo[2] ^ msgo[3] ^ msgo[4];

    if (debugmode > 6) {
	printf ("T ");
	for (index = 0; index < 6; index++)
	    printf ("%03o ", msgo[index] & 0xFF);
	putchar ('\n');
    }

    write (fdtty, msgo, 6);
    rseq = (rseq + 1) & 7;
}

ctlmsg (byte)
char    byte;
{
    int     cksm,
            index;

    msgo[0] = 020;
    msgo[1] = 9;
    msgo[4] = (CTRL << 6) | byte;
    cksm = MAGIC - msgo[4];
    msgo[2] = cksm;
    msgo[3] = cksm >> 8;
    msgo[5] = msgo[1] ^ msgo[2] ^ msgo[3] ^ msgo[4];

    if (debugmode > 6) {
	printf ("T ");
	for (index = 0; index < 6; index++)
	    printf ("%03o ", msgo[index] & 0xFF);
	putchar ('\n');
    }

    write (fdtty, msgo, 6);
}

lngput (s, n)
char   *s;
int     n;
{
    int     cksm,
            index;

    zero (msgo, 256);
    msgo[0] = 020;
    msgo[1] = segsiz + 1;
    msgo[4] = (LNGDAT << 6) + (wseq << 3) + rseq;
    for (index = 0; index < (segsiz + 1) * 32; index++)
	msgo[6 + index] = 0;
    for (index = 0; index < n; index++)
	msgo[6 + index] = *(s + index);
    cksm = MAGIC - (chksum (&msgo[6], (segsiz + 1) * 32) ^ (0377 & msgo[4]));
    msgo[2] = cksm;
    msgo[3] = cksm >> 8;
    msgo[5] = msgo[1] ^ msgo[2] ^ msgo[3] ^ msgo[4];

    if (debugmode > 6) {
	printf ("T ");
	for (index = 0; index < (segsiz + 1) * 32 + 6; index++)
	    printf ("%03o ", msgo[index] & 0xFF);
	putchar ('\n');
    }

    do {
	write (fdtty, msgo, (segsiz + 1) * 32 + 6);
	if (inpkt ())
	    return (1);
    }
    while (tt != CTRL || xxx != RR || yyy != wseq);
    wseq = (wseq + 1) & 7;
    return (0);
}

shtput (s, n)
char   *s;
int     n;
{
    int     cksm,
            index;

    zero (msgo, 256);
    msgo[0] = 020;
    msgo[1] = segsiz + 1;
    msgo[4] = (SHTDAT << 6) + (wseq << 3) + rseq;
    for (index = 0; index < (segsiz + 1) * 32; index++)
	msgo[6 + index] = 0;
    msgo[6] = (segsiz + 1) * 32 - n;
    for (index = 0; index < n; index++)
	msgo[7 + index] = *(s + index);
    cksm = MAGIC - (chksum (&msgo[6], (segsiz + 1) * 32) ^ (0377 & msgo[4]));
    msgo[2] = cksm;
    msgo[3] = cksm >> 8;
    msgo[5] = msgo[1] ^ msgo[2] ^ msgo[3] ^ msgo[4];

    if (debugmode > 4) {
	printf ("T ");
	for (index = 0; index < (segsiz + 1) * 32 + 6; index++)
	    printf ("%03o ", msgo[index] & 0xFF);
	putchar ('\n');
    }

    do {
	write (fdtty, msgo, (segsiz + 1) * 32 + 6);
	if (inpkt ())
	    return (1);
    }
    while (tt != CTRL || xxx != RR || yyy != wseq);
    wseq = (wseq + 1) & 7;
    return (0);
}

instr (s, n)
char   *s;
int     n;
{
    int     data,
            count,
            i,
            j;

    count = 0;

    if (debugmode > 4) {
	printf ("Expecting ");
	for (i = 0; i < n; i++)
	    printf ("%03o ", *(s + i));
	printf ("\nR ");
    }

    while ((data = xgetc ()) != EOF) {
	msgi[count++] = data & 0x7F;

	if (debugmode > 4) {
	    printf ("%03o ", msgi[count - 1]);
	}

	if (count >= n) {
	    for (i = n - 1, j = count - 1; i >= 0; i--, j--)
		if (*(s + i) == '*' || *(s + i) != msgi[j])
		    break;
	    if (i < 0 || *(s + i) == '*') {

		if (debugmode > 4) {
		    putchar ('\n');
		}

		return (0);
	    }
	}
    }

    if (debugmode > 4) {
	putchar ('\n');
    }

    msgi[count] = 0;
    return (1);
}

inpkt () {
    int     data,
            count,
            need;

    count = 0;

    if (debugmode > 4) {
	printf ("R ");
    }

    while ((data = xgetc ()) != EOF) {

	if (debugmode > 4) {
	    printf ("%03o ", data & 0xFF);
	}

	switch (count) {
	    case 0: 
		if (data == 020)
		    msgi[count++] = 020;
		break;
	    case 1: 
		msgi[count++] = data;
		if (data == 9)
		    need = 4;
		else
		    need = 32 * data + 4;
		break;
	    case 4: 
		tt = (data >> 6) & 3;
		xxx = (data >> 3) & 7;
		yyy = data & 7;
	    default: 
		msgi[count++] = data;
		if (!--need) {
		    if (debugmode > 4) {
			putchar ('\n');
		    }
		    return (0);
		}
		break;
	}
    }
    if (debugmode > 4) {
	putchar ('\n');
    }
    return (1);
}

chksum (s, n)
register char  *s;
register int    n;
{
    register short  sum;
    register unsigned short t;
    register short  x;

    sum = -1;
    x = 0;
    do {
	if (sum < 0) {
	    sum <<= 1;
	    sum++;
	}
	else
	    sum <<= 1;
	t = sum;
	sum += *s++ & 0377;
	x += sum ^ n;
	if ((unsigned) sum <= t)
	    sum ^= x;
    }
    while (--n > 0);
    return (sum);
}

int
        abort () {
            write (fdtty, msgo5, sizeof (msgo5) - 1);
    if (debugmode) {
	fprintf (stderr, "%s\n", msgo5);
    }
    exit (-1);
}
