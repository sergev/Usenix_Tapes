177a
			lower = 0;
			upper = 0;
.
173,174c
			if (np > name) {
				c = *--np;
				if (c >= 'a' && c <= 'z')
					lower--;
				else if (c >= 'A' && c <= 'Z')
					upper--;
			}
.
165c
			return(0);	/* break */
.
163c
			exit(0);	/* EOT */
.
105a
	6, '2',
	ANYP+RAW, ANYP+XTABS+ECHO+CRMOD,
	B300, B300,
	"\r\nlogin: ",

};

#define	NITAB	sizeof itab/sizeof itab[0]

char	name[16];
int	crmod;
int	upper;
int	lower;

.
101,104c
	5, '1',
	ANYP+RAW+NL1+CR1, ANYP+XTABS+ECHO+CRMOD+LCASE+CR1,
	B110, B110,
	"\r\nlogin: ",

/* table '2'-6 1200,300 in-house hardcopy terminals */
	'2', 6,
	ANYP+RAW, ANYP+XTABS+ECHO+CRMOD,
	B1200, B1200,
	"\r\nlogin: ",
.
99c
	3, 4,
	ANYP+RAW, ANYP+XTABS+ECHO+CRMOD,
	B1200, B1200,
	"\r\nlogin: ",

	4, 5,
	ANYP+RAW, ANYP+XTABS+ECHO+CRMOD,
	B300, B300,
	"\r\nlogin: ",
.
96,97c
	"\r\nlogin: ",
.
86,94c
/* table '1'-3-4-5 9600,1200,300 DEC terminals & 110 TTY */
	'1', 3,
	ANYP+RAW, ANYP+XTABS+ECHO+CRMOD,
.
82,83c
	ANYP+RAW, ANYP+XTABS+ECHO+CRMOD,
	B9600, B9600,
.
80c
/* table '-' - operator console */
.
78c
	"\r\nlogin: ",
.
76c
	ANYP+RAW+NL1+CR1, ANYP+XTABS+ECHO+CRMOD+LCASE+CR1,
.
71,73c
	ANYP+RAW, ANYP+XTABS+ECHO+CRMOD,
	B300, B300,
	"\r\nlogin: ",
.
66,68c
	ANYP+RAW, ANYP+XTABS+ECHO+CRMOD,
	B1200, B1200,
	"\r\nlogin: ",
.
63c
/* table '0'-1-2 1200,300,110 dialups */
.
39,42c
#define	B110	2
#define	B300	5
#define	B1200	7
#define	B9600	14
.
37c
 * speeds - correct for DZ11
.
9,17c
#define	HUPCL	01	/* signal hang_up on close */
#define	XTABS	02	/* no hardware tabs */
#define	LCASE	04	/* no lower-case */
#define	ECHO	010	/* echo input */
#define	CRMOD	020	/* CR is NEWLINE */
#define	RAW	040	/* no line buffering */
#define	ODDP	0100	/* odd parity */
#define	EVENP	0200	/* even parity */
#define	ANYP	0300	/* no parity */
.
3c
 * getty.c - adapt to terminal speed on dialup, and call login
 *
 *	modified 05-Jun-1980 by D A Gwyn:
 *	1) fixed bug in UC/lc detection;
 *	2) changed terminal-type table for DEC terminals & dialups.
.
w
q
