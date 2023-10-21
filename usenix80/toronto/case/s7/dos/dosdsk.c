/*
	convert DOS file(s) to UNIX file(s)

	dosdsk flags disk_name [uic] [files ...]

	This program uses the external functions dosfil.c and r50asc.c.
*/

struct ufdent {
	int	fname[3];	/* filename in rad50 */
	int	cdate;		/* creation date */
	int	nxtfree;	/* next free byte in block */
	int	start;		/* start block */
	int	length;		/* file length */
	int	last;		/* last block in file */
	char	prot,usage;	/* protection code, usage count */
	};
int	tflag=0, xflag=0, uflag=0, vflag=0;
int radbuf[3];
int fi,fo,ufdbuf[256],outbuf[256];
int blkcnt=0;
int	uic = 0;

main(argc,argv)
int argc;
char **argv;
{
	register int i,j;
	char *s, *dsk;
	int ufdstart;

	if ((argc-=2) <= 0) {
		printf("Usage: dosdsk [txuv] disk_name [uic] [files ...]\n");
		exit();
	}
	s = *++argv;
	dsk = *++argv;
	while (*s)
		switch (*s++) {
		case 't':	tflag++;	break;
		case 'x':	xflag++;	break;
		case 'u':	uflag++;	break;
		case 'v':	vflag++;	break;
		default:
			printf("Bad key\n");
			exit();
			}
	if (uflag) {
		if (--argc)
			uic = calcuic(*++argv);
		else {
			printf("No UIC given\n");
			exit();
			}
		}
	else	uic = 0401;
	if ((fi = open(dsk, 0)) == -1) {
		printf("Can't open disk\n");
		exit();
		}
	rawread(1,ufdbuf);
	i=0;
	rawread(ufdbuf[i],ufdbuf);
	if (ufdbuf[i++] != 0) {
		printf("format error in %s\n", dsk);
		exit();
	}
	if (tflag && !uflag) {
		do {
			if ((j = ufdbuf[i++]) == 0) continue;
			printf("[%o,%o]", (j>>8)&0377, j&0377);
			if (vflag)
				printf("\t%o\n", ufdbuf[i]);
			else	printf("\n");
		} while ((i =+ 3) < 253);
		exit();
	}
	while (ufdbuf[i++] != uic)
		if ((i =+ 3) > 252) {
			printf("Can't find [%o,%o]\n", (uic>>8)&0377, uic&0377);
			exit();
			}
	ufdstart = ufdbuf[i];
	if (argc == 1)
		srchufd(0, ufdstart);
	else
		while (--argc)
			srchufd(*++argv, ufdstart);
}

srchufd(f, us)
char *f;
int us;
{
	int rb[3];
	register char *s;
	register struct ufdent *ufd;

	if (f != 0)
		scdosfil(f, rb);
	while (us) {
		rawread(us, ufdbuf);
		for (ufd = &ufdbuf[1]; ufd<&ufdbuf[256-9]; ufd++) {
			if (ufd->fname[0]==0 || (s=r50toasc(ufd->fname))==0)
				continue;
			if (f == 0) {
				if (tflag)
					table(ufd);
				else	xferfile(s, ufd);
				continue;
				}
			if (ufd->fname[0] == rb[0] &&
			    ufd->fname[1] == rb[1] &&
			    ufd->fname[2] == rb[2])  {
				if (tflag)
					table(ufd);
				else	xferfile(f, ufd);
				return;
			}
		}
		us = ufdbuf[0];
	}
	if (f != 0)
		printf("Can't find %s\n", f);
}

table(u)
register struct ufdent *u;
{
	int i, yr;
	long day;
	char *s;
	s = r50toasc(u->fname);
	if (!vflag)
		printf("%s\n", s);
	else {
		printf("%-10.10s %5d%c ", s, u->length, u->cdate&0100000?'C':' ');
		i = u->cdate & 077777;
		yr = i/1000;
		day = yr/4 * (365*3 + 366);
		yr %= 4;
		day += yr*365 + (yr>2);
		day += i%1000;
		day *= 86400L;
		day += 12*3600;
		s = ctime(&day);
		printf("%2.2s %3.3s %2.2s  ", &s[8], &s[4], &s[22]);
		printf("<%3o>\n", u->prot&0377);
		}
	return;
}

xferfile(s, u)
char *s;
struct ufdent *u;
{
	register int i,j,k;

	if (vflag)	printf("x %s\n", s);
	if ((fo = creat(s, 0600)) < 0) {
		printf("cannot creat %s\n", s);
		exit();
	}
	if ((i=u->length) == 0)
		goto close1;
	k = u->start;
	j = u->nxtfree;
	if (u->cdate & 0100000) goto contiguous;
	rawread(k,outbuf);
	while(outbuf[0] != 0) {
		rawwrite(&outbuf[1], 510);
		k = outbuf[0];
		rawread(k, outbuf);
		i--;
	}
	rawwrite(&outbuf[1], j);
closefile:
	if (i != 1)	printf("Length error for %s\n", s);
	if (k!=u->last)	printf("Last block error for %s\n", s);
close1:
	close(fo);
	return;
contiguous:
	while (--i) {
		rawread(k++, outbuf);
		rawwrite(outbuf, 512);
	}
	rawread(k, outbuf);
	if (j == 0) j = 512;
	rawwrite(outbuf, j);
	i++;
	goto closefile;
}

rawread(bno,buffer)
{
	register int n;
	extern errno;

	if ((bno < 0) || (bno >= 4800))  {
		printf("block # %d out of range\n", bno);
		exit();
	}
	seek(fi, bno, 3);
	if ((n=read(fi, buffer, 512)) != 512) {
		 printf("read error %d\n", bno);
		 printf("count = %d;errno = %d\n", n, errno);
		 exit();
	}
}

rawwrite(buffer,bytes)
{
	register int n;
	extern errno;

	if ((n=write(fo, buffer, bytes)) != bytes) {
		printf("write error\n");
		printf("count = %d; errno = %d\n", n, errno);
		exit();
	}
	if (++blkcnt >= 4800) {
		printf("too many blocks xferred!\n");
		exit();
	}
}
/*
	calculate octal uic from ascii string
*/

int calcuic(string)
char *string;
{
	register char *s;
	register int uichi,uiclow;

	s = string;
	uichi = uiclow = 0;
	while (*s >= '0' && *s <= '7') {
		uichi =* 8;
		uichi =+ *s++ - '0';
	}
	if (*s++ != ',') uicerr(string);
	while (*s >= '0' && *s <= '7') {
		uiclow =* 8;
		uiclow =+ *s++ - '0';
	}
	uichi = (uichi << 8) | (uiclow & 0377);
	if (uichi == 0) uicerr(string);
	return(uichi);
}

uicerr(string)
char *string;
{
	printf("%s invalid UIC \n",string);
	exit();
}
