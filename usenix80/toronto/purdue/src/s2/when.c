#
/*
 * when -- print out selected "wtmp" info.
 *
 *	when arg1 ...
 *
 *	where args are:
 *		<user name>	to look for a particular user
 *		boot		to see when system was rebooted
 *		tty#		to see use of tty number #
 *		time		to see when time was changed.
 * 		all		list all occurances  (not just last).
 */

#define MAXF	10
#define MAXN	25

struct wtmp {
	char name[8];
	char tty;
	char pad1;
	int time[2];
	char pad2[2];
};
struct	wtmp	plast;
int	any	0;	/* non-zero if any entries found */

char	*file "/usr/adm/wtmp";

int	fout;
int	fd;
int     allflag 0;

int	buf[256];
char	flags[MAXF];
char	*names[MAXN];

struct wtmp *np;	/* points to next available */
int	num	0;	/* number of entries left */

main(argc, argv)
char **argv;
{
	register struct wtmp *p;

	if (argc <= 1) {
		printf("Usage: when [<name>] [time] [boot] [tty#] [all]\n");
		exit();
	}
	if ((fd = open(file, 0)) < 0) {
		printf("cannot open history file: %s\n");
		exit();
	}
	fout = dup(1);	/* for buffered I/O */
	setup(argv);	/* setup translation table */
	/*
	 * look through file:
	 */
	if (allflag) {
		while ((p = next()) != 0)
			if (selected(p))
				print(p);

	} else {
		while ((p = next()) != 0)
			if (selected(p))
				save(p);
		if (any) 
			print(&plast);
	}
	flush();
	exit();
}

setup(ap)
char *ap[];
{
	register char *p1, **p2;

	p1 = flags;
	p2 = names;
	while (*++ap != -1) {
		if (match("tty", *ap))
			*p1++ = ap[0][3];
	else	if (match("boot", *ap))
			*p1++ = '~';
	else	if (match("time", *ap))
			*p1++ = '}';
	else    if (match("all", *ap))
			allflag =  1;
	else	*p2++ = *ap;
	}
	*p1++ = 0;
	*p2++ = 0;
}

next()
{
	register cnt;

	if (num-- <= 0) {
		if ((cnt = read(fd, buf, 512)) <= 0)
			return(0);
		num = cnt>>4;
		np = buf;
	}
	return(np++);
}

selected(ap)
struct wtmp *ap;
{
	register char *p1, **p2;

	p1 = flags;
	while (*p1)
		if (*p1++ == ap->tty)
			return(1);
	p2 = names;
	if (ap->name[0] == ' ' || ap->name[0] == 0)
		return(0);
	while (*p2)
		if (match(ap->name, *p2++))
			return(1);
	return(0);
}

print(ap)
struct wtmp *ap;
{
	register int i;

	switch(ap->tty) {

	case '~':
		printf("system  boot"); break;
	case '|':
		printf("old sys date"); break;
	case '}':
		printf("new sys date"); break;
	default:
		printf("%8.8s tty%c", ap->name, ap->tty);
	}
	printf("%s", ctime(ap->time) + 3);
}

match(a,b)
char *a,*b;
{
	register char *p1, *p2;
	register i;

	p1 = a;
	p2 = b;
	for (i=0; i<8; i++) {
		if (*p1 == ' ' || *p1 == 0)
			return(1);
		if (*p1++ != *p2++)
			return(0);
	}
	return(1);
}
save(p)
struct  wtmp *p;
{
	register char *p1, *p2;
	register n;
	p2 = &plast;
	p1 = p;
	
	for (n = 0; n<sizeof plast; n++)
		*p2++ = *p1++;
	any++;
	return;
}
