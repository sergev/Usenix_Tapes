/**************************************************************************
*
* File name:	shdir.c
*
* Author:	Paul Lew, General Systems Group, Inc. Salem, NH
* Created at:	02/17/86  04:39 PM
*
* Description:	This program will take input argument from C shell directory
*		stack and display on screen.  User can optionally select a
*		directory to connect to.
*	
* Environment:	4.2 BSD Unix (under Pyramid OSx 2.5)
*
* Usage:	shdir [-r] [-sxxx] [-bh] [-br] [-bd] [-v] `dirs` <CR>
*		where:
*			-v	display version number
*			-bh	use highlighted space for box
*			-br	use reverse video space for box
*			-bd	use dash char for box
*			-r	use scroll if terminal support
*			-sxxx	select directory xxx. If xxx not found or
*				not specified, directory stack will be
*				displayed to allow for selection.
*
* Update History:
*
*      Date		Description					By
*    --------	------------------------------------------------	---
*    02/17/86	Ver 1.0, Initial version				Lew
*    02/28/86	Ver 1.1, add h, k for going backwards			Lew
*    07/01/86	Ver 2.0, modify to work with termcap			Lew
*    10/06/86	Ver 2.1, port to VAX BSD 4.2				Lew
*    10/09/86	Ver 3.0, add scrolling capability			Lew
*    10/13/86	Ver 3.1, add different options for different box style	Lew
*    10/14/86	Ver 3.2, add string search feature			Lew
*
* Build:	cc -s -o shdir shdir.c -ltermcap
*
***************************************************************************/
#include	<stdio.h>
#include	<sgtty.h>
#include	<ctype.h>
#include	<strings.h>
#include	<sys/file.h>

#define		YES	1
#define		NO	0
#define		EOS	'\0'			/* end of string */
#define		SPACE	' '
#define		BS	'\010'
#define		ESC	'\033'
#define		RETURN	'\015'
#define		NEWLINE	'\012'

#define	when		break; case
#define	otherwise	break;default

char		*Version = "shdir Version 3.2  10/16/86  10:47 AM";
char		*Author =
		 "Paul Lew, General Systems Group   UUCP: decvax!gsg!lew";

FILE		*Termfp;		/* terminal file pointer */

#define		MAXDIR	22		/* not over typical screen size */
int		Maxdir = MAXDIR;	/* max # of directory to display */

#define		MAXBAR	200

char		Bar [MAXBAR];		/* vanilla bar */
char		Topbar [MAXBAR];	/* top bar string */
char		Botbar [MAXBAR];	/* bottom bar string */
char		Vbar [20];		/* vertical bar string */

char		Tab [80];
int		Ntab;
char		Stack [1024];
char		*Dirstack [MAXDIR];	/* pointers to directory entries */
int		Arglen [MAXDIR];	/* lengths of each entry */
int		Dircount = 0;
int		Topsel = 0;		/* start selection index */

/* Curpos_query is a string to be sent to VT100 to get current cursor
   position report.  There is no such field in termcap, you have to hack
   the routine for other terminals since they might have different report
   format */

char		*Curpos_query = "\033[6n"; /* cursor position query */
int		Line;			/* # of lines per screen */
int		Fst_line = 0;		/* if %i specified Fst_line = 1 */

struct	sgttyb	Scd;
int		Sno = 1;		/* first entry start index */

int		Select = NO;		/* YES = select entry */
int		Scroll = NO;		/* YES = use scroll feature if exist */
int		Tabstop = 8;		/* default tab-stop */

char		*Search_string = NULL;	/* select search string */
int		Schlen = 0;		/* search string length */
int		Str_total = 0;		/* total matched search string */
int		Str_idx [MAXDIR];	/* string index */

#define	BOX_REVERSE	1
#define	BOX_HIGHLITE	2
#define	BOX_GRAPH	3
#define	BOX_DASH	4

int		Boxstyle = BOX_GRAPH;	/* default box style */

struct	tcap	{
	char		tc_id [3];	/* key for capability	*/
	unsigned char	tc_delay;	/* # of msec to delay	*/
	char		*tc_str;	/* ptr to Tc_str area	*/
	unsigned char	tc_len;		/* length of tc_str	*/
	};

static	char	Termcap [1024];
static	char	Tstr [1024];		/* buffer for real escape sequence */
static	char	*Tsp = Tstr;		/* pointer to be used by tgetstr */
static	int	Tcap_count = 0;		/* # of entries extracted */

/*---------------- You may want to modify the following ----------------*/

#define		AC	0
#define		AE	1
#define		AS	2
#define		LE	3
#define		ND	4
#define		NL	5
#define		UP	6
#define		SR	7
#define		SC	8
#define		RC	9
#define		CS	10
#define		MR	11
#define		MD	12
#define		ME	13
#define		BAD	14

static	struct	tcap Tcap [] = {
		{ "ac",	0, NULL, 0 },	/* alternate chars		*/
		{ "ae",	0, NULL, 0 },	/* exit alternate char set	*/
		{ "as",	0, NULL, 0 },	/* start alternate char set	*/
		{ "le", 0, NULL, 0 },	/* cursor left (CTRL H)		*/
		{ "nd", 0, NULL, 0 },	/* cursor right			*/
		{ "nl", 0, NULL, 0 },	/* cursor down (new line)	*/
		{ "up", 0, NULL, 0 },	/* cursor up			*/
		{ "sr", 0, NULL, 0 },	/* scroll reverse		*/
		{ "sc", 0, NULL, 0 },	/* save cursor position		*/
		{ "rc", 0, NULL, 0 },	/* restore cursor position	*/
		{ "cs", 0, NULL, 0 },	/* set cursor scroll range	*/
		{ "mr", 0, NULL, 0 },	/* mode reverse video		*/
		{ "md", 0, NULL, 0 },	/* mode dense (highlight)	*/
		{ "me", 0, NULL, 0 },	/* mode end (back to normal)	*/
		{   "", 0, NULL, 0 }
	};

#define	SAVE_CURSOR	fprintf (Termfp, "%s", Tcap[SC].tc_str)
#define	RESTORE_CURSOR	fprintf (Termfp, "%s", Tcap[RC].tc_str)

char	*getenv(), *tgetstr(), *tgoto();
char	*cs_define(), get_ac();
char	*malloc();

/*------------------------------------------------------------07/13/84--+
|									|
|	    M a i n    R o u t i n e    S t a r t s    H e r e		|
|									|
+----------------------------------------------------------------------*/
main (argc, argv)
int	argc;		/* number of argument passed		*/
char	**argv;		/* pointer to argument list		*/
	{
	register int	i, j;
	char		*top = "\t";	/* top of stack string */
	int		size;		/* longest string */

	Termfp = fopen ("/dev/tty", "r+");
	if (proc_arg (argc, argv) == 1) size = make_argv ();
	else size = cp_ptr (argc, argv);

	if (!Dircount) my_exit (0);
	if (Search_string) Topsel = search ();

	Ntab = make_bar (size) / 8;
	fprintf (Termfp, "%s\n", Topbar);

	if (Scroll) j = Topsel;
	else	{
		top = "   Top->";
		j = 0;
		}

	for (i=0; i<Dircount; i++) {
		put_line (j, (j==0) ? top : "\t", "\n");
		if (++j >= Dircount) j = 0;
		}

	fprintf (Termfp, "%s\n", Botbar); fflush (Termfp);

	if (Select && Dircount > 1) {
		if (Tcap[UP].tc_len) my_exit (get_rdata ());
		else my_exit (get_data ());
		}

	my_exit (0);
	}

/*-------------------------------------------------------------07/02/86-+
|									|
|	      make_bar : make all the graphical bar strings		|
|									|
+----------------------------------------------------------------------*/
make_bar (size)
int	size;
	{
	register int	i;
	char		*p;
	char		hbar = get_ac('q');

	size = ((size + 5)/Tabstop + 1) * Tabstop;
	for (i=0; i<size-1 && i<MAXBAR; i++)
		Bar[i] = hbar;		/* create bar string */
	Bar[i] = EOS;

	switch (Boxstyle) {
	    case BOX_GRAPH:
		if (Tcap[AC].tc_len) break;
		Boxstyle = BOX_HIGHLITE;
	    case BOX_HIGHLITE:
		if (Tcap[MD].tc_len && Tcap[ME].tc_len) break;
		Boxstyle = BOX_REVERSE;
	    case BOX_REVERSE:
		if (Tcap[MR].tc_len && Tcap[ME].tc_len) break;
	    default:
		Boxstyle = BOX_DASH;
	    }

	if (Boxstyle == BOX_GRAPH) {
		sprintf (Topbar, "\015\t%s%c%s%c%s", Tcap[AS].tc_str,
			 get_ac('l'), Bar, get_ac('k'), Tcap[AE].tc_str);
		sprintf (Vbar, "%s%c%s", Tcap[AS].tc_str, get_ac('x'),
			 Tcap[AE].tc_str);
		sprintf (Botbar, "\t%s%c%s%c%s", Tcap[AS].tc_str,
			 get_ac('m'), Bar, get_ac('j'), Tcap[AE].tc_str);
		}
	else if (Boxstyle == BOX_REVERSE || Boxstyle == BOX_HIGHLITE) {
		while (--i >= 0) Bar[i] = SPACE;
		p = Tcap [(Boxstyle == BOX_HIGHLITE) ? MD : MR].tc_str;
		sprintf (Topbar, "\015\t%s %s %s",
			 p, Bar, Tcap[ME].tc_str);
		sprintf (Vbar, "%s %s", p, Tcap[ME].tc_str);
		strcpy (Botbar, Topbar);
		}
	else	{
		sprintf (Topbar, "\015\t+%s+", Bar);
		sprintf (Vbar, "|");
		strcpy (Botbar, Topbar);
		}
	return (size);
	}

/*-------------------------------------------------------------07/01/86-+
|									|
|		 get_rdata : get raw input and process it		|
|									|
+----------------------------------------------------------------------*/
get_rdata ()
	{
	register int	i, j, n;
	int		x;		/* 0-9 index for next position */
	char		c, buf[80];
	int		dir, sel;
	int		fd = fileno (Termfp);

	rawio (fd, &Scd);
	fputc (RETURN, Termfp);

	if (Scroll) {			/* use scroll region */
		sel = scroll_sel (fd);
		i = 0; goto eos;
		}
	move_many (Dircount+1-Topsel, UP); /* back to top */
	move_many (13, ND);

	for (i=Topsel; ;) {		/* selection loop */
		read (fd, &c, 1);
		n = i;  x = 0;
		switch (c & 0x7f) {
			case '9': case '8': case '7': case '6':	case '5':
			case '4': case '3': case '2': case '1': case '0':
					x = (c & 0x7f) - '0';
					if (x >= Dircount) continue;
					i = figure_dir (x, i);
					break;
			case 'n':	i = find_idx (1, i);	/* next */
					break;
			case 'p':	i = find_idx (-1, i);	/* prev */
					break;
			case 'h': case 'H': case 'k': case 'K':
			case BS:	if (--i < 0) i = Dircount-1;
					break;
			case RETURN:	sel = i; goto eos;
			case SPACE:
			default:	if (++i >= Dircount) i = 0;
					break;
			}
		if (n == i) continue;
		if (n > i) { n = n - i;  dir = UP; }
		else { n = i - n;  dir = NL; }

		/* move to next field */
		strcpy (buf, " \010");
		for (j=0; j<n; j++) strcat (buf, Tcap[dir].tc_str);
		strcat (buf, "-\010");
		fprintf (Termfp, "%s", buf);
		fflush (Termfp);
		}
eos:
	move_many (Dircount-i+1, NL);
	fputc (RETURN, Termfp);
	fflush (Termfp);
	resetio (fd, &Scd);
	return (sel);
	}

/*-------------------------------------------------------------10/09/86-+
|									|
|		put_line : put a line on screen to scroll		|
|									|
+----------------------------------------------------------------------*/
put_line (i, lead, trail)
int	i;				/* index to Dirstack */
char	*lead;				/* leading string */
char	*trail;				/* trailing string */
	{
	register int	n, j;

	n = Ntab - (Arglen[i]+5+1)/Tabstop;
	for (j=0; j<n; j++) Tab[j] = '\t'; Tab[j] = EOS;
	fprintf (Termfp, "%s%s %2d: %s%s%s%s",
		lead, Vbar, i, Dirstack[i], Tab, Vbar, trail);
	}

/*-------------------------------------------------------------10/09/86-+
|									|
|		scroll_sel : scroll to show the selection		|
|									|
+----------------------------------------------------------------------*/
scroll_sel (fd)
int	fd;
	{
	register int	idx;		/* index to Dirstack of top element */
	register int	target;		/* target index */
	register int	move_cnt;	/* # of up/down to move */
	int		topline;	/* top line number of the box */
	int		botline;	/* bottom line # of the box */
	int		ld;		/* last digit (0-9) */
	char		c;

	/* find out where is the cursor and define the scroll region
	   accordingly. Note this should be done AFTER the directory
	   stack dislpayed since you do not know where the original
	   screen boundaries defined to be. */

	botline = get_curline (fd) - 2;
	topline = botline - Dircount + 1;		/* define region */
	fprintf (Termfp, "%s%s%s", Tcap[SC].tc_str,
		 cs_define (topline, botline), Tcap[RC].tc_str);

	move_many (Dircount+1, UP);
	move_many (13, ND);

	for (idx=Topsel, move_cnt=1; ; move_cnt=1) {
		read (fd, &c, 1);	/* get user key stroke */
		c &= 0x7F;
		switch (c) {
		    case RETURN:
			goto eofor;

		    case 'n':
			target = find_idx (1, idx);
			goto move;
		    case 'p':
			target = find_idx (-1, idx);
			goto move;
		    case '9': case '8': case '7': case '6': case '5':
		    case '4': case '3': case '2': case '1': case '0':
			ld = c - '0';
			if (ld >= Dircount) continue;
			target = idx;
			do	{
				if (++target >= Dircount) target = 0;
				} while (target % 10 != ld);
move:			move_cnt = target - idx;
			if (move_cnt < 0) move_cnt += Dircount;
			if (move_cnt > Dircount/2) {
				move_cnt = Dircount - move_cnt;
				goto up;
				}
			goto down;

		    case 'k': case 'K': case 'h': case 'H': case BS:
			/* scroll down one entry */
up:			SAVE_CURSOR;
			while (move_cnt-- > 0) {
				move_many (1, SR);
				if (--idx < 0) idx = Dircount - 1;
				put_line (idx, "\r\t", "\r");
				}
			RESTORE_CURSOR;
			break;

		    case 'j': case 'J': case 'l': case 'L': case SPACE:
		    default:	/* scroll up one entry */
down:			SAVE_CURSOR;
			move_many (Dircount-1, NL);
			while (move_cnt-- > 0) {
				move_many (1, NL);
				put_line (idx, "\r\t", "\r");
				if (++idx >= Dircount) idx = 0;
				}
			RESTORE_CURSOR;
			break;
		    }
		fflush (Termfp);
		}
eofor:
	/* restore scrolling region to full screen (assumed the
	   original state is so) */
	fprintf (Termfp, "%s%s%s", Tcap[SC].tc_str,
 		 cs_define (Fst_line, Line-1+Fst_line),
		 Tcap[RC].tc_str);		/* reset region */
	fflush (Termfp);
	return (idx);
	}

/*-------------------------------------------------------------10/09/86-+
|									|
| cs_define : return a string contains cursor scroll region definition	|
|									|
+----------------------------------------------------------------------*/
char	*
cs_define (from, to)
int	from;				/* start line number */
int	to;				/* end line number */
	{
	static char	cs_buf [20];
	static int	cs_from = 0;
	static int	cs_to = 0;
	static char	tc1, tc2;
	register char	*p;
	register int	i;

	if (cs_from == 0) {
		p = Tcap[CS].tc_str;
		i = 0;
		while (*p != EOS) {
			if (strncmp (p, "%i", 2) == 0) {
				Fst_line = 1;
				p += 2;
				continue;
				}
			if (strncmp (p, "%d", 2) == 0) {
				if (cs_from == 0) cs_from = i;
				else cs_to = i;
				i += 3;		/* 3 digits reserved */
				p += 2;
				continue;
				}
			cs_buf [i++] = *p++;
			}
		cs_buf [i] = EOS;
		tc1 = cs_buf [cs_from + 3];
		tc2 = cs_buf [cs_to + 3];
		}
	sprintf (&cs_buf[cs_from], "%03d", from);
	cs_buf [cs_from + 3] = tc1;

	sprintf (&cs_buf[cs_to], "%03d", to);
	cs_buf [cs_to + 3] = tc2;

	return (cs_buf);
	}

/*-------------------------------------------------------------10/09/86-+
|									|
|	    get_curline : get current cursor line number		|
|									|
+----------------------------------------------------------------------*/
get_curline (fd)
int	fd;
	{
	register int	i, gotesc = NO;
	char		c;
	int		line, column;
	char		buf[80];

	/* Ask terminal to report its cursor position */

	fprintf (Termfp, "%s", Curpos_query); fflush (Termfp);
	for (i=0; i<80-1; i++) {
loop:		read (fd, &c, 1);
		c &= 0x7F;
		if (gotesc == NO) {
			if (c != ESC) goto loop;
			gotesc = YES;
			}
		if ((buf[i] = c) == 'R') {	 /* end of report */
			buf[++i] = EOS;
			break;
			}
		}
	sscanf (buf, "\033[%d;%dR", &line, &column);	/* VT100 only */
	return (line);
	}

/*-------------------------------------------------------------10/09/86-+
|									|
|     move_many : repeatedly write certain string to output device	|
|									|
+----------------------------------------------------------------------*/
move_many (n, code)
int	n;				/* repeat count */
int	code;				/* index to Tcap structure */
	{
	register int	i;

	for (i=0; i<n; i++) {
		fprintf (Termfp, "%s", Tcap[code].tc_str);
		}
	fflush (Termfp);
	}

/*-------------------------------------------------------------10/14/86-+
|									|
|	     my_exit : close opened terminal file then exit 		|
|									|
+----------------------------------------------------------------------*/
my_exit (n)
int	n;
	{
	if (Termfp) fclose (Termfp);
	exit (n);
	}

/*-------------------------------------------------------------07/01/86-+
|									|
|   get_data : get data for terminal with no cursor addr capability	|
|									|
+----------------------------------------------------------------------*/
get_data ()
	{
	char		buf [20];
	register int	i = 0;

	printf ("Please Select [0-%d]: ", Dircount-1);
	gets (buf);
	if (strlen (buf)) {
		i = atoi (buf);
		if (i < 0 || i > Dircount-1) i = 0;
		}
	return (i);
	}

#define	PADDING	'\200'
/*-------------------------------------------------------------05/10/86-+
|									|
|	   tcap_init : initialize termcap data structure		|
|									|
+----------------------------------------------------------------------*/
tcap_init ()
	{
	register int	i;
	struct	tcap	*p;
	char		*tp;
	unsigned int	delay;
	int		status;
	char		*termtype = getenv ("TERM");

	if ((status = tgetent (Termcap, termtype)) != 1) {
		if (status == 0) {
			fprintf (stderr, "No entry for %s in termcap\r\n",
				 termtype);
			}
		else	fprintf (stderr, "Can not open termcap file\r\n");
		my_exit (1);
		}

	for (p= &Tcap[0]; strlen (p->tc_id) > 0; p++) {
		tp = tgetstr (p, &Tsp);
		if (tp == NULL) tp = "";	 /* no such capability */
		delay = 0;
		while (isdigit (*tp)) {
			delay = delay*10 + (*tp++) - '0';
			}
		p->tc_delay = delay;
		p->tc_len = strlen (tp);
		if (delay) {
			p->tc_str = malloc (p-> tc_len + delay);
			strcpy (p->tc_str, tp);
			tp = p->tc_str + p->tc_len;
			for (i=0; i<delay; i++) *tp++ = PADDING;
			*tp = EOS;
			p->tc_len += delay;
			}
		else	p->tc_str = tp;
		Tcap_count++;
		}

	Line = tgetnum ("li");
	tcap_special ();
	}

/*-------------------------------------------------------------07/02/86-+
|									|
|		tcap_special : special initialization			|
|									|
+----------------------------------------------------------------------*/
tcap_special ()
	{
	if (Tcap[NL].tc_len == 0) {
		Tcap[NL].tc_str = "\n";
		Tcap[NL].tc_delay = 0;
		Tcap[NL].tc_len = 1;
		}
	}

/*-------------------------------------------------------------07/01/86-+
|									|
|  get_ac : get alternate character for a given VT100 alternate char	|
|									|
+----------------------------------------------------------------------*/
char
get_ac (c)
char	c;
	{
	register char	*p;
	char		oc;

	if (Tcap_count == 0) tcap_init ();
	for (p=Tcap[AC].tc_str; *p != EOS; p += 2) {
		if (*p == c) return *(p+1);
		}
	switch (c) {
		case 'j':		/* lower right corner	*/
		case 'k':		/* upper right corner	*/
		case 'l':		/* upper left corner	*/
		case 'm':  oc = '+';	/* lower left corner	*/
			   break;
		case 'q':  oc = '-';	/* horizontal bar */
			   break;
		case 'x':  oc = '|';	/* vertical bar */
			   break;
		default:   oc = '?';	/* bad choice */
		}
	return (oc);
	}

/*-------------------------------------------------------------02/18/86-+
|									|
|	      figure_dir : figure out which direction to go		|
|									|
+----------------------------------------------------------------------*/
figure_dir (x, i)
int	x;				/* target index (0-9) */
int	i;				/* curent index (0-Dircount) */
	{
	int	n;

	n = i % 10;
	if (x == n) {
		if (i + 10 < Dircount) return (i+10);
		}
	if (x > n) {
		if (i - n + x < Dircount) return (i - n + x);
		}
	else	{
		if (i - n + x + 10 < Dircount) return (i - n + x + 10);
		}

	return (x);
	}

/*-------------------------------------------------------------02/17/86-+
|									|
|	     cp_ptr : copy all the argv pointers to Dirstack		|
|									|
+----------------------------------------------------------------------*/
cp_ptr (argc, argv)
int	argc;
char	**argv;
	{
	register int	i, j;
	int		n;
	int		size = 0;

	for (j=0, i=Sno; i<argc; i++) {
		if (j >= Maxdir) {
			fprintf (stderr, "Max %d entries, extra ignored\n",
				 Maxdir);
			fflush (stderr);
			break;
			}
		Dirstack[j] = argv[i];			/* save start addr */
		Arglen[j++] = n = strlen (argv[i]);	/* and length */
		if (n > size) size = n;
		}
	Dircount = j;
	return (size);			/* return the longest length */
	}

/*-------------------------------------------------------------02/17/86-+
|									|
|		    make_argv : make an argument list			|
|									|
+----------------------------------------------------------------------*/
make_argv ()
	{
	char		*p = Stack;
	char		*op = Stack;	/* old pointer */
	char		c;
	register int	i = 0;
	int		size = 0;
	int		n;

	gets (Stack);
	while ((c = *p) != NEWLINE && c != EOS) {
		if (c == SPACE) {
			*p = EOS;
			Dirstack[i] = op;
			Arglen[i++] = n = p - op;
			if (n > size) size = n;
			op = p+1;
			}
		p++;
		}
	if (i > Maxdir) {
		fprintf (stderr, "Max %d entries, extra ignored\n", Maxdir);
		fflush (stderr);
		i = Maxdir;
		}
	Dircount = i;
	return (size);
	}
/*-------------------------------------------------------------10/14/86-+
|									|
|	    find_idx : find next/prev index from Str_idx array		|
|									|
+----------------------------------------------------------------------*/
find_idx (dir, n)
int	dir;				/* direction: 1=next, -1=prev */
int	n;				/* current index */
	{
	register int	i;

	for (i=0; i<Str_total; i++) {
		if (Str_idx[i] == n) {		/* found current, locate next */
			i += dir;
			if (i == Str_total) i = 0;
			if (i < 0) i = Str_total - 1;
			return (Str_idx[i]);
			}
		}
	return (n);
	}

/*------------------------------------------------------------07/12/84--+
|									|
|	proc_arg : routine to process command arguments (options)	|
|									|
+----------------------------------------------------------------------*/
proc_arg (argc, argv)
int	argc;
char	**argv;
	{
	register int	i;
	register int	n;
	char		*p;

	p = getenv ("TABSTOP");
	if (p != NULL) {		/* set tabstop to n (default 8) */
		n = atoi (p);
		if (n > 0) Tabstop = n;
		}
	for (n=i=0; i<argc; i++) {
		if (argv[i][0] != '-') {
			n++;		/* count non-option argument */
			continue;
			}
		Sno++;
		switch (argv[i][1]) {
		    when 's':	p = &argv[i][2];
				if ((Schlen = strlen (p)) == 0) Select = YES;
				else Search_string = p;

		    when 'r':	tcap_init ();
				if (Tcap[SC].tc_len != 0 &&
				    Tcap[RC].tc_len != 0 &&
				    Tcap[CS].tc_len != 0) Scroll = YES;

		    when 'b':	box_style (argv[i][2]);

		    when 'v':	printf ("%s\n%s\n", Version, Author);
				my_exit (0);

		    otherwise:	Sno--;		/* dont count */
		    }
		}
	return (n);
	}

/*-------------------------------------------------------------10/14/86-+
|									|
|   search : find string in dir stack and exit or return found index	|
|									|
+----------------------------------------------------------------------*/
search ()
	{
	register int	i, j;
	register char	*p;
	int		len;
	int		sel;

	/* If argument starts with a number, it will be used as index to
	   directory stack.  If not, it will be used as string to search */

	sel = atoi (Search_string);
	if (0 < sel && sel < Dircount) {	 /* number used as index */
match:		puts (Dirstack[sel]);
		my_exit (sel);
		}
	for (Str_total=i=0, sel = -1; i<Dircount; i++) {
		len = Arglen[i];
		p = Dirstack[i];
		for (j=0; j<=len-Schlen; j++) {
			if (strncmp (Search_string, p+j, Schlen) == 0) {
				Str_idx[Str_total++] = i;
				if (sel == -1) sel = i;
				break;
				}
			}
		}
	if (Str_total == 1) goto match;
	Select = YES;
	return ((sel == -1) ? 0 : sel); /* more than one, stay around */
	}

/*-------------------------------------------------------------10/14/86-+
|									|
|		    box_style : select box style			|
|									|
+----------------------------------------------------------------------*/
box_style (style)
char	style;				/* style */
	{
	switch (style) {
		when 'h':	Boxstyle = BOX_HIGHLITE;
		when 'r':	Boxstyle = BOX_REVERSE;
		when 'g':	Boxstyle = BOX_GRAPH;
		otherwise:	Boxstyle = BOX_DASH;
		}
	}

int		Old_flags;
int		Pseudo = -1;		/* 0/1 = normal/pseudo terminal */
/*------------------------------------------------------------07/10/84--+
|									|
|	   rawio : oepn terminal in RAW mode without ECHO		|
|									|
+----------------------------------------------------------------------*/
rawio (fd, mp)
int		fd;			/* file descriptor */
struct	sgttyb	*mp;			/* ptr to mode structure */
	{
	char	*tname = (char *) ttyname (fileno(stdout)) + 8;
					/* /dev/ttypxxx	*/
					/* 012345678	*/

	if (*tname == 'p' || *tname == 'q' || *tname == 'r') {
		/* pseudo terminal, delay a while so that output will
		   drain...   */
		Pseudo = YES;
		sleep (1);
		}
	else	Pseudo = NO;

	ioctl (fd, TIOCGETP, mp);
	Old_flags = mp-> sg_flags;	/* save old setting */
	mp-> sg_flags |= RAW;
	mp-> sg_flags &= ~ECHO;
	ioctl (fd, TIOCSETP, mp);
	}

/*------------------------------------------------------------07/10/84--+
|									|
|	resetio : close terminal and restore original setting		|
|									|
+----------------------------------------------------------------------*/
resetio (fd, mp)
int		fd;			/* file descriptor */
struct	sgttyb	*mp;			/* ptr to old setting */
	{
	if (Pseudo) sleep (1);

	mp-> sg_flags = Old_flags;
	ioctl (fd, TIOCSETP, mp);	/* restore original setting */
	}
