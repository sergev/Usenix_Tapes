#
#define XSTTY /* use expanded stty call (speed word differ see tty.h */
/*
 * set teletype modes
 *
 * changed 50 baud table to 19200 baud.
 * -ghg 2/25/77
 *
 *	Added "list" verb
 *	-TGI	[ 07/23 19:38 ]
 */

/*
 * tty flags
 */
#define	HUPCL	01
#define	XTABS	02
#define	LCASE	04
#define	ECHO	010
#define	CRMOD	020
#define	RAW	040
#define	ODDP	0100
#define	EVENP	0200
#define	ANYP	0300

#ifdef XSTTY
/* extra mode bits in high order 12 bits of speed word.
 * Note only 4 bits (3-0) are used to set speed, ispeed and 
 * ospeed are both set to speed.
 * -ghg 7/4/77.
 */

#define SPEED	017		/* index into baud rate table */
#define DRSVD	020		/* device reserved by another handler */
#define READC	040		/* realtime read to circular user buffer */
#define NBLKR	0100		/* non blocking reads on this device */
#define NBLKW	0200		/* non blocking writes on this device */
#define PUMP	0400		/* special processing for PUMP */
#define SUSNL	01000		/* suspend output on newline output */
#define BLITZ	02000		/* blitz - abort all i/o until close */
#define	EP	04000		/* electrostatic printer (internal use) */
#define	APL	010000		/* special stuff for Peter Hallenbeck */
#define	HRAW	020000		/* Half raw - wakeup on control chars */
#endif

/*
 * Delay algorithms
 */
#define	CR0	0
#define	CR1	010000
#define	CR2	020000
#define	CR3	030000
#define	NL0	0
#define	NL1	000400
#define	NL2	001000
#define	NL3	001400
#define	TAB0	0
#define	TAB1	002000
#define	TAB2	004000
#define	TAB3	006000
#define	FF0	0
#define	FF1	040000
#define	BS0	0
#define	BS1	0100000
#define	ALL	0177400

struct
{
	char	*string;
	int	speed;
} speeds[]
{
	"0",	(0<<8)|0,
	"50",	(1<<8)|1,
	"75",	(2<<8)|2,
	"110",	(3<<8)|3,
	"134",	(4<<8)|4,
	"134.5",(4<<8)|4,
	"150",	(5<<8)|5,
	"200",	(6<<8)|6,
	"300",	(7<<8)|7,
	"600",	(8<<8)|8,
	"1200",	(9<<8)|9,
	"1800",	(10<<8)|10,
	"2400",	(11<<8)|11,
	"4800",	(12<<8)|12,
	"9600",	(13<<8)|13,
	"19200",(1<<8)|1,
	"19.2",(1<<8)|1,
	"exta",	(14<<8)|14,
	"warp", (14<<8)|14,
	"38400",(14<<8)|14,
	"38.4",(14<<8)|14,
	"extb",	(15<<8)|15,
	0,
};
struct
{
	char	*string;
	int	set;
	int	reset;
} modes[]
{
	"even",
	EVENP, 0,

	"-even",
	0, EVENP,

	"odd",
	ODDP, 0,

	"-odd",
	0, ODDP,

	"raw",
	RAW, 0,

	"-raw",
	0, RAW,

	"cooked",
	0, RAW,

	"-nl",
	CRMOD, 0,

	"nl",
	0, CRMOD,

	"echo",
	ECHO, 0,

	"-echo",
	0, ECHO,

	"LCASE",
	LCASE, 0,

	"lcase",
	LCASE, 0,

	"-LCASE",
	0, LCASE,

	"-lcase",
	0, LCASE,

	"-tabs",
	XTABS, 0,

	"tabs",
	0, XTABS,

	"hup",
	HUPCL, 0,

	"-hup",
	0, HUPCL,

	"cr0",
	CR0, CR3,

	"cr1",
	CR1, CR3,

	"cr2",
	CR2, CR3,

	"cr3",
	CR3, CR3,

	"tab0",
	TAB0, TAB3,

	"tab1",
	TAB1, TAB3,

	"tab2",
	TAB2, TAB3,

	"tab3",
	TAB3, TAB3,

	"nl0",
	NL0, NL3,

	"nl1",
	NL1, NL3,

	"nl2",
	NL2, NL3,

	"nl3",
	NL3, NL3,

	"ff0",
	FF0, FF1,

	"ff1",
	FF1, FF1,

	"bs0",
	BS0, BS1,

	"bs1",
	BS1, BS1,

	"33",
	CR1, ALL,

	"tty33",
	CR1, ALL,

	"mowle",
	LCASE, 0,
	
	"MOWLE",
	LCASE, 0,
	
	"-mowle",
	0, LCASE,
	
	"-MOWLE",
	0, LCASE,

	"upper",
	LCASE, 0,

	"UPPER",
	LCASE, 0,

	"-upper",
	0, LCASE,

	"-UPPER",
	0, LCASE,

	"37",
	FF1+CR2+TAB1+NL1, ALL,

	"tty37",
	FF1+CR2+TAB1+NL1, ALL,

	"05",
	NL2, ALL,

	"vt05",
	NL2, ALL,

	"tn",
	CR1, ALL,

	"tn300",
	CR1, ALL,

	"ti",
	CR2, ALL,

	"ti700",
	CR2, ALL,

	"tek",
	FF1, ALL,

	0,
	};

#ifdef XSTTY
struct {
	char *string;
	int set;
	int reset;
} modes2[] {

	"drsvd",
	DRSVD,
	0,

	"-drsvd",
	0,
	DRSVD,

	"readc",
	READC,
	0,

	"-readc",
	0,
	READC,

	"nblkr",
	NBLKR,
	0,

	"-nblkr",
	0,
	NBLKR,

	"nblkw",
	NBLKW,
	0,

	"-nblkw",
	0,
	NBLKW,

	"pump",
	PUMP,
	0,

	"-pump",
	0,
	PUMP,

	"susnl",
	SUSNL,
	0,

	"-susnl",
	0,
	SUSNL,

	"blitz",
	BLITZ,
	0,

	"-blitz",
	0,
	BLITZ,

	"apl",
	APL,
	0,

	"-apl",
	0,
	APL,

	0};

#endif

char	*arg;
int	mode[3];

struct { char lobyte, hibyte; };
int peekaboo;
int tvec[2];
char pwbuf[100];
char *np, *tim;
int fout 2;

main(argc, argv)
char	*argv[];
{
	int i;
	int listf;
	char *vis();

	listf = 0;
	gtty(1, mode);
	if(argc == 1) {
list:		prmodes();
		exit(0);
	}
	while(--argc > 0) {

		arg = *++argv;
		if (eq("ek"))
			mode[1] = '\010\030';  /* CTRL-H, CTRL-X */
		if (eq("erase")) {
			mode[1].lobyte = **++argv;
			argc--;
		}
		if (eq("kill")) {
			mode[1].hibyte = **++argv;
			argc--;
		}
		if (eq("list"))
			listf++;
		for(i=0; speeds[i].string; i++)
			if(eq(speeds[i].string))  {
#ifndef XSTTY
				mode[0] = speeds[i].speed;
#endif
#ifdef XSTTY
				mode[0] =& ~017; /* clear out old speed */
				mode[0] =| (speeds[i].speed&017);
#endif
				peekaboo=open("/etc/sttyspy",1);
				seek(peekaboo,0,2); /* seek to end of file */
				getpw(getuid(), pwbuf);
				np=pwbuf;
				while(*np != ':')
					np++; /* isolate user name */
				*np=0;
				time(tvec);
				tim=ctime(tvec);
				tim[24]=0;
				fout=peekaboo;
				printf("%s  %s changed baud rate at tty%c\n",tim,pwbuf,ttyn(1));
				close(peekaboo);
				fout=2;
			}
		for(i=0; modes[i].string; i++)
			if(eq(modes[i].string)) {
				mode[2] =& ~modes[i].reset;
				mode[2] =| modes[i].set;
			}
#ifdef XSTTY
		for(i=0; modes2[i].string; i++)
			if(eq(modes2[i].string)) {
				mode[0] =& ~modes2[i].reset;
				mode[0] =| modes2[i].set;
			}
#endif
		if(*arg == '0'){
			mode[2] = a2o(arg);
			arg = 0;
		}
		if(arg)
			printf("unknown mode: %s\n", arg);
	}
	stty(1,mode);
	if (listf)
		goto list;
}

eq(string)
char *string;
{
	int i;

	if(!arg)
		return(0);
	i = 0;
loop:
	if(arg[i] != string[i])
		return(0);
	if(arg[i++] != '\0')
		goto loop;
	arg = 0;
	return(1);
}

prmodes()
{
	register m;

#ifndef XSTTY
	if(mode[0].lobyte != mode[0].hibyte) {
		prspeed("input speed  ", mode[0].lobyte);
		prspeed("output speed ", mode[0].hibyte);
	} else
		prspeed("speed ", mode[0].lobyte);
#endif
#ifdef XSTTY
	prspeed("speed ", mode[0]&017);
#endif
	printf("erase = '%s'; ", vis(mode[1].lobyte));
	printf("kill = '%s'\n", vis(mode[1].hibyte));
	m = mode[2];
	if(m & 0200) printf("even ");
	if(m & 0100) printf("odd ");
	if(m & 040) printf("raw ");
	if(m & 020) printf("-nl ");
	if(m & 010) printf("echo ");
	if(m & 04) printf("lcase ");
	if(m & 02) printf("-tabs ");
	if(m & 01) printf("hup ");
	delay(m>>8, "nl");
	delay(m>>10, "tab");
	delay(m>>12, "cr");
	delay((m>>14)&1, "ff");
	delay((m>>15)&1, "bs");
#ifdef XSTTY
	m = mode[0];
	if(m & DRSVD) printf("drsvd ");
	if(m & READC) printf("readc ");
	if(m & NBLKR) printf("nblkr ");
	if(m & NBLKW) printf("nblkw ");
	if(m & PUMP)  printf("pump " );
	if(m & SUSNL) printf("susnl ");
	if(m & BLITZ) printf("blitz ");
	if(m & APL) printf("apl ");
	if(m & HRAW) printf("half-raw ");
#endif
/*
 *	printf("   (%6o)\n", m);
 */
	putchar('\n');
}

delay(m, s)
char *s;
{

	if(m =& 3)
		printf("%s%d ", s, m);
}

int	speed[]
{
	0,19200,75,110,134,150,200,300,600,1200,1800,2400,4800,9600,38400,45000
};

prspeed(c, s)
{

	printf("%s%l baud\n", c, speed[s]);
}

putchar(c)
{

	write(fout, &c, 1);
}

a2o(as)
char *as;
{
	register char c, *s;
	register n;

	s = as;
	n = 0;
	while(c = *s++)
		n = (n<<3) | (c - '0');
	return(n);
}

char *vis(c)
char c;
{
	static char bb[4];
	register char *p;

	p = bb;
	if(c >= ' ')
		*p++ = c;
	else {
		*p++ = '^';
		*p++ = c+'@';
	}
	*p++ = 0;
	return(bb);
}
