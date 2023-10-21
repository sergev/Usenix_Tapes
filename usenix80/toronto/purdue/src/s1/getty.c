#
#define XSTTY	/* uses Purdue's extended stty calls */
/*
 * getty -- adapt to terminal speed on dialup, and call login
 */

/*
 * tty flags
 */
#define	HUPCL 01
#define	XTABS	02
#define	LCASE	04
#define	ECHO	010
#define	CRMOD	020
#define	RAW	040
#define	ODDP	0100
#define	EVENP	0200
#define	ANYP	0300

/*
 * Delay algorithms
 */
#define	CR1	010000
#define	CR2	020000
#define	CR3	030000
#define	NL1	000400
#define	NL2	001000
#define	NL3	001400
#define	TAB1	002000
#define	TAB2	004000
#define	TAB3	006000
#define	FF1	040000

#define	ERASE	010
#define	KILL	030

/*
 * speeds
 */
#define B75	2
#define	B110	3
#define B134	4
#define	B150	5
#define B200	6
#define	B300	7
#define B600	8
#define B1200	9
#define B1800	10
#define B2400	11
#define B4800	12
#define	B9600	13
#define B19200	1	/* 50 baud on our DH11 is 19.2kb */
#define B38400	14	/* exta is 38.4kb on our DH11 */
#define B0001	14
#define B0002	15

#define	SIGINT	2
#define	SIGQIT	3


struct	sgtty {
	char	sgispd, sgospd;
	char	sgerase, sgkill;
	int	sgflag;
} tmode;

struct	tab {
	int	tname;		/* this table name */
	int	nname;		/* successor table name */
	int	iflags;		/* initial flags */
	int	fflags;		/* final flags */
	int	ispeed;		/* input speed */
	int	ospeed;		/* output speed */
	char	*erase;		/* string to erase screen */
} itab[] {


/* table '-' -- Console TTY 300 baud */
	'-', '-',
	ANYP+RAW+CRMOD+XTABS, ANYP+ECHO+CRMOD+XTABS,
	B300, B300,
	"\r\n\n\n",

/* table '1' -- Silent 700 with 64 char. ASCII */
	'1','1',
	ANYP+RAW+CR2+CRMOD+XTABS, ANYP+ECHO+LCASE+CRMOD+XTABS+CR2,
	B300, B300,
	"\r\n\n\n",

/* table '2' -- 4800 baud Superbee */
	'2', '2',
	ANYP+RAW+CRMOD+XTABS, ANYP+XTABS+ECHO+CRMOD,
	B4800, B4800,
	"\033E\000\000\037",

/*  table '3' -- T.I. Silent 700  */

	'3','3',
	ANYP+RAW+CR2+CRMOD+XTABS,ANYP+ECHO+CRMOD+XTABS+CR2,
	B300, B300,
	"\r\n\n",

/* table '4'  9600 baud Tektronix 4010  */

	'4','4',
	ANYP+RAW+NL1+CRMOD+FF1,ANYP+ECHO+CRMOD+XTABS+LCASE,
	B9600, B9600,
	"\033\014\n\n",

/* table '5' -- 19200 baud Lear Siegler */

	'5','5',
	ANYP+RAW+CRMOD, ANYP+ECHO+CRMOD+XTABS+TAB3,
	B19200, B19200,
	"\033H\033J\032\0\0\0\n\n",

/* table '6' -- 1200 baud Lear Sigler */

	'6','6',
	ANYP+RAW+CRMOD, ANYP+ECHO+CRMOD+XTABS+TAB3,
	B1200, B1200,
	"\033:\033H\033J\032\0\0\0\n\n",

/* table '7' -- 9600 baud Lear Sigler */

	'7','7',
	ANYP+RAW+CRMOD, ANYP+ECHO+CRMOD+XTABS+TAB3,
	B9600, B9600,
	"\033:\033H\033J\032\0\0\0\n\n",

/* table '8' -- Mowle's terminal (1200 baud hp or lear) */

	'8','8',
	ANYP+RAW+CRMOD, ANYP+ECHO+CRMOD+XTABS+TAB3+LCASE,
	B1200, B1200,
	"\033H\033J\032\0\0\0\n\n",

/* table '9' -- 2400 baud Lear Sigler */

	'9','9',
	ANYP+RAW+CRMOD, ANYP+ECHO+CRMOD+XTABS+TAB3,
	B2400, B2400,
	"\033H\033J\032\0\0\0\n\n",

/* table '0' -- 4800 baud Lear Sigler */

	'0','0',
	ANYP+RAW+CRMOD, ANYP+ECHO+CRMOD+XTABS+TAB3,
	B4800, B4800,
	"\033H\033J\032\0\0\0\n\n",

/* table 'a' -- 38400 baud Lear Sigler */

	'a','a',
	ANYP+RAW+CRMOD, ANYP+ECHO+CRMOD+XTABS+TAB3,
	B38400, B38400,
	"\033H\033J\032\0\0\0\n\n",

/* table '%' -- pty */

	'%','%',
	0,0,
	0,0,
	"\033:\033H\033J\032\0\0\0\n\n",
};

#define	NITAB	sizeof itab/sizeof itab[0]

char	name[16];
int	crmod;
int	upper;
int	lower;


struct {
	char *exec;
	char *name;
} nextp[] {

/*
 * Table of startup processors which getty will exec in
 * depending on first digit on /etc/ttys file.. 0 = off,
 * 1 - n is index into below table.
 */

/* off */	"/bin/login",      "login",
/* 1 */		"/bin/login",      "login",
/* 2 */		"/bin/sh",         "-",
/* 3 */		"/etc/down",       "down",
/* 4 */		"/usr/bin/strek",  "STREK - The Final Frontier",
/* 5 */		"/usr/bin/chess",  "C H E S S",
/* 6 */		"/usr/bin/conn",  "conn",
	};

char *login[]
{
	"/bin/login", "login",
};

main(argc, argv)
char **argv;
{
	register struct tab *tabp;
	register tname;

/*
	signal(SIGINT, 1);
	signal(SIGQIT, 0);
*/
	if (argc > 2 && (argv[2][0] < sizeof nextp/4+'0' && argv[2][0] > '0')){
		login[0] = nextp[argv[2][0] - '0'].exec;
		login[1] = nextp[argv[2][0] - '0'].name;
	}

	tname = '0';
	if (argc > 1)
		tname = *argv[1];
	for (;;) {
		for(tabp = itab; tabp < &itab[NITAB]; tabp++)
			if(tabp->tname == tname)
				break;
		if(tabp >= &itab[NITAB])
			tabp = itab;
#ifdef XSTTY
	tmode.sgispd = tabp->ispeed&017; /* only right 4 bits of speed used */
#endif
#ifndef XSTTY
		tmode.sgispd = tabp->ispeed;
		tmode.sgospd = tabp->ospeed;
#endif
		tmode.sgflag = tabp->iflags;
		if(tabp->ispeed != 0) stty(0, &tmode);
		puts(tabp->erase);
		name[0] = 0;
		if(tabp->tname == tabp->nname ||getname()) {
			tmode.sgerase = ERASE;
			tmode.sgkill = KILL;
			tmode.sgflag = tabp->fflags;
			if(crmod)
				tmode.sgflag =| CRMOD;
			if(upper)
				tmode.sgflag =| LCASE;
			if(lower)
				tmode.sgflag =& ~LCASE;
			if(tabp->ispeed != 0) stty(0, &tmode);
			if (login[0] == nextp[2].exec)
				execl(nextp[2].exec, nextp[2].name, 0);
			execl(login[0], login[1], name, 0);
			exit(1);
		}
		tname = tabp->nname;
	}
}

getname()
{
	register char *np;
	register c;
	static cs;

	crmod = 0;
	upper = 0;
	lower = 0;
	np = name;
	do {
		if (read(0, &cs, 1) <= 0)
			exit(0);
		if ((c = cs&0177) == 0)
			return(0);
		write(1, &cs, 1);
		if (c>='a' && c <='z')
			lower++;
		else if (c>='A' && c<='Z') {
			upper++;
			c =+ 'a'-'A';
		} else if (c==ERASE) {
			if (np > name)
				np--;
			continue;
		} else if (c==KILL) {
			np = name;
			continue;
		}
		*np++ = c;
	} while (c!='\n' && c!='\r' && np <= &name[16]);
	*--np = 0;
	if (c == '\r') {
		write(1, "\n", 1);
		crmod++;
	} else
		write(1, "\r", 1);
	return(1);
}

puts(as)
char *as;
{
	register char *s;

	s = as;
	while (*s)
		write(1, s++, 1);
}
