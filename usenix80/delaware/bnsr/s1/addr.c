/*% cc -s -O % -o addr -lPW
 *
 *	created dmtilbrook
 *
 *	find an address, produce form letters or mailing labels
 *		by searching address master files
 *
 *	Master files contain sets of lines for each address.
 *	The first line begins with a `!' <key>.
 *	The 1st line also contains a salutation field (default `Sir')
 *	and the name.
 *	The subsequent lines contain the address.
 *	For convenience, `;' is interpreted as a NEWLINE.
*/
#define	USAGE "addr [-{e|k|l}d] [-pN] [-oN] [-sN] [-a file] [-f formletter] key[, dear] ..."
#define	ADDRDEFAULT "/usr/adm/addr"
#define	OFFSETDEFAULT 35
#define DOWNDEFAULT	12
#define	PAGEDEFAULT	66
#define	DEVTTY	"/dev/tty"
char	addbuf[512];
char	*address;
char	*salut;
#include	"../include/getcbuf.h"
struct	getcbuf	addrbuf;
struct	getcbuf	formbuf;
char	afile;
char	ffile;
char	keysonly;
char	addrfile[64];
char	formfile[64];
char	offdefined;
char	pdefined;
char	ddefined;
char	pause;
int	offset	OFFSETDEFAULT;
int	down	DOWNDEFAULT;
int	plen	PAGEDEFAULT;
int	first;
int	(*output)();
int	keycount;
int	ttybuf[3];
char	leader[80];
extern	fin;
extern	fout;
getout()
{
	exit(1);
}
exit(status)
{
	if (fin) {
		ttybuf[2] =| 010;
		stty(fin, ttybuf);
	}
	_exit(status);
}
straight()
{
	if (first++) {
		putchar('\n');
	}
	printf("%s", address);
}
envel()
{
	register int lc;
	register char *p;
	if (pause) {
		getchar();
	}
	for (lc = 0; lc < down; lc++) {
		putchar('\n');
	}
	for (p = address; *p; ) {
		printf(leader);
		/* depends on fact that '\n' preceeds end of string */
		do putchar(*p); while (*p++ != '\n');
		lc++;
	}
	while (lc++ < plen) {
		putchar('\n');
	}
}
formlet(key) char *key;
{
	register char c;
	register char last;
	register char *p;
	check(seek(formbuf.fildes, 0, 0), formfile);
	formbuf.nleft = 0;
	for (p = key; *p && *p++ != ', '; );
	if (!*p) {
		p = (*salut) ? salut : "Sir";
	}
	last = '\n';
	while ((c = getc(&formbuf)) > 0) {
		if (last == '\n' && c == '!') {
			if ((c = getc(&formbuf)) == '\n') {
				break;
			}
			putchar('!');
		}
		putchar(last = c);
	}
	if (c <= 0) {
		derror("No '!' line in form letter", formfile, 1);
	}
	printf("%s\nDear %s:\n", address, p);
	while ((c = getc(&formbuf)) > 0) {
		putchar(c);
	}
}
main(argc, argv) int argc; char **argv;
{
	register int i;
	register char *key;
	output = &straight;
	arginterp(argc, argv);
	if (!pause) {
		fout = check(dup(1), "dup(1)");
	}
	check(fopen(addrfile, &addrbuf), addrfile);
	if (ffile) {
		check(fopen(formfile, &formbuf), formfile);
	}
	if (keysonly) {
		getkeys();
	} else {
		i = 0;
		do {
			if (getad(key = argv[i++])) {
				(*output)(key);
			} else {
				derror("address not found", key, 0);
			}
		} while (i < keycount);
	}
	flush();
	exit(0);
}
arginterp(argc, argv) int argc; char **argv;
{
	register char *p;
	register int i;
	char	**keys;
	keys = argv;
	while (--argc) {
		argv++;
		if (**argv != '-') {
			keys[keycount++] = *argv;
			continue;
		}
		switch (argv[0][1]) {
		case 'e':
		case 'k':
		case 'l':
		case 'd':
			for (p = &argv[0][1]; *p; ) {
				switch (*p++) {
				case 'e':
					pause++;
				case 'l':
					output = &envel;
					continue;
				case 'k':
					keysonly++;
					continue;
				case 'd':
					strcpy(addrfile, ADDRDEFAULT);
					afile++;
					continue;
				default:
					usage("invalid flag", *argv);
				}
			}
			continue;
		case 'o':
			if (offdefined++) {
				derror("offset doubly defined", *argv, 1);
			}
			offset = atoi(*argv + 2);
			if (offset < 0 ||
				(offset / 8 + offset % 8) >= sizeof leader) {
				derror("offset out of range", *argv, 1);
			}
			continue;
		case 'p':
			if (pdefined++) {
				derror("page length doubly defined", *argv, 1);
			}
			plen = atoi(*argv + 2);
			continue;
		case 's':
			if (ddefined++) {
				derror("down doubly defined", *argv, 1);
			}
			down = atoi(*argv + 2);
			continue;
		case 'a':
			if (!--argc) {
				derror("missing address file", *argv, 1);
			}
			afile++;
			strcpy(addrfile, *++argv);
			continue;
		case 'f':
			if (!--argc) {
				derror("missing form letter file", *argv, 1);
			}
			if (ffile++) {
				derror("form letter file doubly defined", *argv, 1);
			}
			strcpy(formfile, *++argv);
			output = &formlet;
			continue;
		default:
			usage("invalid option", *argv);
		}
	}
	if (!(keycount || keysonly)) {
		usage("no keys", "");
	}
	if (afile >= 2) {
		derror("address file doubly defined", *argv, 1);
	}
	if (afile == 0) {
		sprintf(addrfile, "%s/.addr", logdir());
	}
	if (pause) {
		signal(2, getout);
		fin = check(open(DEVTTY, 0), DEVTTY);
		check(gtty(fin, ttybuf), DEVTTY);
		ttybuf[2] =& ~010;
		check(stty(fin, ttybuf), DEVTTY);
	}
	p = leader;
	for (i = offset; i >= 8; i =- 8) {
		*p++ = '\t';
	}
	while (i--) {
		*p++ = ' ';
	}
}
usage(mess, arg) char *mess; char *arg;
{
	derror(mess, arg, 0);
	derror(USAGE, "Usage", 1);
}
check(cbit, mess) int cbit; char *mess;
{
	if (cbit >= 0) return(cbit);
	perror(mess);
	flush();
	exit(1);
}
findbang()
{
	register char c;
	while ((c = getc(&addrbuf)) != '!' && c > 0);
	return(c == '!');
}
getad(key) char *key;
{
	register char *p;
	register char c;
	register char last;
	int found;
	check(seek(addrbuf.fildes, 0, 0), addrfile);
	addrbuf.nleft = 0;
	for (; ; ) {
		if (!findbang()) return(0);
		for (p = key; (*p == (c = getc(&addrbuf))) && c > 0; p++);
		if (c != '\t' || (*p && *p != ', ')) {
			if (c <= 0) {
				derror("Unexpected EOF", addrfile, 1);
			}
			continue;
		}
		p = addbuf;
		last = 0;
		found = 0;
		salut = "",
		address = addbuf;
		for (; ; ) {
			switch (c = getc(&addrbuf)) {
			case 0:
			case -1:
				if (last != '\n') {
					*p++ = '\n';
				}
				*p = 0;
				return(1);
			case '\t':
				if (found++ == 0) {
					*p++ = 0;
					address = p;
					salut = addbuf;
					last = 0;
					continue;
				}
				break;
			case ';':
				found++;
				*p++ = '\n';
				last = c;
				continue;
			case ' ':
				if (last == ';') {
					continue;
				}
				break;
			case '\n':
				found++;
				break;
			case '!':
				if (last == '\n') {
					*p = 0;
					return(1);
				}
			}
			*p++ = c;
			last = c;
		}
	}
}
getkeys()
{
	register int c;
	for (; ; ) {
		switch (c = getc(&addrbuf)) {
		case -1:
		case 0:
			return;
		case '!':
			while ((c = getc(&addrbuf)) > 0 && c != '\n') {
				putchar(c);
			}
			putchar('\n');
			continue;
		case '\n':
			continue;
		default:
			while ((c = getc(&addrbuf)) > 0 && c != '\n');
		}
	}
}
