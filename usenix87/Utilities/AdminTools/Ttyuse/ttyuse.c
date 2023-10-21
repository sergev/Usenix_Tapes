/*
** Still to handle:  do days with no logins or logouts but tty use!
*/

#include "stdio.h"
#include "ctype.h"
#include "utmp.h"
#include "time.h"

#ifdef OBJECTID
static char	sccsid[] = "@(#)ttyuse.c	1.1";
#endif

#ifdef USG
/* Uncompatible Software Glitch */
#define index	strchr
#endif

#ifndef TRUE
#define TRUE	(1)
#define FALSE	(0)
#endif

#ifndef arg4alloc
#define arg4alloc	unsigned
#endif

extern char *		calloc();
extern char *		ctime();
extern char *		index();
extern struct tm *	localtime();
extern long		lseek();
extern char *		realloc();
extern char *		sprintf();
extern char *		strcat();
extern char *		strcpy();
extern long		time();

static long		midnight();
static long		mdy2t();

#define MAXDAYHOURS	25
#define HOURSLOTS	3
#define LONGHOURSLOTS	5

#define MINUTE		60
#define HOUR		(60*MINUTE)

struct entry {
	struct utmp	u;
	int		marked;
	int		daysecs;
	char		l[MAXDAYHOURS * LONGHOURSLOTS + 1];
};

static struct entry *	entries;
static struct entry *	ebeyond;

static int		dayhours;
static int		dayslots;
static int		hourslots;

#define NAMESIZE	(sizeof entries[0].u.ut_name)
#define LINESIZE	(sizeof entries[0].u.ut_line)
#define namecmp(name1, name2) strncmp(name1, name2, NAMESIZE)
#define linecmp(line1, line2) strncmp(line1, line2, LINESIZE)
#define namecpy(to, from) strncpy(to, from, NAMESIZE)
#define linecpy(to, from) strncpy(to, from, LINESIZE)

#define BOOTCHAR	'~'
#define OLDTCHAR	'|'
/*
** 4.[12]bsd writes '{' to the file, but the documentation says that '}' is
** the character.  We'll take either.
**
** System V does things differently, of course.
*/
#define NEWTCHAR	'{'
#define OTHTCHAR	'}'

#define BOOTLINE(s) (*(s) == BOOTCHAR && (s)[1] == '\0' || \
			strcmp((s), "reboot") == 0)
#define OLDTLINE(s) (*(s) == OLDTCHAR && (s)[1] == '\0' || \
			strncmp((s), "old time", 8) == 0)
#define NEWTLINE(s) (*(s) == NEWTCHAR && (s)[1] == '\0' || \
			*(s) == OTHTCHAR && (s)[1] == '\0' || \
			strncmp((s), "new time", 8) == 0)
#define TIMELINE(s) (OLDTLINE(s) || NEWTLINE(s))
#define SPCLLINE(s) (TIMELINE(s) || BOOTLINE(s))

static struct entry *
lookup(up, doenter)
register struct utmp *	up;
{
	register struct entry *	ep;
	register char *		cp;
	register int		i;

	cp = up->ut_line;
	if (SPCLLINE(cp))
		return NULL;
	for (ep = entries; ep < ebeyond; ++ep)
		if (linecmp(cp, ep->u.ut_line) == 0)
			return ep;
	if (!doenter)
		return NULL;
	i = ebeyond - entries;
	if (i == 0)
		ep = (struct entry *) calloc(1, sizeof *ep);
	else	ep = (struct entry *) realloc((char *) entries,
			(arg4alloc) ((i + 1) * sizeof *ep));
	if (ep == NULL)
		for ( ; ; )
			wildrexit("alloc");
	entries = ep;
	ebeyond = ep + i + 1;
	ep = ebeyond - 1;
	(void) linecpy(ep->u.ut_line, up->ut_line);
	(void) sprintf(ep->l, "%*s", dayslots, "");
	ep->u.ut_time = 0;
	ep->u.ut_name[0] = '\0';
	ep->marked = FALSE;
	ep->daysecs = 0;
	return ep;
}

static char		headline[MAXDAYHOURS * LONGHOURSLOTS + 1];

static int		lflag;	/* long lines */
static char *		wname = "/usr/adm/wtmp";

static char *		name;

static char **		argusers;
static int		cntusers;

static long		starttime;
static long		stoptime;

static
dodate(string)
register char *	string;
{
	struct tm	local;
	int		bmon, bday, byear, emon, eday, eyear, i;
	long		now;
	char		c;

	(void) time(&now);
	local = *localtime(&now);
	c = '\0';
	i = sscanf(string, "%d/%d-%d/%d%c", &bmon, &bday, &emon, &eday, &c);
	if (i != 4 || c != '\0') {
		c = '\0';
		i = sscanf(string, "%d/%d-%d%c", &bmon, &bday, &eday, &c);
		if (i == 3 && c == '\0') {
			emon = bmon;
			if (bday > eday)
				for ( ; ; )
					wildexit("date");
		} else {
			c = '\0';
			i = sscanf(string, "%d/%d%c", &bmon, &bday, &c);
			if (i == 2 && c == '\0') {
				emon = bmon;
				eday = bday;
			} else {
				c = '\0';
				i = sscanf(string, "%d/%d-%c",
					&bmon, &bday, &c);
				if (i == 2 && c == '\0') {
					emon = local.tm_mon + 1;
					eday = local.tm_mday;
				} else for ( ; ; )
					wildexit("date");
			}
		}
	}
	byear = eyear = local.tm_year + 1900;
	if (bmon > emon || bmon == emon && bday > eday)
		--byear;
	starttime = mdy2t(bmon, bday, byear);
	stoptime = mdy2t(emon, eday, eyear);
}

static char *
ttyabbr(string)
register char *	string;
{
	static char	buf[3];

	if (strncmp(string, "tty", 3) == 0)
		(void) strncpy(buf, string + 3, 2);
	else	(void) strncpy(buf, string, 2);
	buf[2] = '\0';
	return buf;
}

static struct utmp *	utmp;
static struct utmp *	ubeyond;
static struct utmp * 	unext;
static struct utmp *	ubase;

static int
qtime(up1, up2)
char *	up1;
char *	up2;
{
	register long	diff;

	diff = ((struct utmp *) up1)->ut_time - ((struct utmp *) up2)->ut_time;
	if (diff == 0)
		return 0;
	else if (diff > 0)
		return 1;
	else	return -1;
}

main(argc, argv)
int	argc;
char *	argv[];
{
	register FILE *		fp;
	register struct entry *	ep;
	register int		i;
	register long		t;
	register long		lastm, m;
	register int		slot;
	register int		didttys;
	register long		filesize;
	static int		Rflag;
	static int		diddate;
	char			buf[134];
	extern int		optind;
	extern char *		optarg;

	for (name = argv[0]; *name != '\0'; ++name)
		if (name[0] == '/' && name[1] != '\0' && name[1] != '/')
			argv[0] = ++name;
	name = argv[0];
	if (argc == 2 && strcmp(argv[1], "=") == 0) {
		(void) printf(
"%s: usage is %s [-lR] [-w wtmp] [m/d[-[[m/]d]]] [user...] [/dev/*...]\n",
			name, name);
		for ( ; ; )
			exit(1);
	}
	while ((i = getopt(argc, argv, "lRw:")) != EOF)
		switch (i) {
			case 'l':
				lflag = TRUE;
				break;
			case 'R':
				Rflag = TRUE;
				starttime = 0;
				(void) time(&stoptime);
				break;
			case 'w':
				wname = optarg;
				break;
			default:
				for ( ; ; )
					wildexit("usage");
		}
	hourslots = lflag ? LONGHOURSLOTS : HOURSLOTS;
	/*
	** Parse arguments
	*/
	i = (argc - optind) + 1;
	argusers = (char **) calloc((arg4alloc) i, sizeof *argusers);
	if (argusers == NULL)
		for ( ; ; )
			wildrexit("calloc");
	cntusers = 0;
	for (i = optind; i < argc; ++i)
		if (strncmp(argv[i], "/dev/", 5) == 0) {
			struct utmp	fake;

			(void) linecpy(fake.ut_line, &argv[i][5]);
			(void) namecpy(fake.ut_name, "fake");
			if (lookup(&fake, FALSE) != NULL)
				for ( ; ; )
					wildrexit("duplicate /dev argument");
			if (lookup(&fake, TRUE) == NULL) /* "cannot happen */
				for ( ; ; )
					wildrexit("lookup");
		} else if (index(argv[i], '/') == 0)
			argusers[cntusers++] = argv[i];
		else if (Rflag)
			for ( ; ; )
				wildexit("combination of date and -R");
		else if (diddate)
			for ( ; ; )
				wildexit("multiple dates");
		else {
			dodate(argv[i]);
			diddate = TRUE;
		}
	argusers[cntusers] = NULL;
	didttys = entries != NULL;
	if (!Rflag && !diddate) {
		(void) time(&starttime);
		stoptime = starttime;
	}
	starttime = midnight(starttime);
	stoptime = midnight(stoptime);
	stoptime = midnight(stoptime + MAXDAYHOURS * HOUR);
	/*
	** And now the real work begins.
	*/
	if ((fp = fopen(wname, "r")) == NULL)
		for ( ; ; )
			wildrexit("opening wtmp");
	(void) fseek(fp, 0L, 2);
	filesize = ftell(fp);
	if (filesize == 0)
		for ( ; ; )
			tameexit();
	if (filesize < 0 || (filesize % sizeof *utmp) != 0)
		for ( ; ; )
			wildexit("wtmp size");
	i = filesize / sizeof *utmp;
	utmp = (struct utmp *) calloc((arg4alloc) i, sizeof *utmp);
	if (utmp == NULL)
		for ( ; ; )
			wildrexit("calloc");
	ubeyond = utmp + i;
	/*
	** Step 1--read it in.
	*/
#ifdef fileno
	(void) lseek(fileno(fp), 0L, 0);
	i = read(fileno(fp), (char *) utmp, (int) filesize);
#endif
#ifndef fileno
	(void) fseek(fp, 0L, 0);
	i = fread((char *) utmp, sizeof *utmp, (int) i, fp) * sizeof *utmp);
#endif
	if (i != filesize || ferror(fp) || fclose(fp))
		for ( ; ; )
			wildrexit("reading wtmp");
	/*
	** Step 2--eliminate records up to and including last reboot before
	** start time.
	*/
	for (unext = utmp; unext<ubeyond && unext->ut_time<starttime; ++unext)
		if (BOOTLINE(unext->ut_line))
			utmp = unext + 1;
	/*
	** Step 3--eliminate trailing records for times past stoptime.
	*/
	while ((ubeyond - 1) > utmp && (ubeyond - 1)->ut_time >= stoptime)
		--ubeyond;
	/*
	** Step 4--eliminate bogus records.
	*/
	for (unext = ubase = utmp; unext < ubeyond; ++unext)
		if (unext->ut_time == 0 || unext->ut_line[0] == '\0')
			continue;
		else if (ubase == unext)
			++ubase;
		else	*ubase++ = *unext;
	/*
	** Step 5--eliminate leading logouts, reboots, and time changes.
	*/
	while (utmp < ubeyond && SPCLLINE(utmp->ut_line))
		++utmp;
	/*
	** Step 6--eliminate trailing reboots and time changes.
	*/
	while ((ubeyond - 1) > utmp && SPCLLINE((ubeyond - 1)->ut_line))
		--ubeyond;
	/*
	** Step 7--if ttys were specified, eliminate records for other ttys.
	*/
	if (didttys)
		for (ubase = unext = utmp; unext < ubeyond; ++unext)
			if (SPCLLINE(unext->ut_line) ||
				lookup(unext, FALSE) != NULL)
					if (ubase == unext)
						++ubase;
					else	*ubase++ = *unext;
	/*
	** Final step--sort records between time changes.
	*/
	for (ubase = unext = utmp; unext < ubeyond; ++unext)
		if (TIMELINE(unext->ut_line)) {
			if (unext != ubase)
				(void) qsort((char *) ubase, unext - ubase,
					sizeof *ubase, qtime);
			ubase = unext + 1;
		}
	if (ubeyond > ubase)
		(void) qsort((char *) ubase, ubeyond - ubase,
			sizeof *ubase, qtime);
	if (Rflag)
		unext = utmp;
	else {
		/*
		** Find first record that's for after the start time.
		*/
		for (unext = utmp; unext < ubeyond; ++unext)
			if (unext->ut_time >= starttime)
				break;
		if (unext >= ubeyond)	/* easy enough! */
			for ( ; ; )
				tameexit();
		ubase = unext;
		/*
		** Scan back to a reboot or to the first record
		** (or, if we didttys, until all ttys are accounted for).
		*/
		for ( ; unext > utmp; --unext) {
			if (BOOTLINE(unext->ut_line))
				break;
			if ((ep = lookup(unext, !didttys)) == NULL)
				continue;
			if (ep->marked)
				continue;
			ep->marked = TRUE;
			(void) namecpy(ep->u.ut_name, unext->ut_name);
			if (didttys) {
				for (ep = entries; ep < ebeyond; ++ep)
					if (!ep->marked)
						break;
				if (ep >= ebeyond)
					break;
			}
		}
		/*
		** If users were specified, zap the records for anybody else.
		*/
		if (cntusers > 0)
			for (ep = entries; ep < ebeyond; ++ep) {
				for (i = 0; i < cntusers; ++i)
					if (namecmp(argusers[i],
						ep->u.ut_name) == 0)
							break;
				if (i >= cntusers)
					ep->u.ut_name[0] = '\0';
			}
		/*
		** We're ready!
		*/
		utmp = unext = ubase;
	}
	lastm = starttime - 1;
	for ( ; unext < ubeyond; ++unext, lastm = m) {
		t = unext->ut_time;
		m = midnight(t);
		if (m != lastm) {
			if (lastm >= starttime && lastm < stoptime) {
				for (ep = entries; ep < ebeyond; ++ep) {
					if (ep->u.ut_name[0] == '\0')
						continue;
					ep->daysecs += (lastm + dayhours *
						HOUR) - ep->u.ut_time;
					if (ep->daysecs == 0)
						++(ep->daysecs);
					colonize(ep->l, dayslots - 1);
				}
				dump(lastm);
			}
			newday(m, 0L, '\0');
		}
		slot = (unext->ut_time - m) / (HOUR / hourslots);
		if (BOOTLINE(unext->ut_line)) {
			headline[slot] = BOOTCHAR;
			for (ep = entries; ep < ebeyond; ++ep) {
				if (ep->u.ut_name[0] != '\0') {
					ep->daysecs += unext->ut_time -
						ep->u.ut_time;
					if (ep->daysecs == 0)
						++(ep->daysecs);
					colonize(ep->l, slot);
					ep->u.ut_name[0] = '\0';
					ep->u.ut_time = 0;
				}
			}
			continue;
		}
		if (NEWTLINE(unext->ut_line))
			for ( ; ; )
				wildexit("new time without old time");
		if (OLDTLINE(unext->ut_line)) {
			if (unext == ubeyond || !NEWTLINE((unext + 1)->ut_line))
				for ( ; ; )
					wildexit("old time without new time");
			headline[slot] = OLDTCHAR;
			for (ep = entries; ep < ebeyond; ++ep) {
				if (ep->u.ut_name[0] == '\0')
					ep->l[slot] = OLDTCHAR;
				else {
					ep->daysecs += unext->ut_time -
						ep->u.ut_time;
					if (ep->daysecs == 0)
						++(ep->daysecs);
					colonize(ep->l, slot);
				}
			}
			dump(lastm);
			++unext;
			if (unext->ut_line[0] == NEWTCHAR ||
				unext->ut_line[0] == OTHTCHAR)
					newday(m, unext->ut_time,
						unext->ut_line[0]);
			else	newday(m, unext->ut_time, NEWTCHAR);
			continue;
		}
		if ((ep = lookup(unext, !didttys)) == NULL)
			continue;
		/*
		** If a login is in progress, either a login or
		** logout terminates it.
		*/
		if (ep->u.ut_name[0] != '\0') {
			ep->daysecs += unext->ut_time - ep->u.ut_time;
			if (ep->daysecs == 0)
				++(ep->daysecs);
			colonize(ep->l, slot);
			ep->u.ut_name[0] = '\0';
			ep->u.ut_time = 0;
		}
		if (unext->ut_name[0] == '\0')
			continue;
		if (cntusers > 0) {
			for (i = 0; i < cntusers; ++i)
				if (namecmp(argusers[i], unext->ut_name) == 0)
					break;
			if (i == cntusers)
				continue;
		}
		ep->u.ut_time = unext->ut_time;
		if (isupper(ep->l[slot])) {
			buf[0] = '2';
			i = 1;
		} else {
			register char *	cp;

			cp = index("23456789**", ep->l[slot]);
			if (cp != 0) {
				buf[0] = *(cp + 1);
				i = 1;
			} else	i = 0;
		}
		fudge(unext->ut_name);
		(void) namecpy(ep->u.ut_name, unext->ut_name);
		(void) namecpy(&buf[i], unext->ut_name);
		/*
		** Ensure termination.
		*/
		buf[NAMESIZE + i] = '\0';
		if (i == 0 && islower(buf[0]))
			buf[0] = toupper(buf[0]);
		i = strlen(buf);
		if (i > (dayslots - slot))
			i = dayslots - slot;
		(void) strncpy(&ep->l[slot], buf, i);
	}
	for (ep = entries; ep < ebeyond; ++ep) {
		if (ep->u.ut_name[0] == '\0')
			continue;
		ep->daysecs += t - ep->u.ut_time;
		if (ep->daysecs == 0)
			++(ep->daysecs);
		colonize(ep->l, slot);
	}
	dump(lastm);
	for ( ; ; )
		tameexit();
}

static int
qline(ep1, ep2)
char *	ep1;
char *	ep2;
{
	return linecmp(((struct entry *) ep1)->u.ut_line,
		((struct entry *) ep2)->u.ut_line);
}

static int
ttysused(base, count)
register int	base;
register int	count;
{
	register struct entry *	ep;
	register int		i;
	register int		result;

	result = 0;
	for (ep = entries; ep < ebeyond; ++ep)
		for (i = 0; i < count; ++i) {
			switch (ep->l[base + i]) {
				case ' ':
				case BOOTCHAR:
				case OLDTCHAR:
				case NEWTCHAR:
				case OTHTCHAR:
					continue;
			}
			++result;
			break;
		}
	return result;
}

static
dump(m)
long	m;	/* midnight */
{
	register struct entry *	ep;
	register char *		cp;
	register int		i, j, k, lines;
	register int		width, prec;
	register double		d;
	struct tm		tm;
	long			grandtot;
	char			buf[20];

	tm = *localtime(&m);
	grandtot = 0;
	for (ep = entries; ep < ebeyond; ++ep)
		grandtot += ep->daysecs;
	if (grandtot == 0)
		return;
	(void) qsort((char *) entries, ebeyond - entries,
		sizeof entries[0], qline);
	lines = 0;
	width = (lflag ? 131 : 79) - dayslots;
	prec = (width >= 7) ? width : 3;
	for (ep = entries; ep < ebeyond; ++ep) {
		if (ep->daysecs == 0)
			continue;
		if (lines == 0) {
			cp = ctime(&m);
			(void) printf("%.11s%s", cp, cp + 20);
			(void) printf("%-*.*s", width, prec, "TT USE");
			(void) puts(headline);
		}
		d = ep->daysecs / (double) HOUR;
		(void) sprintf(buf, "%2.2s%4.1f", ttyabbr(ep->u.ut_line), d);
		if (buf[2] != ' ')
			(void) sprintf(buf, "%2.2s%4.0f",
				ttyabbr(ep->u.ut_line), d);
		(void) printf("%-*.*s", width, prec, buf);
		for (i = 0; i < dayslots; ++i)
			if (headline[i] == BOOTCHAR && ep->l[i] == ' ')
				ep->l[i] = BOOTCHAR;
		cp = ep->l + dayslots;
		while (cp > ep->l && *(cp - 1) == ' ')
			--cp;
		(void) printf("%.*s\n", cp - ep->l, ep->l);
		++lines;
	}
	if (lines <= 1)
		return;
	if (prec > 6)
		(void) sprintf(buf, "%6.1f", grandtot / (double) HOUR);
	else	buf[0] = '\0';
	(void) printf("%-*.*s", width, prec, buf);
	for (i = 0; i < dayslots; i += j) {
		j = lflag ? hourslots : (2 * hourslots);
		if (i == 0 && (dayslots % j) != 0)
			j += dayslots % j;
		if ((k = ttysused(i, j)) == 0)
			(void) printf("%*s", j, "");
		else	(void) printf("<%*d>", j - 2, k);
	}
	(void) printf("\n");
}

static
colonize(line, slot)
register char *	line;
register int	slot;
{
	register int	i;

	for (i = slot + 1; i < dayslots && line[slot] != ' '; ++i)
		line[i] = ' ';
	while (slot >= 0 && line[slot] == ' ') {
		line[slot] = ':';
		--slot;
	}
}

static
wildrexit(string)
char *	string;
{
	(void) fprintf(stderr, "%s: wild result from %s\n", name, string);
	for ( ; ; )
		exit(1);
}

static
wildexit(string)
char *	string;
{
	(void) fprintf(stderr, "%s: wild %s\n", name, string);
	for ( ; ; )
		exit(1);
}

static
tameexit()
{
	for ( ; ; )
		exit(0);
}

static
fudge(buf)
register char *	buf;
{
	register int	i;

	for (i = 0; i < NAMESIZE; ++i)
		switch (buf[i]) {
			case '\0':
				return;
			case BOOTCHAR:
			case OLDTCHAR:
			case NEWTCHAR:
			case OTHTCHAR:
			case ':':
			case '*':
			case ' ':
			case '2': case '3': case '4': case '5':
			case '6': case '7': case '8': case '9':
				buf[i] = '?';
				break;
			default:
				if (!isascii(buf[i]))
					buf[i] = '?';
				else if (isupper(buf[i]))
					buf[i] = tolower(buf[i]);
				else if (!isprint(buf[i]))
					buf[i] = '?';
				break;
		}
}

static long
mdy2t(m, d, y)
{
	register struct tm *	tmp;
	register int		diff;
	register int		lastdiff;
	long			guess;

	if (y < 1969 || m <= 0 || m > 12 || d <= 0 || d >= 32)
		for ( ; ; )
			wildrexit("date");
	guess = d + (m - 1) * (365.25 / 12.) + (y - 1970) * 365.25;
	guess = guess * HOUR * 24;
	y -= 1900;
	--m;
	lastdiff = 0;
	for ( ; ; ) {
		tmp = localtime(&guess);
		if ((diff = tmp->tm_year - y) == 0 &&
			(diff = tmp->tm_mon - m) == 0)
				diff = tmp->tm_mday - d;
		if (diff == 0)
			return midnight(guess);
		if (lastdiff != 0)
			if ((diff > 0) != (lastdiff > 0))
				for ( ; ; )
					wildrexit("date");
		if (diff > 0)
			guess -= 12 * HOUR;
		else	guess += 12 * HOUR;
		lastdiff = diff;
	}
}

static long
midnight(t)
long	t;
{
	register int	diff, lastdiff;
	struct tm 	tm;
	struct tm	mtm;
	long		m;

	tm = *localtime(&t);
	if (tm.tm_hour == 0 && tm.tm_min == 0 && tm.tm_sec == 0)
		return t;
	m = t;
	m -= tm.tm_hour * HOUR;
	m -= tm.tm_min * MINUTE;
	m -= tm.tm_sec;
	lastdiff = 0;
	for ( ; ; ) {
		mtm = *localtime(&m);
		if ((diff = mtm.tm_year - tm.tm_year) == 0 &&
			(diff = mtm.tm_mon - tm.tm_mon) == 0 &&
			(diff = mtm.tm_mday - tm.tm_mday) == 0 &&
			(diff = mtm.tm_hour) == 0)
				if (mtm.tm_min == 0 && mtm.tm_sec == 0)
					return m;
				else	for ( ; ; )
						wildrexit("finding midnight");
		if (lastdiff != 0)
			if ((diff > 0) != (lastdiff > 0))
				for ( ; ; )
					wildrexit("finding midnight");
		if (diff > 0)
			m -= HOUR;
		else	m += HOUR;
		lastdiff = diff;
	}
}

static
newday(m, t, headchar)
long	m, t;
{
	register struct entry *	ep;
	register int		i, j, slot;
	long			t2;
	char			buf[20];

	dayhours = (midnight(m + MAXDAYHOURS * HOUR) - m) / HOUR;
	if (dayhours <= 0 || dayhours > MAXDAYHOURS)
		for ( ; ; )
			wildexit("number of hours in a day");
	dayslots = hourslots * dayhours;
	headline[0] = '\0';
	for (i = 0; i < dayhours; ++i) {
		if (dayhours == 24)
			j = i;
		else {
			t2 = m + i * HOUR;
			j = localtime(&t2)->tm_hour;
		}
		if ((j %= 12) == 0)
			j = 12;
		if (j == 12 && lflag)
			if (i == 0)
				(void) strcpy(buf, "Mid.-");
			else	(void) strcpy(buf, "Noon-");
		else	(void) sprintf(buf, "%d-----", j);
		buf[hourslots] = '\0';
		(void) strcat(headline, buf);
	}
	if (t == 0)
		slot = 0;
	else {
		slot = (t - m) / (HOUR / hourslots);
		headline[slot] = headchar;
	}
	for (ep = entries; ep < ebeyond; ++ep) {
		(void) sprintf(ep->l, "%*s", dayslots, "");
		ep->daysecs = 0;
		if (ep->u.ut_name[0] == '\0') {
			ep->u.ut_time = 0;
			if (t != 0)
				ep->l[slot] = headchar;
		} else {
			ep->u.ut_time = (t == 0) ? m : t;
			for (i = 0; i < NAMESIZE; ++i) {
				if (ep->u.ut_name[i] == '\0')
					break;
				if ((slot + i) >= dayslots)
					break;
				ep->l[slot + i] = ep->u.ut_name[i];
			}
		}
	}
}
