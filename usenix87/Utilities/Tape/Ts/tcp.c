/*
 *      tcp - this routine runs in input (-i) and output (-o) modes. On
 *      input, it reads a tape's contents (out to the double tape-marks)
 *      and writes that data to files in a the current directory, along
 *      with a file of data which will allow reconstruction of the tape
 *      in the output mode.
 *
 *      Copyright (C) 1983, 1984 Lyle McElhaney
 *      Permission to copy for non-commercial use granted under condition
 *      that this notice remains intact in the copy.
 *
 *      Address: 2489 W. Ridge Rd., Littleton, CO 80120
 *      ....denelcor!lmc
 *
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#ifdef MTIO
#include <sys/mtio.h>
#endif
#define MAXBUF 32768            /* maximum tape physical record size */
#ifndef YES
#define NO 0
#define YES 1
#endif
#define NYU 2

extern char *index();
extern int errno;
char nrtape[]="/dev/nrmt8\0\0";
char tape[]="/dev/rmt8\0\0";
char ffh[]="tcp,f";      /* headers for temp filenames */
char nfh[]="tcp,n";
char format[]="%s%02d%05d"; /* format of filenames. args: [fcn]fh, tapen, nfile */
#define FMTSIZ 13       /* limit on size of filenames created + 1 */
char ffile[FMTSIZ], cfile[FMTSIZ];
char buff[MAXBUF];
char buf[10];
int nfile;
int tapen=0;
int inmode=NYU;         /* input mode - not yet set. User must choose. */
FILE *tapedev, *filedev, *ctldev;
long recno, nrec, fsize, filen;
#ifdef MTIO
int convmode=NO;        /* conversational mode default to no */
struct mtop mtop;
#endif

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
#ifdef MTIO
		case 'c':
			convmode = YES;
			break;
#endif
		case 'i':
			if (inmode != NYU) goto Usage;
			inmode = YES;
			break;
		case 'o':
			if (inmode != NYU) goto Usage;
			inmode = NO;
			break;
		case 'n':
			tapen = atoi (p);
			break;
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '0':
		    tape[8] = nrtape[9] = *(p-1);
		    break;
		case '1':
		    tape[8] = nrtape[9] = '1';
		    if (*p >= '0' && *p <= '5')
			tape[9] = nrtape[10] = *p++;
		    break;
		default:
Usage:
		    fprintf (stderr, "Usage: tcp [-i] [-o] [-nxx] [-1...15]\n");
		    exit (2);
		}
	    }
	    argc--;
	    argv++;
	}
    } else goto Usage;
    if (inmode == NYU) goto Usage;

    if (inmode) {
/*
 *      input mode - read tape into files.
 */
	sprintf (cfile, format, nfh, tapen, 0);
	if ((ctldev = fopen (cfile, "w+")) == NULL) {
	    fprintf (stderr, "tcp: cannot open %s\n", cfile);
	    exit (2);
	}
	if ((tapedev = fopen (tape, "r")) == NULL) {
	    fprintf (stderr, "tcp: cannot open %s\n", tape);
	    exit (2);
	}
	filen = 0;
	for (;;) {
	    recno = nrec = 0;
reread:     while ((n = read (fileno (tapedev), buff, sizeof buff)) > 0) {
		if (recno == 0) {
		    fsize = n;
		    sprintf (ffile, format, ffh, tapen, filen);
		    if ((filedev = fopen (ffile, "w+")) == NULL) {
			fprintf (stderr, "tcp: cannot open %s\n", ffile);
			exit (2);
		    }
		}
		if (write (fileno (filedev), buff, n) <= 0) {
		    fprintf (stderr, "tcp: file write error #%d in %s\n",
			errno, ffile);
		    exit (2);
		}
		if (fsize != n) {
			fprintf (ctldev, "%d,%d\n", recno - nrec, fsize);
			fsize = n;
			nrec = recno;
		}
		recno ++;
	    }
	    if (n == 0)
		if (recno == 0)
		    break;
		else {
		    fprintf (ctldev, "%d,%d\n0,0\n", recno - nrec, fsize);
		    filen ++;
		    fclose (filedev);
		}
	    else {
		printf ("Tape read error %d in record %ld\n", n, recno);
#ifdef MTIO
		for (;;) {
		    if (convmode) {
			printf ("Abort, Retry, or Ignore? ");
			if (gets (buf) != NULL) {
			    if (*buf == 'R' || *buf == 'r') {
				mtop.mt_count = 1;
				mtop.mt_op = MTBSR;
				if (ioctl (fileno (tapedev), MTIOCTOP, &mtop) < 0) {
				    printf ("error %d in ioctl; ignoring prev error.\n", errno);
				    recno ++;
				}
				goto reread;
			    } else if (*buf == 'I' || *buf == 'i') {
				recno ++;
				goto reread;
			    } else if (*buf == 'A' || *buf == 'a')
				exit (1);
			}
		    } else
			exit (1);
		}
#else
	    exit (1);
#endif  MTIO
	    }
	}
	fclose (tapedev);
	fprintf (ctldev, "0,1\n");
	fclose (ctldev);
    } else {
/*
 *      output mode - copy files back out to tape.
 */
	sprintf (cfile, format, nfh, tapen, 0);
	if ((ctldev = fopen (cfile, "r")) == NULL) {
	    fprintf (stderr, "tcp: cannot open %s\n", cfile);
	    exit (2);
	}
	recno = 0;
	for (;;) {
	    if (fgets (buf, sizeof(buf), ctldev) == NULL) {
		fprintf (stderr, "tcp: file %s malformed.\n", cfile);
		exit (2);
	    }
	    nrec = atoi (buf);
	    n = atoi (index (buf, ',') + 1);
	    if (nrec != 0) {
		if (recno == 0) {
		    if ((tapedev = fopen (nrtape, "a")) == NULL) {
			fprintf (stderr, "tcp: cannot open %s\n", nrtape);
			exit (2);
		    }
		    sprintf (ffile, format, ffh, tapen, filen);
		    if ((filedev = fopen (ffile, "r")) == NULL) {
			fprintf (stderr, "tcp: cannot open %s\n", ffile);
			exit (2);
		    }
		}
		for (; nrec > 0; nrec--) {
		    if (read (fileno (filedev), buff, n) <= 0) {
			fprintf (stderr, "tcp: file read error #%d in %s\n",
			    errno, ffile);
			exit (2);
		    }
		    if (write (fileno (tapedev), buff, n) <= 0) {
			fprintf (stderr, "tcp: tape write error #%d in %s\n",
			    errno, nrtape);
			exit (2);
		    }
		    recno ++;
		}
	    } else if (n == 0) {
		fclose (filedev);
		fclose (tapedev);
		filen ++;
		recno = 0;
	    } else
		break;
	}
	fclose (ctldev);
    }
    exit (0);
}
