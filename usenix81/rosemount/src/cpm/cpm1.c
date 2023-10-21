/* cpm - UNIX to CP/M 1.4 diskette interface program
 *
 * for questions, corrections, or suggestions my address is
 *	David L. Pederson
 *	Rosemount Inc.
 *	mail station c11
 *	12001 W. 78th St.
 *	Eden Prairie, Minn. 55344
 *	(612) 937-3679
 */

/* 11/May/81 - origional release
 */

# include <stdio.h>
# include <ctype.h>
# include "cpm.h"

int cfd = -1;
int sat[GROUPS];
struct fcb dir[SLOTS];		/* in core version of directory */
char *drvnam[] = {
	"/dev/rx0c",
	"/dev/rx1c"
};
# define NUMDRVS (sizeof drvnam / sizeof drvnam[0])

char *help[] = {
	"  cpu    copy CP/M file to UNIX\n",
	"  cpc    copy UNIX file to CP/M\n",
	"  ls     list CP/M directory\n",
	"  mv     change CP/M file name\n",
	"  rm     remove a CP/M file\n",
	"  help   print this message\n",
	"  quit   quit this program\n",
	"  q      same as 'quit'\n",
	"  ! prg  run a Unix program\n"
};
# define NUMHELPS (sizeof help / sizeof help[0])




main(argc, argv)
int argc;
char **argv;
{
	int c, drive, dr[2];
	char *v[ARG+1], cmd[CMD];
	char *drvs = { "01" };

	dr[0] = dr[1] = 0;
	for (argc--, argv++; argc > 0 && **argv == '-'; argc--, argv++)
		if (getops(*argv, drvs, dr) == -1) {
			err(USAGE);
			exit(1);
		}
	if (dr[0]+dr[1] == 1)
		drive = (dr[0] == 1 ? 0 : 1);
	else {
		err("missing drive number\n");
		exit(1);
	}
	if (login(drive) == -1)
		exit(1);
	if (argc > 0)
		docmd(argc, argv);
	else
		loop {
			printf("%c> ", drvs[drive]);
			if (fgets(cmd, CMD, stdin) == NULL)
				break;
			if (! isatty(fileno(stdin)))
				printf("%s", cmd);
			if (cmd[0] == '!') {
				system(&cmd[1]);
				printf("!\n");
			}
			else {
				c = getarg(cmd, v, ARG);
				if (docmd(c, v) != 0)
					break;
			}
		}
	logout();
	exit(0);
}



login(drive)
int drive;
{
	if ((cfd = open(drvnam[drive], 2)) == -1)
		fprintf(stderr, "%s: can't open\n", drvnam[drive]);
	else
		getdir();
	return cfd;
}



getdir()
{
	int group, slot, i;
	char dirbuf[IOBUF], *p;
	struct fcb *d;

	d = &dir[0];
	for (group = 0; group < 2; group++) {
		grpio(group, dirbuf, 0);
		p = &dirbuf[0];
		for (slot = 0; slot < (SLOTS/2); slot++) {
			d->et = p[0] & 255;
			strncpy(d->fn, &p[1], 11);
			d->fn[11] = NULL;
			d->ex = p[12] & 255;
			d->rc = p[15] & 255;
			for (i = 0; i < 16; i++)
				d->dm[i] = p[i+16] & 255;
			p += 32;
			d++;
		}
	}
	buildsat();	/* just to test soundness */
	return;
}



getarg(cmd, argv, argcmax)
char *cmd;
char *argv[];
int argcmax;
{
	register int count;
	register char *p;
	int inword;

	count = inword = 0;
	for (p = cmd; *p; p++) {
		if (isspace(*p)) {
			*p = NULL;
			inword = 0;
		}
		else if (inword == 0) {
			inword = 1;
			if (count >= argcmax) {
				err("too many arguments\n");
				return 0;
			}
			argv[count++] = p;
		}
	}
	argv[count] = NULL;
	return count;
}



docmd(argc, argv)
int argc;
char **argv;
{
	int i;

	if (argc <= 0)
		return 0;
	if (strcmp("cpu", *argv) == 0)
		cpu(argc, argv);
	else if (strcmp("cpc", *argv) == 0)
		cpc(argc, argv);
	else if (strcmp("rm", *argv) == 0)
		rm(argc, argv);
	else if (strcmp("ls", *argv) == 0)
		ls(argc, argv);
	else if (strcmp("mv", *argv) == 0)
		mv(argc, argv);
	else if (strcmp(":", *argv) == 0)	/* shell type comment */
		;
	else if (strcmp("help", *argv) == 0)
		for (i = 0; i < NUMHELPS; i++)
			fputs(help[i], stdout);
	else if (strcmp("quit", *argv) == 0 || strcmp("q", *argv) == 0)
		return -1;
	else {
		fprintf(stderr, "'%s': unknown command.", *argv);
		err("  Type 'help' for a list of commands.\n");
	}
	return 0;
}



getops(line, legal, flags)
char *line, *legal;
int *flags;
{
	register char *lp;
	register int *fp;
	int ok;

	if (*line != '-')		/* not an option line */
		return -1;
	while (*++line) {
		ok = 0;
		for (lp = legal, fp = flags; *lp; lp++, fp++)
			if (*line == *lp) {
				*fp = 1;
				ok = 1;
				break;
			}
		if (! ok) {		/* illegal option */
			fprintf(stderr, "'%c': illegal option\n", *line);
			return -1;
		}
	}
	return 0;
}



cpu(argc, argv)		/* copy CP/M to Unix */
int argc;
char **argv;
{
	int ab[2];
	char tmode;
	FILE *ufp;

	ab[0] = ab[1] = 0;
	for (argc--, argv++; argc > 0 && **argv == '-'; argc--, argv++)
		if (getops(*argv, "ab", ab) == -1)
			return;
	switch(ab[0]+ab[1]) {

	case 0:
		tmode = 'a';
		break;

	case 1:
		tmode = (ab[0] == 1 ? 'a' : 'b');
		break;

	default:
		err("illegal transfer modes\n");
		return;
	}
	if (argc < 1 || argc > 2) {
		err("usage: cpu [-{ab}] cpmfile [unixfile]\n");
		return;
	}
	if (copen(*argv, 'r', tmode) == -1)
		return;
	argc--;
	argv++;
	if (argc == 0)
		ufp = stdout;
	else if ((ufp = fopen(*argv, "w")) == NULL) {
		fprintf(stderr, "%s: cannot open\n", *argv);
		return;
	}
	tounix(ufp);
	cclose();
	if (ufp != stdout)
		fclose(ufp);
	return;
}



logout()
{
	close(cfd);
	return;
}



cpmname(c, u)
char *c, *u;
{
	int i, ret = -1;

	strcpy(c, "           ");
	for (i = 0; *u != NULL; u++) {
		if (*u == '.')
			i = 8;
		else if (isalnum(*u) || *u == '_') {
			ret = 0;
			if (i < 11)
				c[i++] = (islower(*u) ? toupper(*u) : *u);
		}
	}
	u[11] = NULL;
	if (ret)
		fprintf(stderr, "'%c': bad CP/M name\n", u);
	return ret;
}



grpio(group, buf, way)
int group;
char *buf;
int way;
{
	int count;
	long lseek();

	if (group < 0 || group >= GROUPS) {
		fprintf(stderr, "error: group %d was accessed\n", group);
		exit(1);
	}
	if (lseek(cfd, (long) group * (long) IOBUF, 0) == -1L) {
		err("lseek error\n");
		exit(1);
	}
	if (way == 1)	/* write */
		count = write(cfd, buf, IOBUF);
	else
		count = read(cfd, buf, IOBUF);
	if (count != IOBUF) {
		err("I/O error\n");
		exit(1);
	}
	return;
}



slotnum(name, extent)		/* find slot with name with extent */
char *name;
int extent;			/* if -1, then any slot will do */
{
	register slot;

	for (slot = 0; slot < SLOTS; slot++)
		if (dir[slot].et != MDEL
		&& strcmp(name, dir[slot].fn) == 0
		&& (extent == -1 || dir[slot].ex == extent))
			return slot;
	return -1;
}



buildsat()
{
	register group, slot, i;

	for (i = 0; i < GROUPS; i++)
		sat[i] = 0;
	sat[0] = sat[1] = 1;	/* directories occupy groups 0 and 1 */

	for (slot = 0; slot < SLOTS; slot++) {
		if (dir[slot].et == MDEL)
			continue;
		if (dir[slot].rc > 128) {
			fprintf(stderr, "bad record count(%d), slot %d\n",
			dir[slot].rc, slot);
			exit(1);
		}
		for (i = 0; i < (dir[slot].rc+7)/8; i++) {
			group = dir[slot].dm[i];
			if (group < 2 || group > GROUPS) {
				fprintf(stderr, "bad group (%d), slot %d\n",
				group, slot);
				exit(1);
			}
			else if (++sat[group] > 1) {
				fprintf(stderr,
				"group double defined(%d), slot %d\n",
				group, slot);
				exit(1);
			}
		}
	}
	return;
}



putdir()
{
	register char *p;
	register slot;
	register struct fcb *d;
	int i, group;
	char dirbuf[IOBUF];

	d = &dir[0];
	for (group = 0; group < 2; group++) {
		p = &dirbuf[0];
		for (slot = 0; slot < (SLOTS/2); slot++) {
			p[0] = d->et;
			strncpy(&p[1], d->fn, 11);
			p[12] = d->ex;
			p[13] = 0;
			p[14] = 0;
			p[15] = d->rc;
			for (i = 0; i < 16; i++)
				p[i+16] = d->dm[i];
			p += 32;
			d++;
		}
		grpio(group, dirbuf, 1);
	}
	return;
}



cpc(argc, argv)		/* copy Unix to CP/M */
int argc;
char **argv;
{
	int ab[2];
	char tmode = 'a';
	FILE *ufp;

	ab[0] = ab[1] = 0;
	for (argc--, argv++; argc > 0 && **argv == '-'; argc--, argv++)
		if (getops(*argv, "ab", ab) == -1)
			return;
	switch(ab[0]+ab[1]) {

	case 0:
		tmode = 'a';
		break;

	case 1:
		tmode = (ab[0] == 1 ? 'a' : 'b');
		break;

	default:
		err("illegal transfer modes\n");
		return;
	}
	if (argc < 1 || argc > 2) {
		err("usage: cpc [-{ab}] [unixfile] cpmfile\n");
		return;
	}
	if (argc == 1)
		ufp = stdin;
	else {
		if ((ufp = fopen(*argv, "r")) == NULL) {
			fprintf(stderr, "%s: cannot open\n", *argv);
			return;
		}
		argc--;
		argv++;
	}
	if (copen(*argv, 'w', tmode) == -1)
		return;
	tocpm(ufp);
	cclose();
	if (ufp != stdin)
		fclose(ufp);
	return;
}



rm(argc, argv)
int argc;
char **argv;
{
	char name[12];
	register int slot;
	int all[1];

	all[0] = 0;
	for (argc--, argv++; argc > 0 && **argv == '-'; argc--, argv++)
		if (getops(*argv, "a", all) == -1)
			return;
	if (all[0]) {
		if (argc != 0)
			err("illegal option use\n");
		else {
			for (slot = 0; slot < SLOTS; slot++)
				dir[slot].et = MDEL;
		}
		putdir();
		return;
	}
	for (; argc > 0; argc--, argv++) {
		if (cpmname(name, *argv) == -1)
			return;
		if (slotnum(name, -1) == -1) {
			fprintf(stderr, "%.8s.%.3s not found\n",
			name, &name[8]);
		}
		while ((slot = slotnum(name, -1)) >= 0)
			dir[slot].et = MDEL;
	}
	putdir();
	return;
}



ls(argc, argv)		/* give a directory listing */
int argc;
char **argv;
{
	register int slot;
	int subt, totl = 0, extent, loc;
	int extra = 0;

	for (argc--, argv++; argc > 0 && **argv == '-'; argc--, argv++)
		if (getops(*argv, "l", &extra) == -1)
			return;
	if (argc == 0) {	/* list all */
		for (slot = 0; slot < SLOTS; slot++)
			if (dir[slot].et != MDEL && dir[slot].ex == 0) {
				printf("%.8s.", dir[slot].fn);
				printf("%.3s", &dir[slot].fn[8]);
				subt = 0;
				extent = 0;
				while ((loc = slotnum(dir[slot].fn,
				extent++)) != -1)
					subt += dir[loc].rc;
				if (extra)
					printf(" %3d.%1dK", subt/8, subt%8);
				totl += subt;
				putchar('\n');
			}
		if (extra)
			printf("%4dK total.\n", (totl+7)/8);
	}
	return;
}



mv(argc, argv)
int argc;
char **argv;
{
	int slot, c;
	char *v[ARG];
	char rmbuf[20];
	char cold[12], cnew[12];

	argc--;
	argv++;
	if (argc != 2) {
		err("usage: mv oldname newname\n");
		return;
	}
	if (cpmname(cold, *argv) == -1 || cpmname(cnew, *++argv) == -1)
		return;
	if (slotnum(cold, 0) == -1) {
		fprintf(stderr, "%.8s.%.3s not found\n", cold, &cold[8]);
		return;
	}
	if (slotnum(cnew, 0) >= 0) {
		strcpy(rmbuf, "rm ");
		strcat(rmbuf, cnew);
		c = getarg(rmbuf, v, ARG);
		rm(c, v);
	}
		while ((slot = slotnum(cold, -1)) != -1)
			strcpy(dir[slot].fn, cnew);
	putdir();
	return;
}
