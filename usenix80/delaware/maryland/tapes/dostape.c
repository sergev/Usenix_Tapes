#
#define BUFFSZ 8192
/*
 * DOSTAPE
 * writes DOS/BATCH-11 format magtapes.
 * Program totally rewritten by Robert L. Kirby from
 * original version written by Russ Smith.
 *
 * dostape [ -special ] [ -N ] [ -n alias ] [ -uN ] [ -gN ] [ -pN ]
 *         [ -a ] [ -b ] [ -fN ] [ -hN ] [ file ] ...
 *
 * cc -s -O dostape.c readrow.c errprnt.c; mv a.out dostape
 */
char record[512];
char *buff;
char *buflen 0;
int buffsz BUFFSZ;

int     header[256] {
	0,0,0, /* File name and extension */
	0401,  /* UIC (always (almost) 1,1) */
	0233,  /* Protection code (always 233) */
	033250,/* Date (will be changed...) */
	0      /* Dummy (i.e. not used) */
	};

struct {
	char    junk[32];
	int     cdate[2]; /* Date the file was last modified */
} inode;

struct  integ {
	char    lobyte;
	char    hibyte;
};

#define FASCII 0
#define UFRBIN 1
#define FORBIN 2

int tape -1;
int headlen 0;
int optr;
int chksum;
int *timevec;
char *cptr;
char *tname;
char *dname "/dev/nrw_rmt0"; /* Default drive is low density */
char *argv0 0;
char file;
char errs;
char alias;
char ufilf;
char gfilf;
char ftype UFRBIN;
int *localtime();

main(argc, argv)
char	*argv[];
{
	register char **ap;
	register  i, k;

	ap = argv;
	argv0 = *ap;
	tname = dname;
	if(argc-- == 0) errprnt("no argv[0]");
	errs = dup(2);
	close(2); dup(1);
	while(*++ap != -1 || argc == 0)
	      if(argc != 0 && (i = **ap) == '-' && *++*ap != 0) {
		argc--;
		switch(i = *((*ap)++)) {
		case 'n':
			alias++;
			if(**ap == 0) {
				argc--;
				if(*++ap != -1) makename(*ap, header);
				else errprnt("no alias given after -n");
			} else makename(*ap, header);
			break;
		case 'u':
			if(**ap == 0) ufilf = 0;
			else {
				ufilf++;
				header[3].hibyte = atoi(*ap);
			}
			break;
		case 'g':
			if(**ap == 0) gfilf = 0;
			else {
				gfilf++;
				header[3].lobyte = atoi(*ap);
			}
			break;
		case 'a': /* formatted ascii */
			ftype = FASCII; goto fbsize;
		case 'b': /* unformatted binary (default) */
			ftype = UFRBIN; goto fbsize;
		case 'f': /* formatted binary */
			ftype = FORBIN;
		fbsize:
			if(**ap != 0) {
				if((cptr = atoi(*ap)) == 0)
					errprnt("bad %s input length",
						&*ap[-2]);

			} else cptr = BUFFSZ;
			if(buflen != 0 && cptr > buflen) {
				if(sbrk(cptr - buflen) == -1)
					errprnt("expansion troubles");
				buflen = cptr;
			}
			buffsz = cptr;
			break;
		case 'h':
			cptr = headlen = atoi(*ap);
			if(cptr > buflen) {
				if(sbrk(cptr - buflen) == -1)
					errprnt("expansion troubles");
				buflen = cptr;
			}
			break;
		case 'p':
			header[4] = (**ap == 0 ? 0233 : atoo(*ap));
			break;
		case '-':
			close(2); dup(errs);
			tape = 1;
			tname = "standard output";
			break;
		default:
			if('0' <= i && i < '8' && **ap == 0) {
				dname[12] = i;
				tname = dname;
			} else tname = --*ap;
		}
	} else {
		if(buff == 0 && (buff = sbrk(buflen = buffsz)) == -1)
			errprnt("cannot expand by %d", buffsz);

		if(argc == 0) {
			file = 0;
			*--ap = "standard output";
		} else if((file = open(*ap, 0)) < 0)
			errprnt("cannot open %s for reading", *ap);

		/* Get the last modification time */
		if(fstat(file, &inode) == -1)
			errprnt("cannot fstat %s", *ap);
		timevec = localtime(&inode.cdate[0]);
		/* Convert to DOS format of Julian based on 1970 */
		header[5] = timevec[7] + ((timevec[5] - 70) * 1000);
		if(ufilf == 0) /* user number */
			header[3].hibyte = inode.junk[7];
		if(gfilf == 0) /* group number */
			header[3].lobyte = inode.junk[8];

		if(alias == 0)
			makename(file == 0 ? "unix.out" : *ap, header);
		else alias = 0;

		if(tape == -1 && (tape = creat(tname, 0664)) < 0)
			errprnt("cannot creat %s", tname);

		if(write(tape, header, 14) != 14) {
			seek(tape, 0, 0);
			errprnt("header write error on %s", *ap);
		}

		switch(ftype) {
		case FASCII:
			while((i = read(file, buff, buffsz)) > 0) {
				for(k = 0; k < i; k++) {
					if(buff[k] == '\n')
						outchar('\r');
					outchar(buff[k]);
				}
			}
			break;
		case UFRBIN:
			while((i = read(file, buff, buffsz)) > 0)
				for(k = 0; k < i; k++) outchar(buff[k]);
			break;
		case FORBIN:
			if(headlen != 0) {
				if((i = (file == 0 ?
				   buffsz - readrow(buff, headlen) :
				   read(file, buff, headlen))) <= 0)
					errprnt("data header read error");
				outchar(1); outchar(0); chksum = 1;
				i =+ 4;
				outchar(i); chksum =+ i;
				outchar(i >> 8); chksum =+ i >> 8;
				i =- 4;
				for(k = 0; k < i; k++) {
					outchar(buff[k]);
					chksum =+ buff[k];
				}
				outchar(-chksum);
				if(optr != 0) while(optr != 0) outchar(0);
			}
			while((i = (file == 0 ?
				buffsz - readrow(buff, buffsz) :
				read(file, buff, buffsz))) > 0) {
				outchar(1); outchar(0); chksum = 1;
				i =+ 4;
				outchar(i); chksum =+ i;
				outchar(i >> 8); chksum =+ i >> 8;
				i =- 4;
				for(k = 0; k < i; k++) {
					outchar(buff[k]);
					chksum =+ buff[k];
				}
				outchar(-chksum);
				if((i & 1) != 1) outchar(0);
			}
			break;
		}
		if(optr != 0) while(optr != 0) outchar(0);
		if(i == -1) errprnt("error reading file %s", *ap);
		if(tape != 1) {
			if(close(tape) == -1)
				errprnt("error closing %s", tname);
			tape = -1;
		} else write(tape, record, 0);
		if(file != 0) {
			if(close(file) == -1)
				errprnt("error closing %s", *ap);
		} else return;
	}
}

outchar(cc)
char cc;
{
	register j;

	j = optr;
	record[j] = cc;
	if(j >= 512 - 1) {
		if(write(tape, record, 512) != 512)
			errprnt("write error on %s", tname);
		optr = 0;
	} else optr++;
}

atoo(ascii)
char *ascii;
{
	register int cnt;
	register char *apt;

	cnt = 0;
	for(apt = ascii; *apt != 0; cnt = cnt * 8 + (*apt++ - '0'))
		if('0' > *apt || *apt > '7')
			errprnt("%s is not octal", ascii);
	return(cnt);
}

makename(ascii, rad50)
char *ascii;
int *rad50;
{
	register char *apt, *dot;
	register int *pnt;
	static int cnt, tmp;

	/* find last filename and last dot within it */
	apt = ascii;
	cnt = 0;
	for(dot = apt; *apt != 0;)
		if(*apt == '.') dot = apt++;
		else if(*apt++ == '/') ascii = dot = apt;
	/* translate file name */
	apt = ascii;
	pnt = rad50;
	*pnt = 0;
	for(cnt = 0; cnt < 6;) {
		if((apt != dot || *dot != '.') && *apt != 0) {
			if((tmp = ato50(*apt++)) >= 0) {
				*pnt = *pnt * 050 + tmp;
				if(++cnt == 3) *++pnt = 0;
			}
		} else {
			*pnt =* 050;
			if(++cnt == 3) *++pnt = 0;
		}
	}
	/* translate extension */
	if(*dot == '.') apt = ++dot;
	*++pnt = 0;
	for(cnt = 0; cnt < 3;) {
		if(*apt != 0) {
			if((tmp = ato50(*apt++)) >= 0) {
				cnt++;
				*pnt = *pnt * 050 + tmp;
			}
		} else {
			cnt++;
			*pnt =* 050;
		}
	}
}

ato50(cc)
int cc;
{
	register c;

	c = cc & 0177;
	if('a' <= c && c <= 'z') return(c - 'a' + 1);
	if('0' <= c && c <= '9') return(c - '0' + 036);
	if(c == ' ' || c == '\t') return(0);
	if('A' <= c && c <= 'Z') return(c - 'A' + 1);
	if(c == '-' || c == '_' || c == '$') return(033); /* was just $ */
	if(c == '.') return(034);
	return(035); /* undefined character */
}
