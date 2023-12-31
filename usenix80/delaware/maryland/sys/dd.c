#
/*
 *	Program modified 1 Dec 1976 to provide padded output on "physical"
 *	magnetic tape. Modifications by Larry Bickford, V2 .
 *	KEY --- "ELROND -- lab"
 *
 *	"Creat" filemode changed from 0666 to FILMOD
 *	KEY --- "ELROND -- lab (2)"
 *
 *	fix added to prevent if=sam of=sam
 *	dlm 26 April 1977
 *
 *	"seek=" command seeks by blocks relatively before starting input.
 *	RLKirby December 28, 1978.
 *
 *	"pad" and "trim" conversions remove or insert trailing newline
 *	without translating from or to ascii.
 *	RLKirby January 2, 1978.
 *
 *	Octal numbers in call line parameters are specified by leading zero.
 *	RLKirby January 2, 1978.
 *
 *	"oseek=" command seeks by blocks relatively before starting output.
 *	RLKirby April 15, 1980.
 */
#define	LCASE	01
#define	UCASE	02
#define	SWAB	04
#define NERR	010
#define SYNC	020
#define MTPAD	040	/* ELROND -- lab *** */
#define NOTRNS	0100	/* No translation flag RLKirby */
#define FILMOD	0664	/* ELROND -- lab (2) */
int	cflag;
int	fflag;
int	skip;
int     bseek;
int	oseek;
int     count;
char	*string;
char	*ifile;
char	*ofile;
char	*ibuf;
char	*obuf;
char	*ibs	512;
char	*obs	512;
char	*bs;
char	*cbs;
char	*ibc;
char	*obc;
char	*cbc;
int	nifr;
int	nipr;
int	nofr;
int	nopr;
int	ntrunc;
int	ibf;
int	obf;
char	*op;
int	nspace;
char	etoa[]
{
	0000,0001,0002,0003,0234,0011,0206,0177,
	0227,0215,0216,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0235,0205,0010,0207,
	0030,0031,0222,0217,0034,0035,0036,0037,
	0200,0201,0202,0203,0204,0012,0027,0033,
	0210,0211,0212,0213,0214,0005,0006,0007,
	0220,0221,0026,0223,0224,0225,0226,0004,
	0230,0231,0232,0233,0024,0025,0236,0032,
	0040,0240,0241,0242,0243,0244,0245,0246,
	0247,0250,0133,0056,0074,0050,0053,0041,
	0046,0251,0252,0253,0254,0255,0256,0257,
	0260,0261,0135,0044,0052,0051,0073,0136,
	0055,0057,0262,0263,0264,0265,0266,0267,
	0270,0271,0174,0054,0045,0137,0076,0077,
	0272,0273,0274,0275,0276,0277,0300,0301,
	0302,0140,0072,0043,0100,0047,0075,0042,
	0303,0141,0142,0143,0144,0145,0146,0147,
	0150,0151,0304,0305,0306,0307,0310,0311,
	0312,0152,0153,0154,0155,0156,0157,0160,
	0161,0162,0313,0314,0315,0316,0317,0320,
	0321,0176,0163,0164,0165,0166,0167,0170,
	0171,0172,0322,0323,0324,0325,0326,0327,
	0330,0331,0332,0333,0334,0335,0336,0337,
	0340,0341,0342,0343,0344,0345,0346,0347,
	0173,0101,0102,0103,0104,0105,0106,0107,
	0110,0111,0350,0351,0352,0353,0354,0355,
	0175,0112,0113,0114,0115,0116,0117,0120,
	0121,0122,0356,0357,0360,0361,0362,0363,
	0134,0237,0123,0124,0125,0126,0127,0130,
	0131,0132,0364,0365,0366,0367,0370,0371,
	0060,0061,0062,0063,0064,0065,0066,0067,
	0070,0071,0372,0373,0374,0375,0376,0377,
};
char	atoe[]
{
	0000,0001,0002,0003,0067,0055,0056,0057,
	0026,0005,0045,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0074,0075,0062,0046,
	0030,0031,0077,0047,0034,0035,0036,0037,
	0100,0117,0177,0173,0133,0154,0120,0175,
	0115,0135,0134,0116,0153,0140,0113,0141,
	0360,0361,0362,0363,0364,0365,0366,0367,
	0370,0371,0172,0136,0114,0176,0156,0157,
	0174,0301,0302,0303,0304,0305,0306,0307,
	0310,0311,0321,0322,0323,0324,0325,0326,
	0327,0330,0331,0342,0343,0344,0345,0346,
	0347,0350,0351,0112,0340,0132,0137,0155,
	0171,0201,0202,0203,0204,0205,0206,0207,
	0210,0211,0221,0222,0223,0224,0225,0226,
	0227,0230,0231,0242,0243,0244,0245,0246,
	0247,0250,0251,0300,0152,0320,0241,0007,
	0040,0041,0042,0043,0044,0025,0006,0027,
	0050,0051,0052,0053,0054,0011,0012,0033,
	0060,0061,0032,0063,0064,0065,0066,0010,
	0070,0071,0072,0073,0004,0024,0076,0341,
	0101,0102,0103,0104,0105,0106,0107,0110,
	0111,0121,0122,0123,0124,0125,0126,0127,
	0130,0131,0142,0143,0144,0145,0146,0147,
	0150,0151,0160,0161,0162,0163,0164,0165,
	0166,0167,0170,0200,0212,0213,0214,0215,
	0216,0217,0220,0232,0233,0234,0235,0236,
	0237,0240,0252,0253,0254,0255,0256,0257,
	0260,0261,0262,0263,0264,0265,0266,0267,
	0270,0271,0272,0273,0274,0275,0276,0277,
	0312,0313,0314,0315,0316,0317,0332,0333,
	0334,0335,0336,0337,0352,0353,0354,0355,
	0356,0357,0372,0373,0374,0375,0376,0377,
};


main(argc, argv)
int	argc;
char	**argv;
{
	register (*conv)();
	register char *ip;
	register c;
	int ebcdic(), ascii(), null(), cnull(), term();
	int a;

	conv = null;
	for(c=1; c<argc; c++) {
		string = argv[c];
		if(match("ibs=")) {
			ibs = number();
			continue;
		}
		if(match("obs=")) {
			obs = number();
			continue;
		}
		if(match("cbs=")) {
			cbs = number();
			continue;
		}
		if (match("bs=")) {
			bs = number();
			continue;
		}
		if(match("if=")) {
			ifile = string;
			continue;
		}
		if(match("of=")) {
			ofile = string;
			continue;
		}
		if(match("skip=")) {
			skip = number();
			continue;
		}
		if(match("seek=")) {
			bseek = number();
			continue;
		}
		if(match("oseek=")) {
			oseek = number();
			continue;
		}
		if(match("count=")) {
			count = number();
			continue;
		}
		if(match("conv=")) {
		cloop:
			if(match(","))
				goto cloop;
			if(*string == '\0')
				continue;
			if(match("ebcdic")) {
				conv = ebcdic;
				goto cloop;
			}
			if(match("ascii")) {
				conv = ascii;
				goto cloop;
			}
			if(match("lcase")) {
				cflag =| LCASE;
				goto cloop;
			}
			if(match("ucase")) {
				cflag =| UCASE;
				goto cloop;
			}
			if(match("swab")) {
				cflag =| SWAB;
				goto cloop;
			}
			if(match("noerror")) {
				cflag =| NERR;
				goto cloop;
			}
			if(match("sync")) {
				cflag =| SYNC;
				goto cloop;
			}
			if(match("mtpad")) {
				cflag =| MTPAD;
				goto cloop;
			}
			if(match("pad")) {
				conv = ebcdic;
				cflag =| NOTRNS;
				goto cloop;
			}
			if(match("trim")) {
				conv = ascii;
				cflag =| NOTRNS;
				goto cloop;
			}
		}
		printf("bad arg: %s\n", string);
		exit();
	}
	if( ifile != 0 && equal(ifile,ofile) ) {
		printf("dd: input and output file names are the same\n");
		exit();
	}
	if(conv == null && cflag&(LCASE|UCASE))
		conv = cnull;
	if (bs) {
		ibs = obs = bs;
		if (conv == null)
			fflag++;
	}
	if(ibs == 0 || obs == 0) {
		printf("counts: cannot be zero\n");
		exit();
	}
	ibuf = sbrk(ibs);
	if (fflag)
		obuf = ibuf;
	else
		obuf = sbrk(obs);
	if(ibuf == -1 || obuf == -1) {
		printf("not enough memory\n");
		exit();
	}
	if (ifile)
		ibf = open(ifile, 0);
	else
		ibf = dup(0);
	if(ibf < 0) {
		printf("cannot open: %s\n", ifile);
		exit();
	}
	if ((signal(2, 1) & 01) == 0)
		signal(2, term);
	if(bseek)
		seek(ibf, bseek, 3);
	while(skip) {
		read(ibf, ibuf, ibs);
		skip--;
	}
	if (ofile) {	/* Don't truncate if seeking on output. RLK */
		if(oseek == 0 || (obf = open(ofile, 1)) == -1)
			obf = creat(ofile, FILMOD);	/*ELROND -- lab (2)*/
	} else
		obf = dup(1);
	if(obf < 0) {
		printf("cannot create: %s\n", ofile);
		exit();
	}
	/* Make the translation tables perform NOP. RLK */
	if(cflag & NOTRNS) for(c = 0377; c >= 0; c--)
		atoe[c] = etoa[c] = c;
	ibc = 0;
	obc = 0;
	cbc = 0;
	op = obuf;

	if(oseek)
		seek(obf, oseek, 3);

loop:
	if(ibc-- == 0) {
		ibc = 0;
		if(count==0 || nifr+nipr!=count) {
			if(cflag&(NERR|SYNC))
			for(ip=ibuf+ibs; ip>ibuf;)
				*--ip = 0;
			ibc = read(ibf, ibuf, ibs);
		}
		if(ibc == -1) {
			perror("read");
			if((cflag&NERR) == 0) {
				flsh();
				term();
			}
			ibc = 0;
			for(c=0; c<ibs; c++)
				if(ibuf[c] != 0)
					ibc = c;
			stats();
		}
		if(ibc == 0) {
			flsh();
			term();
		}
		if(ibc != ibs) {
			nipr++;
			if(cflag&SYNC)
				ibc = ibs;
		} else
			nifr++;
		ip = ibuf;
		c = (ibc>>1) & ~1;
		if(cflag&SWAB && c)
		do {
			a = *ip++;
			ip[-1] = *ip;
			*ip++ = a;
		} while(--c);
		ip = ibuf;
		if (fflag) {
			obc = ibc;
			flsh();
			ibc = 0;
		}
		goto loop;
	}
	c = 0;
	c =| *ip++;
	(*conv)(c);
	goto loop;
}

flsh()
{
	register c;

	if(obc) {
		if(cflag & MTPAD) obc = obs;    /* ELROND -- lab */
		if(obc == obs)
			nofr++; else
			nopr++;
		c = write(obf, obuf, obc);
		if(c != obc) {
			perror("write");
			term();
		for(op=obuf+obs; op>obuf;)      /* ELROND -- lab */
			*--op = 0;
		}
		obc = 0;
	}
}

match(s)
char *s;
{
	register char *cs;

	cs = string;
	while(*cs++ == *s)
		if(*s++ == '\0')
			goto true;
	if(*s != '\0')
		return(0);

true:
	cs--;
	string = cs;
	return(1);
}

number()
{
	register char *cs;
	register n;

	cs = string;
	n = 0;
	if(*cs == '0') while(*cs >= '0' && *cs <= '9') {
		if(*cs == '8' || *cs == '9') {
			printf("improper octal number: %s\n", string);
			exit(1);
		}
		n = (n << 3) + (*cs++ - '0');
	} else
		while(*cs >= '0' && *cs <= '9')
			n = n*10 + *cs++ - '0';
	for(;;)
	switch(*cs++) {

	case 'k':
		n =* 1024;
		continue;

	case 'w':
		n =* 2;
		continue;

	case 'b':
		n =* 512;
		continue;

	case '*':
	case 'x':
		string = cs;
		n =* number();

	case '\0':
		return(n);
	}
	printf("not a number: %s\n", string);
	exit();
}

cnull(cc)
{
	register c;

	c = cc;
	if(cflag&UCASE && c>='a' && c<='z')
		c =+ 'A'-'a';
	if(cflag&LCASE && c>='A' && c<='Z')
		c =+ 'a'-'A';
	null(c);
}

null(c)
{

	*op = c;
	op++;
	if(++obc >= obs) {
		flsh();
		op = obuf;
	}
}

ascii(cc)
{
	register c;

	c = 0;
	c =| etoa[cc];
	if(cbs == 0) {
		cnull(c);
		return;
	}
	if(c == ' ') {
		nspace++;
		goto out;
	}
	while(nspace > 0) {
		null(' ');
		nspace--;
	}
	cnull(c);

out:
	if(++cbc >= cbs) {
		null('\n');
		cbc = 0;
		nspace = 0;
	}
}

ebcdic(cc)
{
	register c;

	c = cc;
	if(cflag&UCASE && c>='a' && c<='z')
		c =+ 'A'-'a';
	if(cflag&LCASE && c>='A' && c<='Z')
		c =+ 'a'-'A';
	c = atoe[c] & 0377;
	if(cbs == 0) {
		null(c);
		return;
	}
	if(cc == '\n') {
		while(cbc < cbs) {
			null(atoe[' ']);
			cbc++;
		}
		cbc = 0;
		return;
	}
	if(cbc == cbs)
		ntrunc++;
	cbc++;
	if(cbc <= cbs)
		null(c);
}

term()
{

	stats();
	exit();
}

stats()
{

	printf("%l+%l records in\n", nifr, nipr);
	printf("%l+%l records out\n", nofr, nopr);
	if(ntrunc)
		printf("%l truncated records\n", ntrunc);
}

putchar(c)
{

	write(2, &c, 1);
}
