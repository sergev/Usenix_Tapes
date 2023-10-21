#
/* TDIR
 * prints tape directory listings of the number of files given by parameter
 * from a magtape in many different formats. The formats recognised are
 * tp format, DOS-PIP format, XAP picture with or without a header or a file
 * with a tape label only. if the file format does not belong to the above
 * group then a message 'unknown format' is output.
 * cc -s -O tdir.c -lp -lp; mv a.out tdir0
 */
#define BUFFSZ 44544
int dosflag 0;	/* count for DOS-PIP format files */
int total;	/* number of continious files in a ldostape format */
int outcount;	/* a count for the number of files listed */
int large;      /* number of files on magtape that may be listed  */
int fd;		/* file descriptor */
int bcount;	/* number of bytes to read /usr/mdec/mboot */
int rpcount;	/* number of bytes of records from a file */
int count;	/* number of bytes of the first record in tape file */
int v;		/* holds the address for the parent process to continue
		   after a child process dies */
int rows;	/* number of rows in a XAP picture */
int savrows;
char brief 0;   /* abbrieviated listing flag */
char unit '0';	/* tape unit number digit */
char outs[12];  /* area to build ascii name of file */
char *mt1 "/dev/rmt0";	/* opens a file */
char *mt2 "/dev/nrw_rmt0"; /* skips a file */
char *mt3 "vmt0"; /* tp option string */
char *datestr "%2d-%s-%d";
char *p;     /* numeral suffix for 'tdir' */
char *argv0 0;       /* Save 0th parameter - program name */
char tbuff[512];
char buf[BUFFSZ];
char *mon[] {
	"JAN",
	"FEB",
	"MAR",
	"APR",
	"MAY",
	"JUN",
	"JUL",
	"AUG",
	"SEP",
	"OCT",
	"NOV",
	"DEC"
};
int nlpday[ ] { /* number of days in each month */
	31,
	28,
	31,
	30,
	31,
	30,
	31,
	31,
	30,
	31,
	30,
	31
};
struct integ {
	char lobyte;
	char hibyte;
};

main (argc, argv)
int argc;
char *argv[];
{

	outcount = 0;
	large = 9999;
	p = argv0 = argv[0];
	/* find default tape unit number */
	while (*p != 0) p++;
	--p;
	if ('0' <= *p && *p <= '9')
		unit = *p;
	while(--argc > 0) if(**++argv == '-') {
		if(*++*argv == 'b') brief++;
		else unit = argv[0][0];
	} else {
		large = atoi(*argv); /* Number of files to output */
	}
	mt1[8] = unit;
	mt2[12] = unit;
	mt3[3] = unit;
	if(brief) mt3 = &mt3[1];
	if((fd = open("/usr/mdec/mboot", 0)) == -1) {
		printf("tp bootstrap not found\n");
		cexit(1);
	}
	bcount = read(fd, tbuff, sizeof tbuff);
	close(fd);
	while(large-- != 0) {
		if((fd = open(mt1, 0)) == -1) {
			printf("%s cannot open %s\n", argv0, mt1);
			break;
		}
		count = read(fd, buf, BUFFSZ);
		if(count == 0) {
			edostape();
			printf("%s found end of file\n", argv0);
			break;
		} else if(count == -1) {
			count = 0;
			edostape();
			printf("%s read error on first record of file %s\n",
				argv0, mt1);
			break;
		} else
			close(fd);
		outcount++;
		if(count == 512) {
			for(v = 0; v < bcount; v++)
				if(tbuff[v] != buf[v]) break;
			if(v == bcount) {
				edostape();
				printf("tp %s\n", mt3);
				if(brief) putchar('\n');
				cflush(1);
				if((v = fork()) == -1) {
					printf("%s cannot fork\n", argv0);
					break;
				}
				if(v == 0) {
					execl("/bin/tp", "tp", mt3, 0);
					execl("/usr/bin/tp", "tp", mt3, 0);
					printf("%s cannot exec tp\n", argv0);
					cexit(077);
				} else
					wait(&v);
				putchar('\014');   /* form feed */
				if(v.hibyte == 077) break;
				if((fd = open(mt2, 0)) == -1) {
		printf("%s open error using %s to advance\n", argv0, mt2);
					break;
				}
				close(fd);
				continue;
			}
		}
		if((fd = open(mt2, 0)) == -1) {
			printf("%s cannot open %s\n", argv0, mt2);
			break;
		}
		if(dostest(buf)) {
			if(dosflag == 0) {
				printf("\nNAME  .EXT");
				if(!brief)
				       printf("    UIC  Protection  date   ");
				printf(" blocks\n");
			}
			++dosflag;
			ldostape(buf);
			rows = 0;
			while ((rpcount = read(fd, buf, BUFFSZ)) != 0) {
				if(rpcount == -1) {
					printf("\nerror reading %s\n", mt2);
					break;
				}
				rows++;
			}
			printf("%6d\n", --rows);
			close(fd);      /* file skipped */
			continue;
		}
		edostape();
		if((count == 12) && pctwithhdr(buf)) {
			close(fd);
			continue;
		}
		rows = 0;
		while (count == (rpcount = read(fd, buf, BUFFSZ))) {
			if (rpcount == 0) break;
			rows++;
		}
		if(rpcount == -1) {
			printf("error reading record %d of file %d\n",
				rows + 1, outcount);
			break;
		}
		if(((count == 80) || (count == 81)) && (rows < 4)
		     && (rpcount == 0)) {
			printf("tape label with %d records of %d bytes\n",
				rows, count);
			close(fd);
			continue;
		}
		if(count == 512 && rpcount == 0)
			printf("%d record cooked file of unknown format or\n",
				rows);
		if (rpcount == 0) {
			printf("%d row XAP picture with %d bytes per row\n\n",
				rows, count);
		} else  {
			savrows = rows++;
			while ((rpcount = read(fd, buf, BUFFSZ)) != -1) {
				if (rpcount == 0) break;
				rows++;
			}
			if(rpcount == -1) {
				printf("error reading record %d of", rows+ 1);
				printf(" unknown format file %d\n", outcount);
				break;
			}
			printf("Unknown format file with %d records\n", rows);
			if(savrows != 1)
				printf("the first %d records had %d bytes each\n",
					savrows, count);
		}
		close(fd);
	}
	printf("end %s, %d tape file%s listed\n", argv0, outcount,
		(outcount == 1 ? "" : "s"));
	cexit(0);
}

struct {
	char *cpnt;
};

int dostest(ibuf)
int *ibuf;
{
	register i;
	int sum;

	sum = 0;
	if(count == 512) {
		for(i = 7; i < 256; i++)
			if(ibuf[i] != 0) return(0);
	} else if(count != 14) return(0);
	for (i = 0; i <= 2; ++i) {
		if(ibuf[i].cpnt > 63999) return(0);
		sum =+ ibuf[i];
	}
	if(sum == 0 && count == 512) return(0);
	if((0 >= ibuf[4]) || (ibuf[4] > 0377)) return (0);
	return(1);
}

int pctwithhdr (ibuf)
int *ibuf;
{
	register i;

	i = 0;
	if ((ibuf[i] == 0) && (ibuf[i+2] == 0) && (ibuf[i+4] == 0))  {
		printf ("MINI-XAP picture with header\n");
		printf ("\ncolumns   rows   Pixel size\n");
		printf ("%7d%7d%7d\n",
			ibuf[i + 1], ibuf[i + 3], ibuf[i + 5]);
		return(1);
	}
	return(0);
}

ldostape(header)
int *header;
{
	asciiname(header, outs);
	printf("%s ", outs);
	if(!brief) {
		printf("[%03o,%03o] <%03o> ",
			header[3].hibyte & 0377,
			header[3].lobyte & 0377,
			header[4]);
		jdate(header[5]); /* Julian date based from 1970 */
	}
}

int d50(rad50)
int *rad50;
{
	register int *r50;
	extern ldivr;

	r50 = rad50;
	*r50 = ldiv(0, *r50, 050);
	return(ldivr);
}

asciiname(rad50, ascii)
int *rad50;
char *ascii;
{
	register char *apt;
	register int *r50pt;

	r50pt = rad50; apt = &ascii[2];
	*apt = r50toa(d50(r50pt));
	*--apt = r50toa(d50(r50pt));
	*--apt = r50toa(d50(r50pt));
	r50pt++;
	apt = &apt[5];
	*apt = r50toa(d50(r50pt));
	*--apt = r50toa(d50(r50pt));
	*--apt = r50toa(d50(r50pt));
	r50pt++;
	apt = &apt[6];
	*apt = r50toa(d50(r50pt));
	*--apt = r50toa(d50(r50pt));
	*--apt = r50toa(d50(r50pt));
	*--apt = '.';
}

int r50toa(cc)
int cc;
{
	register c;

	c = cc;
	if(c < 0) c = -c;
	if(1 <= c && c <= 27) return(c - 1 + 'a');
	if(c == 0) return ' ';
	if(036 <= c && c < 050) return(c - 036 + '0');
	if(c == 033) return('-');
	if(c == 034) return('.');
	return('?'); /* undefined character */
}
jdate (juldays)
int juldays;
{
	register days, year, i;

	year = juldays / 1000;
	days = juldays - (year * 1000);
	year = 1970 + year;
	nlpday[1] = 28;

	if(days == 0) {
		days = 1;
		printf(datestr, days, mon[0], year);
		return;
	}
	if((year % 4 == 0) && (year % 100 != 0) || (year % 400 == 0)) {
		if (days == 366)  {
			days = 31;
			printf(datestr, days, mon[11], year);
			return;
		}
		for ( i = 0; days > nlpday[i]; ++i)  {
			if ((i + 1) == 1)
				++nlpday[i + 1];
			days = days - nlpday[i];
		}
		printf(datestr, days, mon[i], year);
		return;
	} else {
		if ( days == 365) {
			days = 31;
			printf(datestr, days, mon[11], year);
			return;
		}
		for (i = 0; days > nlpday[i]; ++i)
			days = days - nlpday[i];
		printf(datestr, days, mon[i], year);
	}
}

edostape()
{
	if(dosflag != 0) {
		printf("\n%s found %d DOS-PIP file%s on %s\n",
			argv0, dosflag, (dosflag == 1 ? "" : "s"), mt1);
		if(dosflag > 1)
			putchar('\014');
	}
	if(count != 0)
		printf("%s of file %d\n", argv0, outcount);
	dosflag = 0;
}

/*
 * cexit and cflush when portable c library not used.
cexit(v)
{
	exit(v);
}
cflush(v)
{
	flush();
}
 */
