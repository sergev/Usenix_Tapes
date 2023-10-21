/*
 * dostp - extract from or write to DOS tape
 *
 * Written by Hughes O'Neill, 10/July/77.
 *
 * Modified for V7 and stdio by Alexis Kwan (HCR for DCIEM), 11/Feb/81.
 */

#include <stdio.h>
#include <ctype.h>
#include <signal.h>

#define	MAXLINE 80
#define	NAMSIZE 12
#define	NRAW_TAPE	"/dev/nrmt0"
#define RAW_TAPE	"/dev/rmt0"
#define	DEFAULT_GROUP	1
#define	DEFAULT_USER	1
#define	PROT_CODE	155

#define	BAD	0
#define	TOC	1
#define	EXTRACT	2
#define	REPLACE	3

#define	VERBOSE	1
#define	WAIT	2
#define	NOREWIND 4
#define	CRFLAG	8

/* This must be aligned on word boundary */
struct {
	int	FNAME[2];
	int	EXT;
	char	USER;
	char	GROUP;
	char	PROTECT;
	char	UNUSEDBYTE;
	int	DATE;
	int	UNUSEDWORD[2];
} hd;
#define HDSIZE 16

#define	YES	1
#define	NO	0
#define	ERROR	-1

struct {
	int fildes;
	int nleft;
	char *nextp;
	char buff[BUFSIZ];	/* This must be aligned on word boundary */
} tbuf;
#define READ 0
#define WRITE 1

FILE *in;
FILE *out;

int group = DEFAULT_GROUP;
int user = DEFAULT_USER;

main (argc, argv)
	int argc;
	char *argv[];
{
	char c;
	int opt, cmd, i;

	if (argc<2)
		cmd = BAD;
	if (argv[1][0]=='-')
		argv[1]++;
	else if (argv[1][0]=='x')
		cmd = EXTRACT;
	else if (argv[1][0]=='t')
		cmd = TOC;
	else if (argv[1][0]=='r')
		cmd = REPLACE;
	else
		cmd = BAD;

	opt = CRFLAG;
	for (i = 1; (c = argv[1][i]); i++) {
		switch (c) {
		case 'v':
			opt |= VERBOSE;
			break;
		case 'w':
			opt |= WAIT;
			break;
		case 'n':
			opt |= NOREWIND;
			break;
		case 'b':
			opt &= ~CRFLAG;
			break;
		}
	}

	switch (cmd) {
	case BAD:
		printf("usage:  dostp key[opts] [file ...]\n");
		break;
	case TOC:
		toc(opt,argc,argv);
		break;
	case EXTRACT:
		extract(opt,argc,argv);
		break;
	case REPLACE:
		replace(opt,argc,argv);
		break;
	}
}

/*
 * Print table of contents of a DOS tape.
 */

toc (opt, argc, argv)
	int opt;
	int argc;
	char *argv[];
{
	char name[NAMSIZE];

	if ((tbuf.fildes = open(RAW_TAPE,READ))==ERROR) {
		perror("toc");
		exit(1);
	}
	for (;;) {
		if (gethdr(name)==EOF)
			break;
		if (match(name,argc,argv)) {
			if (opt&VERBOSE)
				printf("[%o,%o] %s \n",group,user,name);
			else
				printf("%s \n",name);
		}
		while (read(tbuf.fildes,tbuf.buff,BUFSIZ) > 0)
			;
	}
}

/*
 * Extract files from DOS tape.
 */

extract (opt, argc, argv)
	int opt;
	int argc;
	char *argv[];
{
	int do_cmd;
	char name[NAMSIZE], str[MAXLINE];
	register char c, savc;
 
	if ((tbuf.fildes = open(RAW_TAPE,READ))==ERROR) {
		perror("extract");
		exit(1);
	}
	for (;;) {
		if (gethdr(name)==EOF)
			break;
		do_cmd = match(name,argc,argv);
		if ((opt & WAIT) && do_cmd) {
			if (opt & VERBOSE)
				printf("x [%o,%o] %s ? ",group,user,name);
			else
				printf("x %s ? ",name);
			gets(str);
			c = str[0];
			if (c=='x')
				break;
			do_cmd = (c=='y');
		}
		if ((opt & VERBOSE) && do_cmd)
			printf("x %s ",name);
		if (do_cmd) {
			if ((out = fopen(name,"w"))==NULL) {
				perror("extract");
				break;
			}
			/* Starting with fresh buffer */
			tbuf.nleft = 0;
			while ((c = tgetch())!=EOF) {
				if ((c == '\r' ) && (opt & CRFLAG)) {
					savc = c;
					if ((c = tgetch())!='\n')
						putc(savc,out);
					if (c==EOF)
						break;
				}
				putc(c,out);
			}
			fclose(out);
			if (opt & VERBOSE)
				printf("ok\n");
		}
		else {
			while (read(tbuf.fildes,tbuf.buff,BUFSIZ) > 0)
				;
		}
	}
}

/*
 * Rewind tape and die.
 */

quit ()
{
	restore();
	exit(0);
}

/*
 * Write files onto tape in DOS format.
 */

replace (opt, argc, argv)
	int opt;
	int argc;
	char *argv[];
{
	int n, do_cmd;
	char str[MAXLINE], c;

	signal(SIGINT,quit);
	for (n = 2; n<argc; n++) {
		if (argv[n][0]=='-') {
			if (argv[n][1]=='g')
				sscanf(argv[n]+2,"%o",&group);
			else if (argv[n][1]=='u')
				sscanf(argv[n]+2,"%o",&user);
			else {
				printf("bad option \n");
				exit(1);
			}
			continue;
		}
		do_cmd = 1;
		if (opt&WAIT) {
			printf("r %s ? ",argv[n]);
			gets(str);
			c = str[0];
			if (c=='x')
				break;
			do_cmd = (c=='y');
		}
		if (do_cmd) {
			/* Starting with fresh buffer */
			tbuf.nleft = 0;
			if ((tbuf.fildes = open(NRAW_TAPE,WRITE))==ERROR) {
				perror("replace");
				exit(1);
			}

			if ((in = fopen(argv[n],"r"))==NULL) {
				perror("replace");
				break;
			}
			if (opt & VERBOSE)
				printf("r %s \n",argv[n]);
			puthdr(argv[n]);

			while ((c = getc(in)) != EOF) {
				if (c=='\n')
					tputch('\r');
				tputch(c);
			}
			if (tbuf.nleft & 1)	/* odd count */
				tputch('\0');
			tclose();
			fclose(in);
		}
	}
	if ((opt & NOREWIND)==0) {
		/* Rewind on close */
		restore();
	}
}

/*
 * Rewind tape.
 */

restore ()
{
	if (tbuf.fildes != -1 && tbuf.fildes != 0) {
		close(tbuf.fildes);
		close(open(RAW_TAPE,READ));
	}
}
/*
 * Return 1 if name matches any file or if no files.
 */

match (name, argc, argv)
	char *name;
	int argc;
	char *argv[];
{
	int n, g, u, f, octval;

	g = u = f = YES;

	for (n = 2; n < argc; n++)
		if (argv[n][0]=='-')
			if (argv[n][1]=='g') {
				sscanf(argv[n]+2,"%o",&octval);
				g = u = f = (octval == group);
			}
			else if (argv[n][1]=='u') {
				sscanf(argv[n]+2,"%o",&octval);
				u = f = (octval == user);
			}
			else {
				printf("bad option \n");
				exit(1);
			}
		else {
			f = (strcmp(argv[n],name)==0);
			if (g & u & f)
				return(YES);
		}
	return(g & u & f);
}

/*
 * Read in and decode DOS header, return name for file.
 */

gethdr (name)
	char *name;
{
	char DOSname[NAMSIZE];

	if (read(tbuf.fildes,&hd,HDSIZE) <= 0)
		return(EOF);
	r50toc(hd.FNAME[0],&DOSname[0]);
	r50toc(hd.FNAME[1],&DOSname[3]);
	DOSname[6] = '.';
	r50toc(hd.EXT,&DOSname[7]);

	getname(DOSname,name);
	group = hd.GROUP & 0377;
	user  = hd.USER & 0377;
	return(YES);
}

/*
 * Create and write DOS header.
 */

puthdr (name)
	char *name;
{
	char DOSname[NAMSIZE];

	putname(name,DOSname);

	hd.FNAME[0] = ctor50( DOSname );
	hd.FNAME[1] = ctor50(&DOSname[3]);
	hd.EXT = ctor50(&DOSname[7]);

	hd.GROUP = group;
	hd.USER  = user;
	hd.PROTECT = PROT_CODE;
	hd.UNUSEDBYTE = 0;
	hd.DATE = 0;	/* this should be fixed */

	if (write(tbuf.fildes,&hd,HDSIZE) == ERROR) {
		perror("puthdr");
		exit(1);
	}
}

/*
 * Get name from DOS style.
 */

getname (dosp, unixp)
	char *dosp, *unixp;
{
	register int i, j;

	for (i = 0, j = 0; dosp[i]; i++)
		if (dosp[i]!=' ')
			if (isupper(dosp[i]))
				unixp[j++] = tolower(dosp[i]);
			else
				unixp[j++] = dosp[i];
	unixp[j] = '\0';
	if (unixp[j-1]=='.')
		unixp[j-1] = '\0';
}

/*
 * Put name into DOS style.
 */

putname (unixp, dosp)
	char *unixp, *dosp;
{
	register int u, d;

	u = 0;
	for (d = 0; d<6; d++) {
		if (unixp[u]=='.' || unixp[u]=='\0')
			dosp[d] = ' ';
		else
			if (islower(unixp[u]))
				dosp[d] = toupper(unixp[u++]);
			else
				dosp[d] = unixp[u++];
		}
	while (unixp[u]!='.' && unixp[u]!='\0')
		u++;
	if (unixp[u]=='.')
		u++;
	dosp[6] = '.';
	for (d = 7; d<10; d++) {
		if (unixp[u]=='\0')
			dosp[d] = ' ';
		else
			if (islower(unixp[u]))
				dosp[d] = toupper(unixp[u++]);
			else
				dosp[d] = unixp[u++];
	}
	dosp[10] = '\0';
}

/*
 * Convert Radix-50 integer to ASCII string.
 */

char atable[] = {
	' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
	'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
	'X', 'Y', 'Z', '$', '.', '?', '0', '1',
	'2', '3', '4', '5', '6', '7', '8', '9'
};

r50toc (r50, str)
	unsigned r50;
	char *str;
{
	str[0] = atable[r50/03100L];
	r50 = r50%03100L;

        str[1] = atable[r50/050];
        str[2] = atable[r50%050];
        str[3] = '\0';
}

/*
 * Convert ASCII string to Radix-50.
 */

ctor50 (str)
	char *str;
{
	int r50, i, j;

	r50 = 0;
	for (i = 0; i<3; i++) {
		for (j = 0; j<050; j++)
			if (str[i]==atable[j])
				break;
		if (j==050) {
			printf("ctor50: bad char %c (\\%o)\n",str[i],str[i]);
			exit(1);
		}
		r50 *= 050;
		r50 += j;
	}
	return(r50);
}

/*
 * Buffered tape read.
 */

tgetch ()
{
	if (tbuf.nleft > 0) {
		--tbuf.nleft;
		return(*tbuf.nextp++ & 0377);
	}
	if ((tbuf.nleft = read(tbuf.fildes,tbuf.buff,BUFSIZ)) <= 0) {
		tbuf.nleft = 0;
		return(EOF);
	}
	tbuf.nextp = tbuf.buff;
	--tbuf.nleft;
	return(*tbuf.nextp++ & 0377);
}

/*
 * Buffered tape write.
 */

tputch (c)
	char c;
{
	if (tbuf.nleft == 1) {
		*tbuf.nextp = c;
		if (write(tbuf.fildes,tbuf.buff,BUFSIZ)==ERROR) {
			perror("tputch");
			exit(1);
		}
		tbuf.nleft = BUFSIZ;
		tbuf.nextp = tbuf.buff;
		return;
	}
	if (tbuf.nleft == 0) {
		tbuf.nleft = BUFSIZ;
		tbuf.nextp = tbuf.buff;
	}
	--tbuf.nleft;
	*tbuf.nextp++ = c;
}

/*
 * Flush tape write buffer and close.
 */

tclose ()
{
	if (tbuf.nleft != BUFSIZ && tbuf.nleft != 0)
		if (write(tbuf.fildes,tbuf.buff,BUFSIZ-tbuf.nleft)==ERROR) {
			perror("tclose");
			exit(1);
		}
	if (close(tbuf.fildes)==ERROR) {
		perror("tclose");
		exit(1);
	}
}
