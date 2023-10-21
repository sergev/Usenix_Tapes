#
/*
	DECLIST - an output optimizer for DECwriters(r)
	or other terminals with working backspaces and linefeeds.  

	Written by Brad Blasing on 77/04/07,
	University of Minnesota.

	Last modification - 78/03/27.

	Calling sequence:
		used as a filter: 	<cmd> | decl [-opts]
		used to list a file: 	decl [-opts] filename

	-opts is an optional string of option characters.  The legal
	options are:

		e - do not eject the last page when done.
		o - middle of the road optimization - try it.
		m - supress statistics message.

	DECwriter is a registered trademark of the Digital
	Equipment Corporation; Maynard, Mass.

*/

#define LINESIZE	300	/* maximum line size + 1 */
#define INFINITY	4000
#define BKSP		'\010'
#define EJECT		'\013'	/* Page eject character (VTAB) */
#define LINP		66	/* lines per page */
#define CRMOD		00020	/* cr<>lf mapping option for xstty */
#define XTABS		00002	/* Tab expansion */
#define CRTBS		01000	/* Crt backspace mode */

/*
 *	Global variables:
 */
char	tab_size	8;
char	line[LINESIZE]	{'*'};	/* Line buffer */
char	end_ch;			/* Ending character of a line */
int	line_len;		/* Current line length */
int	ph_pos		1;	/* Print-head position */
int	ch_read 	0;	/* Chars. read */
int	ch_write	0;	/* Chars. written */
int	line_num	1;	/* Line number */
int	page_num	1;	/* Page number */
char	m_opt		1;	/* Statistics message option */
char	e_opt		1;	/* Eject last page option */
char	o_opt		0;	/* Funny optimization option */
int	tab_on;			/* Tab expansion */
char	tty[]	{"/dev/ttyx"};	/* The teletype */
int	bad_ch		0;	/* Bad char. encountered in line */
int	inode[18];		/* Argument for fstat */

struct {	/* Info. passed between findbestprintpos and printline */
	int	pos;	/* Best print position */
	int	first;	/* Num. of chars. in first 1/2 of line */
	int	num;	/* Num. of chars. this line will take to print */
} best;

/*
 *  structure for interface with xstty and xgtty
 */

struct  xgs  {
	int	x_speeds;	/* output+input line speed */
	int	x_flags;	/* mode, see below */
	int	x_delay;	/* delay bits */
	char	x_erase;	/* erase character */
	char	x_kill;		/* line delete character */
	char	x_intr;		/* interrupt character */
	char	x_quit;		/* quit character */
	char	x_wake;		/* wake up character */
	char	x_null;		/* unused at present */
	char	x_tabs;		/* tab size */
	char	x_delput;	/* used by xsgtty */
	int	x_inst;		/* reserved for installation */
};

struct	xgs	arg;	/* Argument for xgtty/xstty */



tab(p)
	int	p;
/*
	Tab tab's to print-head position p.  (The first column is #1)
	Tab should never be called to tab to column p > ph_pos nor
	to 2 <= p < (ph_pos+1)/2.
*/
{
	register	int	i;
	if(p == 1) {
		putchar('\r');
		ph_pos = 1;
	} else {
		for(i = ph_pos; i > p; --i) putchar(BKSP);
		ph_pos = p;
	}
}



lfeed(n)
	int	n;
/*
	Lfeed advances the paper by n lines.
*/
{
	if(n == -1) n = LINP - line_num + 1;
	ch_read  =+ n * 2;	/* CR + LF */
	ch_write =+ n;		/* LF only */
	line_num =+ n;
	if(line_num > LINP) {
		line_num =- LINP;
		++page_num;
	}
	while(n--) putchar('\n');
}



puts(ptr,num)
	char	*ptr;
	int	num;
/*
	Puts puts a string from ptr for num characters onto
	standard output.  Puts returns the new print-head
	position resulting from printing the string.
*/
{
	register	char	*p;
	register	int	n;
	register	int	pp;

	p = ptr;  n = num;  pp = ph_pos;
	while(n--) {
		if(*p >= ' ' && *p != '\177')
			++pp;
		else
			if(*p == BKSP) --pp;
			else if(*p == '\r') pp = 1;
			/* else, non-printing char. */
		putchar(*p++);
	}
	return(pp);
}



readline()
/*
	Readline reads one line (up to a '\n' or '\0') into
	the line buffer.  It returns 0 when EOF is read.
*/
{
	register	int	i;
	register	char	ch;
	char	t;

	i = 1;
	bad_ch = 0;
	while((ch = getchar()) != '\n' && ch != '\0' && ch != EJECT) 
		if(i < LINESIZE)
			if(ch == '\t' && tab_on) { 
				t = tab_size - (i - 1) % tab_size;
				if(i + t > LINESIZE) t = LINESIZE - i;
				while(t--) line[i++] = ' ';
			} else {
				line[i++] = ch;
				if(ch < ' ')
					bad_ch++;
			}
	end_ch = ch;
	while(line[--i] == ' ');
	line_len = i;
	ch_read =+ i;
	return(ch != '\0' || i != 0);
}



findbestprintpos()
/*
	Findbestprintpos selects the print-head position
	which will allow the current line to be printed
	with the fewest characters.  The data for this
	position is stored in the structure `best'.
*/
{
	register	int	f,n,p;
	int	start,end;

/*
 *	If bad character was read, cop-out and print the
 *	line from position 1.
 */
	if(bad_ch) {
		best.pos   = 1;
		best.first = 0;
		best.num   = line_len + 1;
		return;
	}
/*
 *	Calculate range of positions to check:
 */
	start = ph_pos > 4 ? (ph_pos + 1) >> 1 : 2;
	end = ph_pos > line_len ? line_len : ph_pos;
/*
 *	Check position 1 if necessary:
 */
	if(o_opt && start <= end && line_len >= ph_pos)
			/* Forget about position 1 */
		best.num = INFINITY;
	else {		/* Check position 1 */
		best.pos   = 1;
		best.first = 0;
		best.num   = line_len + 1;
	}
/*
 *	Calculate and keep num. of chars. needed to print
 *	first part of line (pos. 1 thru p-1).
 */
	for(f = start - 1 ; line[f] == ' ' ; --f);
/*
 *	Find the position which will allow the line to be
 *	cheapest.  Only positions from p to line_len are checked.
 */
	for(p = start ; p <= end ; ++p) {
		/* Update first 1/2 line length. */
		if(line[p - 1] != ' ') f = p - 1;
		/* Calculate total character count. */
		n = ph_pos - p;		/* tab to p */
		n =+ line_len - p + 1;	/* second 1/2 of line */
		if(f != 0) n =+ f + 1;	/* first 1/2 of line */
		/* Choose this pos. if it is cheapest. */
		if(n < best.num) {
			best.pos   = p;
			best.num   = n;
			best.first = f;
		}
	}
}



printline()
/*
	Printline takes the current line and prints it
	according to the info. left in `best' by
	findbestprintpos.
*/
{
	tab(best.pos);
	/* Print second 1/2 of line. */
	ph_pos = puts(&line[best.pos],line_len - best.pos + 1);
	/* Print first 1/2 of line. */
	if(best.first != 0) {
		tab(1);
		ph_pos = puts(&line[1],best.first);
	}
	ch_write =+ best.num;
}



main(argc,argv)
	char	**argv;
{
	register	char	*p;
	register	char	ch;
	register	int	i;
	extern		int	fin,fout;
	extern		int	clean_up();
	int	save;

/*
 *	Process arguments:
 */
	if(argc > 1 && argv[1][0] == '-') {
		p = argv[1] + 1;
		while((ch = *p++) != '\0') {
			if(ch == 'o') o_opt = !o_opt;
			else if(ch == 'e') e_opt = !e_opt;
			else if(ch == 'm') m_opt = !m_opt;
			else {
				fout = 2;
				printf(" Bad argument `%c'.\n",ch);
				exit(1);
			}
		}
		++argv; --argc;
	}
/*
 *	Process file name if present:
 */
	if(argc > 1) {
		p = argv[1];
		if((fin = open(p,0)) < 0) {
			fout = 2;
			printf(" Can't open %s.\n",p);
			exit(1);
		}
	}
	fout = dup(1);		/* Buffer standard output. */
/*
 *	Do a stty to turn off CR<>LF mapping and CRT mode.
 */
	xgtty(2,&arg);
	i = arg.x_flags;
	tab_on = i & XTABS;
	tab_size = arg.x_tabs;
	arg.x_flags =& ~(CRMOD | CRTBS);
	xstty(2,&arg);
	arg.x_flags = i;	/* Reset arg to origonal value. */
/*
 *	Catch interrupts:
 */
	signal(2,clean_up);
	signal(3,clean_up);
/*
 *	Turn off write permission on teletype. (if possible)
 */
	if((tty[8] = ttyn(1)) != 'x') {
		fstat(1, inode);
		i = inode[2] & 07744;
		chmod(tty, i);
	}
/*
 *	Main loop:
 */
	while(readline()) {
		if(line_len) {
			findbestprintpos();
			printline();
		}
		if(end_ch == EJECT)
			lfeed(-1);
		else
			lfeed(1);
	}
/*
 *	Finish up:
 */
	tab(1);
	if(e_opt && line_num > 1) lfeed(-1);
	if(line_num == 1) --page_num;
	flush(fout);
	if(m_opt) {
		fout = 2;
		save = 1000 - ldiv(hmul(ch_write, 1000),
			ch_write*1000, ch_read);
		printf("\n Declist - %d page", page_num);
		if(page_num > 1) putchar('s');
		printf(", %d.%d%% savings.\n\r", save / 10,save % 10);
		flush(fout);
	}
	xstty(2,&arg);	/* Restore TTY. */
	if(tty[8] != 'x')
		chmod(tty, inode[2] & 07777);
	exit(0);
}



clean_up()
/*
	Clean_up cleans up after an interrupt.
*/
{
	signal(2,1);
	signal(3,1);
	xstty(2,&arg);	/* Restore TTY. */
	if(tty[8] != 'x')
		chmod(tty, inode[2] & 07777);
	write(2,"\007\n",2);
	exit(0);
}
