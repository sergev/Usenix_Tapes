static char cpright[] = "(C) Chris Bertin, 1983";

/*
 * Learn how to type ( using cursor control if run on a CRT )
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <curses.h>

#define WORDLIST	"/usr/dict/words"
#define LFCR		"\n\r"
#define CHAR	0	/* answer types */
#define NUM	1	/* ... */
#define PRACTICE 0	/* run types */
#define DICT	1	/* ... */
#define FILEIN	2	/* ... */
#define MAX	128	/* std array size as well as max ASCII */
#define SPEED	0	/* screen locations */
#define SCORE	2	/* ... */
#define TYPE	LINES -12	/* ... */
#define ASK	LINES - 3	/* ... */
#define BOTTOM	LINES - 2	/* ... */
#define ALLCHARS "&*()_+|\\{}:\"~<>?-=[];'`,./!#$%^ ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890 abcdefghijklmnopqrstuvwxyz"

#define moveto(x,y,str) if(CA)move(x,y),clrtoeol();else pr(0,str);
#define dorefresh()	if(CA){clearok(stdscr,TRUE);refresh();clearok(stdscr,FALSE);}
#define sethelp()	wclear(helpscr);wrefresh(helpscr);moveto(TYPE,0,LFCR);

char what_chars[MAX*2];	/* the characters to practice on */
char try_str[MAX*2];	/* the string to be retyped */
char typed_str[MAX*2];	/* the chars being typed by the user */
char work_str[MAX*2];	/* work space (temp string) */
char badchar[MAX];	/* errors; the mistyped chars are pointers into them */
char gbadchar[MAX];	/* same but session total */
int echo_it;		/* echo flag */
int isafastscreen;	/* is a fast screen? */
int trials;		/* number of trials */
int npractice;		/* number of characters to practice on (practice opt.)*/
int lasterr;		/* last error, the next string will start with it */
int linerrors, toterrors, gtoterrors;	/* errors per line / run / session */
float totlngth, gtotlngth;	/* chars typed per run / session */
float tottime, gtottime;	/* time spent per run / session */

struct timeb tb, ta;
struct stat sbuf;
FILE *fp, *dictfp, *fopen();
WINDOW *helpscr, *subwin();

main()
{
	register option, screenloc, lngth, more = 1;
	int out_size, nwords, instruct;
	char fname[MAX];
	float chkstr();

	setbuf(stdin, (char *)NULL);
	ftime(&ta);
	srandom(getpid() + ta.millitm - getuid()); /* should be unique... */
	initscr();
	scrollok(stdscr, FALSE);
	(void) signal(SIGFPE, SIG_IGN);
	if(CA) /* Do we have cursor control?? */
		isafastscreen = highspeed();
	else
		pr(1, "[No cursor addressing, using hard copy mode]\n");
	raw(), noecho();
	moveto(TYPE - 6, 0, "");
	pr(2, "\t\t\tL E A R N  H O W  T O  T Y P E");
	pr(2, "\t\t\t~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	if(instruct = answer(work_str, "Instructions? (y or n)", CHAR) == 1)
		helpscr = subwin(stdscr, 9, COLS, TYPE, 0);
	gtoterrors = gtotlngth = gtottime = 0.;
	zero(gbadchar, sizeof gbadchar);
	while(more) {
		screenloc = TYPE;
		if(instruct)
			help1();
		(void) answer(work_str, "What characters do you want?", CHAR);
		if(! strcmp(work_str, "words")) {
			option = DICT;
			if(dictfp == NULL) /* don't redo it */
				if(((stat(WORDLIST, &sbuf)) < 0) ||
				   ((dictfp = fopen(WORDLIST,"r")) == NULL)){
					note("Can't open %s", WORDLIST);
					continue;
				}
			if(instruct)
				help3();
			nwords = answer(work_str,
				"How many words in each line?", NUM);
			if(nwords > COLS/11)
				note("Set to maximum allowed (%d)",
					(nwords = COLS/11));
		}
		else if(! strcmp(work_str, "file")) {
			option = FILEIN;
			if(instruct)
				help2();
			(void) answer(fname, "Enter file name", CHAR);
			if(! *fname)
				continue;
			if((fp = fopen(fname, "r")) == NULL) {
				note("Can't open %s", fname);
				continue;
			}
		}
		else {
			option = PRACTICE;
			strcpy(what_chars, work_str[0] ? work_str : ALLCHARS);
			npractice = strlen(what_chars);
			if(instruct)
				help4();
			out_size = answer(work_str,
				"How many characters in each string?", NUM);
		}
		if(option != FILEIN) {
			if(instruct)
				help5();
			trials = answer(work_str, "How many trials?", NUM);
		}
		if(instruct)
			help6();
		echo_it = answer(work_str, "Do you want echo?", CHAR);
		if(instruct)
			help7();
		zero(badchar, sizeof badchar);
		lasterr = toterrors = 0;
		totlngth = tottime = 0.;
		if(CA)
			clear();
		moveto(SCORE, 0, LFCR);
		pr(2, "\tNext line: ^C, Quit: ^D, Move Cursor: ^A, Redraw: ^L");
		while(makestr(option, out_size, nwords) >= 0) {
			ioctl(0, TIOCFLUSH, 0);
			if(CA)
				note("", "");
			if((lngth = chkstr(screenloc)) == 0)
				continue;
			totlngth += lngth;
			toterrors += linerrors;
			if(isafastscreen)
				score(badchar, toterrors, totlngth, tottime);
			if((screenloc += 3) > (ASK - 2))
				screenloc = TYPE;
		}
		gtotlngth += totlngth;
		gtoterrors += toterrors;
		if( ! CA || ! isafastscreen)
			score(badchar, toterrors, totlngth, tottime);
		more = answer(work_str, "Try again?", CHAR);
	}
	leave();
}

answer(str, msg, type)
register char *str;
char *msg;
{
	register c, i, ret = 0;

	for(;;) {
		moveto(ASK, 10, "\n\r> ");
		pr(1, "%s: ", msg);
		for(i=0; ((c = getch()) != '\n' && c != '\r') ;)
		    switch(c) {
			case '\f':
				dorefresh();
				break;
			case '\10':
				if(i>0)
					--i, pr(1, "\010 \010");
				break;
			case '\04':
			case '\03':
			case 0177:
				leave();
			default:
				pr(1, "%c", (str[i++] = noctrl(c)));
		    }
		if(CA)
			note("", "");
		while(str[--i] == ' ') ;
		str[++i] = 0;
		if(type == CHAR)
			if(*str == 'y')
				return(1);
			else
				return(0);
		if((ret = atoi(str)) <= 0) {
			note("Numeric value (> 0) required", "");
			continue;
		}
		return(ret);
	}
}

makestr(opt, lng, n_of_wrds) /* builds the string to be typed */
{
	register ind;

	if(opt != FILEIN && trials <= 0)
		return(-1);
	zero(try_str, sizeof try_str);
	switch(opt) {
	case FILEIN:
		if(fgets(try_str, sizeof try_str, fp) == NULL)
			return(fclose(fp));
		fixstr(try_str);
		return(1);
	case DICT:
		for(ind = 0; ind < n_of_wrds ; ++ind, strcat(try_str, " ")) {
			if(fseek(dictfp, random()%(long)sbuf.st_size, 0) == -1)
				strcpy(work_str, "fseek failed!!!");
			fgets(work_str, sizeof work_str, dictfp);
			if(fgets(work_str, sizeof work_str, dictfp) == NULL)
				strcpy(work_str, "End of file!!!");
			work_str[strlen(work_str) - 1] = 0;
			strcat(try_str, work_str);
		}
		return(--trials);
	case PRACTICE:
		for(ind = 0 ; ind < lng ; ++ind)
			try_str[ind] = what_chars[(int)random() % npractice];
		try_str[0] = (lasterr ? lasterr : try_str[0]);
		lasterr = 0;
		return(--trials);
	}
	/* NOTREACHED */
}

fixstr(str)
char *str;
{
	register char *in, *out;
	short space = 0;

	for(in = str ; isspace(*in) ; ++in) ;
	for(out = str ; *in ; ++in) {
		if(*in == '\010') {
			if(out > str)
				out--;
			continue;
		}
		if(isspace(*in) && space)
			continue;
		space = isspace(*in);
		*out++ = noctrl(*in);
	}
	*out = 0;
}

float
chkstr(location) /* checks the typing */
{
	register ind, c = 0;
	register char *typed_pnt;
	register float n_of_chars;
	float f, elapsed, starttime, gettime();

	zero(typed_str, sizeof typed_str);
	typed_pnt = typed_str;
	if(strlen(try_str) >= COLS-1) {
		note("Line too long, truncated", "");
		try_str[COLS-1] = 0;
	}
	if((n_of_chars = (float) putline(try_str, location)) == 0.)
		return(0.);
	moveto(location+1, 0, LFCR);
	refresh();
	for(linerrors = ind = 0; ind < n_of_chars ; ++ind) {
		switch(c = getch()) {
		  case '\01':
			*typed_pnt = '^';
			*(typed_pnt+1) = 0;
			(void) putline(typed_str, location+1);
			printw("%c", '\10');
			refresh(); /* fall through */
		  case '\n':
		  case '\r':
			--ind;
			break;
		  case '\f':
			dorefresh();
			--ind;
			break;
		  case '\03':
			if((n_of_chars = ind) == 0) /* force out */
				return(0.);
			break;
		  case '\04':
		  case 0177:
			gtotlngth += totlngth + ind;
			gtoterrors += toterrors + linerrors;
			leave();
		  default: /* a character... */
			if(ind == 0)
				starttime = gettime(0, "");
			if(c != try_str[ind]) { /* error */
				if(echo_it)
					pr(0, "%c", ch_case(c, try_str[ind]));
				else {
#ifdef SHOWALL
					*typed_pnt++ = ch_case(c, try_str[ind);
#else
					*typed_pnt++ = noctrl(c);
#endif
					*typed_pnt = 0;
					(void) putline(typed_str, location+1);
				}
				warnerror(location+1, ind+1, c, try_str[ind]);
				lasterr = try_str[ind];
				++linerrors;
				++badchar[lasterr];
				++gbadchar[lasterr];
			}
			else {
				if(echo_it)
					pr(0, "%c", noctrl(c));
				else
#ifdef SHOWALL
					*typed_pnt++ = noctrl(c);
#else
					*typed_pnt++ = ' ';
#endif
				clrtoeol();
			}
			refresh();
			break;
		}
	}
	*typed_pnt = 0;
#ifdef SHOWALL
	if(! echo_it)
		(void) putline(typed_str, location+1);
#endif
	standend();
	clrtoeol();
	elapsed = gettime(12, "/ ") - starttime;
	tottime += elapsed;
	gtottime += elapsed;
	moveto(SPEED, COLS-40, LFCR);
	standout();
	if(! CA)
		printf("\t\t\t\t");
	pr(2, "%.1f secs, %d error(s), %.2f w/min", elapsed, linerrors,
		(f=((n_of_chars / 5.) - linerrors) / (elapsed / 60.))>0.?f:0.0);
	standend();
	refresh();
	return(n_of_chars);
}

float
gettime(where, str)
register char *str;
{
	struct tm *localtime();
	register struct tm *tm;
	register x, y;

	ftime(&tb);
	if(isafastscreen) {
		tm = localtime(&tb.time);
		getyx(stdscr, y, x);
		move(SPEED, where);
		pr(1, "%s%02d:%02d:%02d.%02d", str, tm->tm_hour, tm->tm_min,
			tm->tm_sec, (tb.millitm / 10));
		clrtoeol();
		move(y, x);
		refresh();
	}
	return((float) (tb.time - ta.time) + ((float)tb.millitm / 1000.));
}

putline(outstr, where)
register char *outstr;
{
	register n;
	char *rindex();
	register char *back = rindex(outstr, '\0') - 1;

	while(isspace(*back))
		*back-- = 0;
	moveto(where, 0, "\r");
	for(n = 0 ; *outstr ; ++n)
		pr(0, "%c", noctrl(*outstr++));
	if(n)
		refresh();
	return(n);
}

warnerror(x, y, goodc, badc)
{
	if(! isafastscreen || y > COLS-15) {
		putchar('\07');
		return;
	}
	clrtoeol();
	move(x, y+3);
	standout();
	pr(0, "Error (%c/%c)", goodc, badc);
	standend();
	move(x, y);
	refresh();
}

/* NOSTRICT */
/* VARARGS */
note(str1, str2)
char *str1;
long str2;
{
	moveto(BOTTOM+1, COLS-40, LFCR);
	clrtoeol();
	standout();
	pr(2, str1, str2);
	standend();
}

ch_case(bad, good) /* converts to upper or lower case */
{
	register ret;

	if(isupper(bad))
		return((good - (ret=tolower(bad))) ? ret : bad);
	if(islower(bad))
		return((good - (ret=toupper(bad))) ? ret : bad);
	return(noctrl(bad));
}

noctrl(ch)
register ch;
{
	if(ch == '\t')
		return(' ');
	return(iscntrl(ch) ? (ch - '\01' + 'A') : ch);
}

score(errstr, err, lng, ttime) /* prints the score */
char *errstr;
register float lng, ttime;
{
	register s, i, n=0;
	float f;

	if(lng == 0.)
		return;
	moveto(SCORE, 0, "\n\n\r");
	s = (lng - (float) err) * 100. / lng;
	pr(0,"%d errors on %.0f characters (%d %% error), ", err, lng, (100-s));
	pr(1, "average speed: %.2f words/minute",
		((f = ((lng / 5.) - err) / (ttime / 60.)) >0. ? f : 0.), err);
	if(err) {
		moveto(SCORE + 2, 0, LFCR);
		for(n = i = 0 ; i < MAX ; ++i)
			if(errstr[i] != 0) {
				pr(0, "%4d errors on '%c'", errstr[i], i);
				if(++n > 3) {
					n = 0;
					pr(1, LFCR);
				}
			}
		pr(1, LFCR);
	}
	return;
}

/* NOSTRICT */
/* VARARGS */
pr(mode, fmt, str1, str2, str3, str4, str5) /* mode 2: add LFCR, 1: refresh() */
char *fmt;
long str1, str2, str3, str4, str5;
{
	if( CA) {
		printw(fmt, str1, str2, str3, str4, str5);
		if(mode == 2)
			printw(LFCR);
		if(mode > 0)
			refresh();
	}
	else {
		printf(fmt, str1, str2, str3, str4, str5);
		if(mode == 2)
			printf(LFCR);
		fflush(stdout);
	}
}

highspeed() /*return 1 if 1200 baud or more */
{
	struct sgttyb mod;

	gtty(0, &mod);
	if(mod.sg_ispeed >= B1200)
		return(1);
	move(ASK, 10);
	pr(1, "[Using slow terminal mode]");
	return(0);
}

help1()
{
	sethelp();
	pr(2,"\t If you just hit  <RETURN>,  the system will give you  random");
	pr(2,"\tcharacters  picked from all the  characters of your  keyboard");
	pr(2,"\tto practice on. If you want to limit the range of characters,");
	pr(2,"\tenter the list followed by a <RETURN>.");
	pr(2,"\t If you want to practice on random words from the dictionnary");
	pr(2,"\ttype 'words', and if you want to practice on an existing file");
	pr(2,"\ttype 'file'.");
}

help2()
{
	sethelp();
	pr(2, "\tThe system will print one by one all the lines from the file");
	pr(2, "\tand wait for you to retype them.   If the file is not in the");
	pr(2, "\tcurrent directory, enter the full path name (Ex:/home/test).");
	pr(2, "\t Note:   trailing spaces and tabs are truncated, tabs in the");
	pr(2, "\tlines are replaced with spaces and empty lines are skipped.");
}

help3()
{
	sethelp();
	pr(2, "\t The system will now pick random words from the dictionary.");
	pr(2, "\tEnter  the number of words you want the system to  give you");
	pr(2, "\ton each line.");
}

help4()
{
	sethelp();
	pr(2, "\t The system will now make strings of characters for you to");
	pr(2, "\tpractice on. Enter the length that you want each string to");
	pr(2, "\tbe.");
}

help5()
{
	sethelp();
	pr(2, "\t How many lines do you want the system to give you before");
	pr(2, "\tit resets the errors to zero?");
}

help6()
{
	sethelp();
	pr(2,"\t Do you want to see the characters you are typing as you type");
#ifdef SHOWALL
	pr(2,"\tthem?  If you type 'no', the line will be printed only if you");
	pr(2,"\tmake a mistake.  In any case, it is  printed in full when the");
	pr(2,"\tline is complete.");
#else
	pr(2,"\tthem?  If you type 'no', only the errors will be printed.");
#endif
}

help7()
{
	sethelp();
	pr(2, "\tMiscellaneous Notes: <RETURN>  and  <LINE-FEED> are ignored.");
	pr(2, "\tCtrl_A   moves the cursor to the next character to be typed.");
	pr(2, "\tEach error subtracts one word from the speed.    (The longer");
	pr(2, "\tthe strings to be typed are, the more accurate the speed is)");
	pr(2, "\n\t     ---- Written by Chris Bertin, May 1983 ----");
	if(CA)
		(void) answer(work_str, "Hit <RETURN> to continue", CHAR);
}

leave()
{
	if(gtotlngth) {
		moveto(SPEED, 0, LFCR);
		pr(2, "\t\t\tS E S S I O N	T O T A L");
		score(gbadchar, gtoterrors, gtotlngth, gtottime);
	}
	clrtobot();
	moveto(BOTTOM, 0, LFCR);
	refresh();
	endwin();
	exit(0);
}

zero(str, n)
register char *str;
register n;
{
	while(n--)
		*str++ = 0;
}
