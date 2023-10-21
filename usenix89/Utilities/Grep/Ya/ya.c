#include <stdio.h>
#include <string.h>
main(argc,argv)	/* search string filename[s] */
int argc;
char **argv;
{
	/*
	   search searches for instances of an arbitrary string
	   given as the first argument in the files named as
	   second, third,... arguments.  The search string may
	   be completely arbitrary (e.g., contain newlines,
	   non-ASCII characters) except that it must be null
	   terminated (so a string containing nulls cannot be
	   searched for).  Matches are reported on stdout
	   with byte offset in file and line number in file,
	   in the context of either the current line (if the
	   context is text) or of the 512-byte block surrounding
	   the beginning of the match (if the context is binary).
	   Failure to match is reported on stderr and the exit
	   value equals the number of files in which there was
	   no match.
	*/

	char *malloc();
	register char *rdb;	 /* pointer to circular read buffer */
	int irdl;		/* length of read buffer */
	char *rdp;		/* "starting" location in read buffer */
	register char *rde;	/* end of read buffer */
	register char *pat;	/* start of string */
	register char *pp;	/* pointer inside string */
	register char *bp;	/* pointer inside buffer */
	register int i;
	register int ifile;	/* current file arg */
	register long ibyt;
	register int ilin;
	int matched;
	int istex=0;
	int nlpat=0;
	void show();
	void perror();
	FILE *ifp = NULL;	/* input file pointer */
	if (argc <= 2) {
		(void) fprintf(stderr,
			"Usage: %s arbitrary-fixed-string file[s]\n",argv[0]);
		return -1;
	}
	pat = argv[1];
	irdl = strlen(pat);
	if (irdl == 0) {
		(void) fprintf(stderr,"%s: zero length string to match\n",
			argv[0]); return -2;
	}
	for (pp=pat; pp<pat+irdl; pp++) if (*pp == '\n') nlpat++;
	rdb = malloc((unsigned)irdl);
	if (!rdb) {
		(void) fprintf(stderr,"%s: cannot allocate enough memory\n",
			argv[0]);
		return -3;
	}
	rde = rdb + irdl;
	for (ifile=2; ifile<argc; ifile++) {	/* for each file */
		if (!(ifp = fopen(argv[ifile],"r"))) {
			perror(argv[ifile]);
			istex++;
			continue;
		}
		rdp=bp=rdb;
		matched=ibyt=0;
		ilin=1;
		while(bp<rde) {	/* fill the buffer to begin */
			if ((i=getc(ifp)) != EOF) {
				*bp++ = i;
				if (i == '\n') ilin++;
			} else {
				/* EOF */
				(void) fprintf(stderr,
					"file %s: no string match\n",
					argv[ifile]);
				istex++;
				goto nextfile;
			}
		}
compare:	bp=rdp; pp=pat; ibyt++;
		while (bp<rde) if (*bp++ != *pp++) goto nomatch;
		if (rdb != rdp) {
			bp=rdb;
			while (bp<rdp) if (*bp++ != *pp++) goto nomatch;
		}
		/* match */
		(void) fprintf(stdout,
			"file %s: string match at byte %ld (line %d):\n",
			argv[ifile],ibyt,ilin-nlpat);
		matched=1;
		show(argv[ifile],ibyt,irdl);	/* show the match in context */
nomatch:	if ((i=getc(ifp)) != EOF) *rdp++ = i;	/* add character */
		else {
			if (!matched) {
				(void) fprintf(stderr,
					"file %s: no string match\n",
					argv[ifile]);
				istex++;
			}
			goto nextfile;
		}
		if (rdp==rde) rdp=rdb;
		if (i=='\n') ilin++;
		goto compare;
nextfile:	if (ifp != NULL) (void) fclose(ifp);
	}
	return istex;
}

static int isbinary;
#include <fcntl.h>
#include <ctype.h>

void show(filename,nbyte,patsiz)	/* show byte nbyte in context */
char *filename;
long nbyte;
int patsiz;
{
	int open(), close(), read();
	long lseek();
	static int fd = -1;
	int startrd;
	register char *eobspot;
	register char *bytspot;
	int rdwant, rdget;
	char *bod, *eod;
	register int bytbound;
	static long a = -1;
	static long b = -1;
	static char *lastfile=0;
	register char c;
	int strcmp();
	static char buf[1024];
	void dump(), perror();
	char *cp;

	nbyte -= 1;	/* change to a 0-based offset */

	if (filename == lastfile) {	/* can get away with this since names
					   are coming from argv */
		if (a<=nbyte && b>=nbyte) {
			/* already showed the context of this byte */
			(void) printf("**SEE ABOVE**\n");
			return;
		}
	} else {
		if (fd >= 0) {
			(void) close(fd);
			fd = -1;
		}
	}


	if (fd < 0) {	/* open file */
		if ((fd=open(filename,O_RDONLY)) < 0) {
			perror(filename);
			return;
		}
	}

	lastfile = filename;

	startrd=nbyte-512;		/* seek here in file to get context */
	if (startrd<0) startrd=0;
	bytspot=(nbyte-startrd) + buf;	/* location in buf to see this byte */

	if (lseek(fd,(long)startrd,0L) < 0) {
		perror("lseek");
		(void) close(fd);
		fd = -1;
		return;
	}

	rdwant=1024; cp=buf; eobspot=0;

	while (rdwant) {
		rdget=read(fd,cp,(unsigned)rdwant);
		if (rdget<0) {
			perror(filename);
			(void) close(fd);
			fd = -1;
			return;
		} else if (rdget==0) {
			eobspot=cp-1;
			break;
		} else {
			rdwant -= rdget;
			cp += rdget;
		}
	}

	if (!eobspot) eobspot=buf+1023;

	/* Check whether the sample is text or not */
	isbinary=0;
	for (cp=buf; cp<=eobspot; cp++) {
		c = *cp;
		if (!isprint(c) && !(c>=007 && c<=012) /* bell, bs, tab, nl */
			&& c != '\r' && c != '\f' && c != '\033' /* esc */) {
			isbinary=1;
			break;
		}
	}

	if (!isbinary) {
		for (bod=bytspot; bod>=buf; bod--)
			if (*bod == '\n') break;
		for (eod=bytspot; eod <= eobspot; eod++)
			if (*eod == '\n') break;
	}

	if (isbinary || bod<buf) {
		bytbound=nbyte/512;
		bytbound *= 512;
		bod = bytspot-(nbyte-bytbound);
		if (bod<buf) bod=buf;
	}
	if (isbinary || eod>eobspot) {
		bytbound=nbyte/512;
		bytbound += 1;
		bytbound *= 512;
		eod = bytspot+(bytbound-nbyte);
		if (eod>eobspot) eod=eobspot;
	}
	if (!isbinary && *bod == '\n' && bod != bytspot && bod < eod) bod++;
	if ((eod-bytspot+1) < patsiz)
		eod=((bytspot+patsiz-1)<eobspot ? bytspot+patsiz-1 : eobspot);
	a=nbyte-(bytspot-bod);
	b=nbyte+(eod-bytspot);
	dump(bod,eod);
	return;
}

void dump(beg,end)	/* dump characters from beg to end */
register char *beg, *end;
#define PRINTF	(void) printf
{
	register unsigned char c;
	register char *cp;
	for (cp=beg;cp<=end;cp++) {
		if ((c = *cp) >= ' ' && c <= '~') PRINTF("%c",c);
		else {
			switch(c) {
				case '\b':	PRINTF("<\\b>");break;
				case '\t':	PRINTF("<\\t>");break;
				case '\f':	PRINTF("<\\f>");break;
				case '\n':	if (isbinary)
							PRINTF("<\\n>");
						else
							PRINTF("\n");
						break;
				case '\r':	PRINTF("<\\r>");break;
				default:	if (c>0 && c<27) {
							c += '@';
							PRINTF("<^%c>",c);
						}
						else
							PRINTF("<\\%o>",(int)c);
			}
		}
	}
	if (isbinary || (*end != '\n')) PRINTF("\n");
	return;
}
