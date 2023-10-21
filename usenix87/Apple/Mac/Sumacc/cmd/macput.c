/*

Here is the source for the current incarnation of macput . . . .
It is now compatible with the 0.5 Beta version of MacTerminal, and
in case you still need to use the -0.15X version, there's the "-o"
option to provide compatibility.  Some minor bug fixes have been
made, as well as a couple of changes for portability to non-4.2 
systems.  Please pass any improvements/bug fixes on to me, and
if you know of any good protocols for use on a flow-controlled
line, let me know.

	Dave Johnson
	ddj%brown@csnet-relay.arpa
	Brown University Computer Science

*/
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <sgtty.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timeb.h>

/* Mac time of 00:00:00 GMT, Jan 1, 1970 */
#define TIMEDIFF 0x7c25b080

#define RECORDBYTES 132
#define DATABYTES 128
#define NAMEBYTES 63

#define RETRIES 10
#define ACKTIMO 10

#define MAXRECNO 0xff
#define BYTEMASK 0xff

#define TMO -1
#define DUP '\000'
#define SOH '\001'
#define EOT '\004'
#define ACK '\006'
#define NAK '\025'
#define CAN '\030'
#define EEF '\032'
#define ESC '\033'

#define H_NLENOFF 1
#define H_NAMEOFF 2
/* 65 <-> 80 is the FInfo structure */
#define H_TYPEOFF 65
#define H_AUTHOFF 69

#define H_LOCKOFF 81
#define H_DLENOFF 83
#define H_RLENOFF 87
#define H_CTIMOFF 91
#define H_MTIMOFF 95

#define H_OLD_DLENOFF 81
#define H_OLD_RLENOFF 85

#define TEXT 0
#define DATA 1
#define RSRC 2
#define FULL 3

int mode, txtmode;
int pre_beta;	/* -o flag; for compatibility with MacTerminal Version -0.15X */

struct macheader {
	char m_name[NAMEBYTES+1];
	char m_type[4];
	char m_author[4];
	long m_datalen;
	long m_rsrclen;
	long m_createtime;
	long m_modifytime;
} mh;

struct filenames {
	char f_info[256];
	char f_data[256];
	char f_rsrc[256];
} files;

int recno;
char buf[DATABYTES];

/*
 * macput -- send file to macintosh using xmodem protocol
 * Dave Johnson, Brown University Computer Science
 *
 * (c) 1984 Brown University 
 * may be used but not sold without permission
 *
 * created ddj 6/17/84 
 * revised ddj 7/16/84 -- protocol changes for MacTerminal Beta Version 0.5X
 * revised ddj 7/31/84 -- pre-4.2 signal bugs fixed in timedout()
 * revised ddj 7/31/84 -- fixed timeout problem in initial handshake
 */
char usage[] =
    "usage: \"macput [-o] [-rdu] [-t type] [-a author] [-n name] filename\"\n";

main(ac, av)
char **av;
{
	int n;
	char *filename;

	if (ac == 1) {
		fprintf(stderr, usage);
		exit(1);
	}

	mode = FULL;
	ac--; av++;
	while (ac) {
		if (av[0][0] == '-') {
			switch (av[0][1]) {
			case 'r':
				mode = RSRC;
				strncpy(mh.m_type, "APPL", 4);
				strncpy(mh.m_author, "CCOM", 4);
				break;
			case 'u':
				mode = TEXT;
				strncpy(mh.m_type, "TEXT", 4);
				strncpy(mh.m_author, "MACA", 4);
				break;
			case 'd':
				mode = DATA;
				strncpy(mh.m_type, "TEXT", 4);
				strncpy(mh.m_author, "????", 4);
				break;
			case 'n':
				if (ac > 1) {
					ac--; av++;
					n = strlen(av[0]);
					if (n > NAMEBYTES) n = NAMEBYTES;
					strncpy(mh.m_name, av[0], n);
					mh.m_name[n] = '\0';
					break;
				}
				else goto bad_usage;
			case 't':
				if (ac > 1) {
					ac--; av++;
					strncpy(mh.m_type, av[0], 4);
					break;
				}
				else goto bad_usage;
			case 'a':
				if (ac > 1) {
					ac--; av++;
					strncpy(mh.m_author, av[0], 4);
					break;
				}
				else goto bad_usage;
			case 'o':
				pre_beta++;
				break;
			default:
bad_usage:
				fprintf(stderr, usage);
				exit(1);
			}
		}
		else {
			filename = av[0];
		}
		ac--; av++;
	}

	setup_tty();
	find_files(filename, mode);
	if (mode != FULL)
		forge_info();

	if (send_sync() == ACK) {
		txtmode = 0;
		sleep(1);
		send_file(files.f_info, 1);

		if (mode != FULL)
			unlink(files.f_info);

		if (mode == TEXT) txtmode++;
		send_file(files.f_data, 1);

		txtmode = 0;
		send_file(files.f_rsrc, 0);
	}
	reset_tty();
}

find_files(filename, mode)
char *filename;
{
	int n, tdiff;
	struct tm *tp;
	struct timeb tbuf;
	struct stat stbuf;

	sprintf(files.f_data, "%s.data", filename);
	sprintf(files.f_rsrc, "%s.rsrc", filename);

	if (mode == FULL) {
		sprintf(files.f_info, "%s.info", filename);
		if (stat(files.f_info, &stbuf) != 0) {
			perror(files.f_info);
			cleanup(-1);
		}
		return;
	}
	else {
		strcpy(files.f_info, "#machdrXXXXXX");
		mktemp(files.f_info);
	}

	if (mode == RSRC) {
		strcpy(files.f_data, "/dev/null");
		if (stat(files.f_rsrc, &stbuf) != 0) {
			strcpy(files.f_rsrc, filename);
			if (stat(files.f_rsrc, &stbuf) != 0) {
				perror(files.f_rsrc);
				cleanup(-1);
			}
		}
		mh.m_datalen = 0;
		mh.m_rsrclen = stbuf.st_size;
	}
	else {
		strcpy(files.f_rsrc, "/dev/null");
		if (stat(files.f_data, &stbuf) != 0) {
			sprintf(files.f_data, "%s.text", filename);
			if (stat(files.f_data, &stbuf) != 0) {
				strcpy(files.f_data, filename);
				if (stat(files.f_data, &stbuf) != 0) {
					perror(files.f_data);
					cleanup(-1);
				}
			}
		}
		mh.m_datalen = stbuf.st_size;
		mh.m_rsrclen = 0;
	}

	if (!pre_beta) {
		ftime(&tbuf);
		tp = localtime(&tbuf.time);
		tdiff = TIMEDIFF - tbuf.timezone * 60;
		if (tp->tm_isdst)
			tdiff += 60 * 60;
		mh.m_createtime = stbuf.st_mtime + tdiff;
		mh.m_modifytime = stbuf.st_mtime + tdiff;
	}

	if (mh.m_name[0] == '\0') {
		n = strlen(filename);
		if (n > NAMEBYTES) n = NAMEBYTES;
		strncpy(mh.m_name, filename, n);
		mh.m_name[n] = '\0';
	}
}

forge_info()
{
	int n;
	char *np;
	FILE *fp;

	for (np = mh.m_name; *np; np++)
		if (*np == '_') *np = ' ';

	buf[H_NLENOFF] = n = np - mh.m_name;
	strncpy(buf + H_NAMEOFF, mh.m_name, n);
	strncpy(buf + H_TYPEOFF, mh.m_type, 4);
	strncpy(buf + H_AUTHOFF, mh.m_author, 4);
	if (pre_beta) {
		put4(buf + H_OLD_DLENOFF, mh.m_datalen);
		put4(buf + H_OLD_RLENOFF, mh.m_rsrclen);
	}
	else {
		put4(buf + H_DLENOFF, mh.m_datalen);
		put4(buf + H_RLENOFF, mh.m_rsrclen);
		put4(buf + H_CTIMOFF, mh.m_createtime);
		put4(buf + H_MTIMOFF, mh.m_modifytime);
	}
	fp = fopen(files.f_info, "w");
	if (fp == NULL) {
		perror("temp file");
		cleanup(-1);
	}
	fwrite(buf, 1, DATABYTES, fp);
	fclose(fp);
}

send_sync()
{
	int c, i;

	for (i = 0; i < 3; i++) {
		tputc(ESC);
		tputc('a');
		while ((c = tgetc(ACKTIMO)) != TMO) {
			switch (c) {
			case CAN:
			case EOT:
			case ACK:
				return c;
			default:
				continue;
			}
		}
		fprintf(stderr, "starting handshake timeout\r\n");
	}
	fprintf(stderr, "giving up\r\n");
	return CAN;
}

send_file(fname, more)
char *fname;
int more;
{
	register int status, i, n;
	FILE *inf;

	inf = fopen(fname, "r");
	if (inf == NULL) {
		perror(fname);
		cleanup(-1);
	}
	recno = 1;
	for (;;) {
		n = fread(buf, 1, DATABYTES, inf);
		if (n > 0) {
			for (i = 0; i < RETRIES; i++) {
				send_rec(buf, DATABYTES);
				status = tgetc(ACKTIMO);
				if (status != NAK)
					break;
			} 
			if (status == NAK || status == CAN) {
				fclose(inf);
				cleanup(-1);
				/* NOTREACHED */
			}
		}
		if (n < DATABYTES) {
			tputc(EOT);
			if (!pre_beta) {
				status = tgetc(ACKTIMO);
			}
			if (more) {
				status = tgetc(ACKTIMO);
			}
			return;
		}
		recno++;
		recno &= MAXRECNO;
	}
}

send_rec(buf, recsize)
char buf[];
int recsize;
{
	int i, cksum;
	char *bp;

	cksum = 0;
	bp = buf;
	for (i = 0; i < recsize; i++, bp++) {
		if (txtmode && *bp == '\n')
			*bp = '\r';
		cksum += *bp;
	}

	tputc(SOH);
	tputc((char)recno);
	tputc((char)(MAXRECNO - recno));
	tputrec(buf, recsize);
	tputc((char)cksum);
}

static int ttyfd;
static FILE *ttyf;
static jmp_buf timobuf;

tgetc(timeout)
int timeout;
{
	int c;

	if (setjmp(timobuf))
		return TMO;

	alarm(timeout);
	c = getc(ttyf);
	alarm(0);

	if (c == -1)	/* probably hung up or logged off */
		return EOT;
	else
		return c & BYTEMASK;
}

tputrec(buf, count)
char *buf;
int count;
{
	write(ttyfd, buf, count);
}

tputc(c)
char c;
{
	write(ttyfd, &c, 1);
}

timedout()
{
	signal(SIGALRM, timedout);	/* for pre-4.2 systems */
	longjmp(timobuf, 1);
}

static struct sgttyb otty, ntty;
/* should turn messages off */

setup_tty()
{
	int cleanup();
	int timedout();

	ttyf = stdin;
	ttyfd = fileno(stdin);
	ioctl(ttyfd, TIOCGETP, &otty);
	signal(SIGHUP, cleanup);
	signal(SIGINT, cleanup);
	signal(SIGQUIT, cleanup);
	signal(SIGTERM, cleanup);
	signal(SIGALRM, timedout);
	ntty = otty;
	ntty.sg_flags = RAW|ANYP;
	ioctl(ttyfd, TIOCSETP, &ntty);
}

reset_tty()
{
	if (ttyf != NULL) {
		sleep(2);	/* should wait for output to drain */
		ioctl(ttyfd, TIOCSETP, &otty);
	}
}

cleanup(sig)
int sig;
{
	reset_tty();
	exit(sig);
}

put4(bp, value)
char *bp;
long value;
{
	register int i, c;

	for (i = 0; i < 4; i++) {
		c = (value >> 24) & BYTEMASK;
		value <<= 8;
		*bp++ = c;
	}
}
