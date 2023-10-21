#

/*
 * upload - read high speed serial line input to standard output
 *
 * % upload [timeout] >file
 *
 *   timeout: number of seconds of no data which will terminate xfer
 *             default 5.  Min 1, max 300.
 *
 * -ghg 07/04/77
 */

#define BUFSIZ 8000
#define ENXIO 6  /* error returned for device in use */

struct buf {
	char *in;	/* addr of next input char (rel to start of struct) */
	char *lim;	/* addr+1 of end of buffer (releative to struct) */
	char *out;	/* addr of next output char (relative to struct) */
	char bufr[BUFSIZ]; /* circular buffer for read */
} b;

char dbuf[512];		/* disk output buffer */
char *indev "/dev/xx?";	/* dev name of high speed input */
int tout	5;	/* default timeout for no data */
int fout;		/* printf redirected to error message output */
int errno;		/* error returned on last system call */
int abtf;		/* abort - hangup from xx dev on bad data */

main(argc, argv)
char *argv[];
{
	register char *bp;
	int timeout, fd;
	char c;
	extern hung();

	fout = 2;
	if(argc > 1)
		tout = atoi(argv[1]);
	if(tout <= 0 || tout > 300) {
		printf("upload: bad timeout value, must be between 1 and 300\n");
		flush();
		exit();
	}
	timeout = 0;
	bp = dbuf;
	b.out = b.in = 6;
	b.lim = &b.bufr[BUFSIZ] - &b.in;

	if((indev[7]=ttyn(0)) == 'x') {
		printf("upload: no tty defined on standard input\n");
		flush();
		exit();
	}
	if((fd = open(indev,0)) < 0) {
		if(errno == ENXIO)
			printf("upload: another upload or other xfer in progress\n");
		else
			printf("upload: this terminal not capable of uploading\n");
		flush();
		exit();
	}
	signal(1, &hung);
	read(fd,&b,1);	/* start realtime read to circular buffer */
	putchar(021); /* send control-q to signal start of read */
	flush();
loop:
	sleep(1);
	while(b.in != b.out) {
		timeout = 0;
		if (abtf) {
			b.in = b.out = 6;	/* eat buffer on error */
			break;
		}
		*bp++ = b.bufr[b.out-6];
		b.out++;
		if(bp == &dbuf[512]) {
			bp = dbuf;
			write(1,dbuf,512);
		}
		if(b.out == b.lim)
			b.out = 6;
	}
	if(timeout++ >= tout) {
		write(1, dbuf, (bp - dbuf));
		putchar(023); /* send a control-s when done */
		if(abtf)
		  printf("upload: Buffer overrun or character frame error - try again\n");
		flush();
		exit();
	}
	goto loop;
}

hung()
{
	signal(1,1);
	abtf++;
}
