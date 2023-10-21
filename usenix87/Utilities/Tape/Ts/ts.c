/*
 *      ts - This program outputs a file/record description of a tape's
 *      structure.
 *
 *      Exits:  0 - normal.
 *              1 - usage error.
 *              2 - tape read error.
 *              3 - SIGINT received.
 *
 *      Copyright (C) 1983, 1984 Lyle McElhaney
 *      Permission to copy for non-commercial use granted under condition
 *      that this notice remains intact in the copy.
 *
 *      Address: 2489 W. Ridge Rd., Littleton, CO 80120
 *      ....denelcor!lmc
 */
#include <stdio.h>
#include <signal.h>

#define MAXBUF=32767
/*      This can be whatever maximum the machine/os will take.
 *      16000 is allowed on an 11/44 running System 3 Unix (tm).
 */

#ifndef YES
#define YES 1
#define NO 0
#endif

char tape[]="/dev/rmt0\0\0";
char buff[MAXBUF];
FILE *tapedev;
long recno, nrec, fsize, filen;
extern void sig();

main (argc, argv)
    int argc;
    char **argv;
{
    char *p;
    int n;

    argv++;
    if (argc > 1) {
	p = *argv;
	if (*p == '-') {
	    p++;
	    while (*p != '\0') {
		switch (*p++) {
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '0':
		    tape[8] = *(p-1);
		    break;
		case '1':
		    tape[8] = '1';
		    if (*p >= '0' && *p <= '5')
			tape[9] = *p++;
		    break;
		default:
Usage:
		    fprintf (stderr, "Usage: ts [-0...15]\n");
		    exit (2);
		}
	    }
	    argc--;
	    argv++;
	}
    else goto Usage;
    }

    signal (SIGINT, sig);
    if ((tapedev = fopen (tape, "r")) == NULL) {
	fprintf (stderr, "ts: cannot open %s\n", tape);
	exit (2);
    }

    filen = 0;
    for (;;) {
	recno = nrec = 0;
	for (;;) {
	    n = read (fileno (tapedev), buff, sizeof buff);
	    if (n > 0) {
		if (recno == 0) {
		    printf ("File %ld:\n", filen);
		    fsize = n;
		}
		if (n != fsize) {
		    printf ("    Records %ld through %ld (%ld records), length = %ld bytes\n",
			    nrec, recno - 1, recno - nrec, fsize);
		    fsize = n;
		    nrec = recno;
		}
		recno ++;
	    } else goto Out;
	}
Out:    if (n == 0) {
	    if (recno != 0)
		printf ("    Records %ld through %ld (%ld records), length = %ld bytes\n",
		    nrec, recno - 1, recno - nrec, fsize);
	} else {
	    printf ("Tape read error %d in record %ld\n", n, recno);
	    exit (1);
	}
	if (recno == 0) break;
	filen ++;
    }
    printf ("End of tape reached.\n");
    fclose (tapedev);
    exit (0);
}

void sig ()
{
    printf ("    Records %ld through %ld (%ld records), length = %ld bytes\n",
	    nrec, recno-1, recno - nrec, fsize);
    printf ("ts: aborted by interrupt at that point.\n");
    close (tapedev);
    exit (3);
}
