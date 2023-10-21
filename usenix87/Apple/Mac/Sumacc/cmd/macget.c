#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <sgtty.h>

#ifdef NO_RENAME
#define rename(old, new)	link(old, new); unlink(old)
#endif

/* Mac time of 00:00:00 GMT, Jan 1, 1970 */
#define TIMEDIFF 0x7c25b080

#define RECORDBYTES 132
#define DATABYTES 128
#define NAMEBYTES 63

#define RETRIES 10
#define SOHTIMO 10
#define LINTIMO 20
#define CHRTIMO 2

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

char tmpname[16];

int lastack;
char buf[DATABYTES];

/*
 * macget -- receive file from macintosh using xmodem protocol
 * Dave Johnson, Brown University Computer Science
 *
 * (c) 1984 Brown University 
 * may be used but not sold without permission
 *
 * created ddj 5/22/84 
 * revised ddj 6/29/84 -- added [-rdu] options
 * revised ddj 7/16/84 -- protocol changes for MacTerminal Beta Version 0.5X
 * revised ddj 7/31/84 -- pre-4.2 signal bugs fixed in timedout()
 */
char usage[] = "usage: \"macget [-o] [-rdu] [filename]\"\n";

main(ac, av)
char **av;
{
	char *name;

	mode = FULL;
	name = "";
	ac--; av++;
	while (ac) {
		if (av[0][0] == '-') {
			switch (av[0][1]) {
			case 'r':
				mode = RSRC;
				break;
			case 'd':
				mode = DATA;
				break;
			case 'u':
				mode = TEXT;
				break;
			case 'o':
				pre_beta++;
				break;
			default:
				fprintf(stderr, usage);
				exit(1);
			}
		}
		else {
			name = av[0];
		}
		ac--; av++;
	}

	setup_tty();
	if (send_sync() == ACK) {
		txtmode = 0;
		recv_hdr(name);
		if (mode == TEXT) txtmode++;
		recv_file(files.f_data, mh.m_datalen, 1);
		txtmode = 0;
		recv_file(files.f_rsrc, mh.m_rsrclen, 0);
	}
	reset_tty();
}

recv_hdr(name)
char *name;
{
	long get4();
	int n;
	FILE *fp;
	char *np;

	strcpy(tmpname, "#machdrXXXXXX");
	mktemp(tmpname);
	recv_file(tmpname, (long)DATABYTES, 1);

	fp = fopen(tmpname, "r");
	if (fp == NULL) {
		perror("temp file");
		cleanup(-1);
	}
	fread(buf, 1, DATABYTES, fp);
	fclose(fp);

	if (name && *name) {
		n = strlen(name);
		if (n > NAMEBYTES) n = NAMEBYTES;
		strncpy(mh.m_name, name, n);
		mh.m_name[n] = '\0';
	}
	else {
		n = buf[H_NLENOFF] & BYTEMASK;
		if (n > NAMEBYTES) n = NAMEBYTES;
		strncpy(mh.m_name, buf + H_NAMEOFF, n);
		mh.m_name[n] = '\0';
	}
	for (np = mh.m_name; *np; np++)
		if (*np == ' ') *np = '_';

	if (mode == FULL) {
		sprintf(files.f_info, "%s.info", mh.m_name);
		rename(tmpname, files.f_info);
		tmpname[0] = '\0';
		sprintf(files.f_data, "%s.data", mh.m_name);
		sprintf(files.f_rsrc, "%s.rsrc", mh.m_name);
	}
	else {
		unlink(tmpname);
		tmpname[0] = '\0';
		switch (mode) {
		case RSRC:
			sprintf(files.f_data, "/dev/null");
			sprintf(files.f_rsrc, "%s.rsrc", mh.m_name);
			break;

		case DATA:
			sprintf(files.f_data, "%s.data", mh.m_name);
			sprintf(files.f_rsrc, "/dev/null");
			break;

		case TEXT:
			sprintf(files.f_data, "%s.text", mh.m_name);
			sprintf(files.f_rsrc, "/dev/null");
			break;
		}
	}

	strncpy(mh.m_type, buf + H_TYPEOFF, 4);
	strncpy(mh.m_author, buf + H_AUTHOFF, 4);
	if (pre_beta) {
		mh.m_datalen = get4(buf + H_OLD_DLENOFF);
		mh.m_rsrclen = get4(buf + H_OLD_RLENOFF);
	}
	else {
		mh.m_datalen = get4(buf + H_DLENOFF);
		mh.m_rsrclen = get4(buf + H_RLENOFF);
		mh.m_createtime = get4(buf + H_CTIMOFF);
		mh.m_modifytime = get4(buf + H_MTIMOFF);
	}
}

recv_file(fname, bytes, more)
char *fname;
long bytes;
int more;
{
	register int status, n;
	FILE *outf;
	int naks = 0;

	lastack = 0;
	outf = fopen(fname, "w");
	if (outf == NULL) {
		perror(fname);
		cleanup(-1);
	}
	for (;;) {
		status = rec_read(buf, DATABYTES);
		switch (status) {
		case EOT:
			if (!pre_beta)
				tputc(ACK);
			if (more)
				tputc(NAK);
			fclose(outf);
			return;
		case ACK:
			tputc(ACK);
			naks = 0;
			n = (bytes > DATABYTES) ? DATABYTES : bytes;
			bytes -= n;
			fwrite(buf, n, 1, outf);
			break;
		case DUP:
			tputc(ACK);
			naks = 0;
			break;
		case NAK:
			purge(CHRTIMO);
			if (naks++ < RETRIES) {
				tputc(NAK);
				break;
			}
			/* fall through */
		case CAN:
			tputc(CAN);
			fclose(outf);
			/* unlink fname? */
			cleanup(-1);
			/* NOTREACHED */
		}
	}
}

send_sync()
{
	int c;

	for (;;) {
		c = tgetc(60);
		switch (c) {
		case ESC:
			break;
		case CAN:
		case EOT:
		case TMO:
			return c;
		default:
			continue;
		}
		c = tgetc(1);
		if (c != 'a')
			continue;
		tputc(ACK);
		return ACK;
	}
}

rec_read(buf, recsize)
char buf[];
int recsize;
{
	int c, rec, rec_bar, cksum;

	c = tgetc(SOHTIMO);
	switch (c) {
	case TMO:
	default:
		return NAK;
	case EOT:
		return EOT;
	case CAN:
		return CAN;
	case SOH:
		/* read header */
		rec = tgetc(CHRTIMO);
		if (rec == TMO)
			return NAK;
		rec_bar = tgetc(CHRTIMO);
		if (rec_bar == TMO)
			return NAK;

		/* check header */
		if (rec != MAXRECNO - rec_bar) return NAK;

		/* fill buffer */
		cksum = tgetrec(buf, recsize, LINTIMO);
		if (cksum == TMO)
			return NAK;

		/* get checksum */
		c = tgetc(CHRTIMO);
		if (c == TMO)
			return NAK;
		if (c != (cksum & BYTEMASK))
			return NAK;

		/* check record number */
		if (rec == lastack)
			return DUP;
		if (rec != ((lastack + 1) & MAXRECNO))
			return CAN;
		else {
			lastack = rec;
			return ACK;
		}
	}
	/* NOTREACHED */
}

purge(timeout)
int timeout;
{
	int c;

	do {
		c = tgetc(timeout);
	} while (c != TMO);
}

static int ttyfd;
static FILE *ttyf;
jmp_buf timobuf;

tgetrec(buf, count, timeout)
char *buf;
int count, timeout;
{
	char *bp;
	int i, cksum;

	if (setjmp(timobuf))
		return TMO;
	
	alarm(timeout);
	i = fread(buf, 1, count, ttyf);
	alarm(0);
	if (i != count)
		return TMO;
	
	cksum = 0;
	bp = buf;
	for (i = 0; i < count; bp++, i++) {
		cksum += *bp;
		if (txtmode && *bp == '\r')
			*bp = '\n';
	}
	return cksum;
}

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
	sleep(2);	/* should wait for output to drain */
	ioctl(ttyfd, TIOCSETP, &otty);
}

cleanup(sig)
int sig;
{
	if (tmpname[0] != '\0')
		unlink(tmpname);
	reset_tty();
	exit(sig);
}

long
get4(bp)
char *bp;
{
	register int i;
	long value = 0;

	for (i = 0; i < 4; i++) {
		value <<= 8;
		value |= (*bp & BYTEMASK);
		bp++;
	}
	return value;
}
