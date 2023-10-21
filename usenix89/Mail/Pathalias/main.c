/* pathalias -- by steve bellovin, as told to peter honeyman */
#ifndef lint
static char	*sccsid = "@(#)main.c	8.1 (down!honey) 86/01/19";
#endif

#define MAIN	/* for sccsid in header files */

#include "def.h"

#define USAGE "usage: %s [-vci] [-l localname] [-d deadlink] [-t tracelink] [-g file] [-s file]\n"
extern	nodecount, linkcount;

main(argc, argv) 
int	argc; 
char	*argv[];
{
	node 	*Homenode;
	char	*locname = 0;
	int	c, errflg = 0;
	int	pstat;

	ProgName = argv[0];
	(void) allocation();	/* initialize data space monitoring */
	Cfile = "[deadlinks]";	/* for tracing dead links */

	meminit();

	while ((c = getopt(argc, argv, "cd:g:il:s:t:v")) != EOF)
		switch(c) {
		case 'c':	/* print cost info */
			Cflag++;
			break;
		case 'd':	/* dead host or link */
			deadlink(optarg);
			break;
		case 'g':	/* graph output file */
			Graphout = optarg;
			break;
		case 'i':	/* ignore case */
			Iflag++;
			break;
		case 'l':	/* local name */
			locname = optarg;
			break;
		case 's':	/* show shortest path tree */
			Linkout = optarg;
			break;
/*		case 't':	/* trace this link */
/*			if (tracelink(optarg) < 0) {
				fprintf(stderr, "%s: can trace only %d links\n", ProgName, NTRACE);
				exit(1);
			}
			Tflag = 1;
			break;
*/
		case 'v':	/* verbose stderr, mixed blessing */
			Vflag++;
			break;
		default:
			errflg++;
		}

	if (errflg) {
		fprintf(stderr, USAGE, ProgName);
		exit(1);
	}
	argv += optind;		/* kludge for yywrap() */

	if (*argv) {
		Ifiles = argv;
		freopen("/dev/null", "r", stdin);
	}

	if (!locname) 
		locname = local();
	if (*locname == 0) {
		locname = "lostinspace";
		fprintf(stderr, "%s: using \"%s\" for local name\n",
				ProgName, locname);
	}

	Home = addnode(locname);	/* add home node */
	Homenode = getnode(Home);
	Homenode->n_cost = 0;		/* doesn't cost to get here */
	putnode(Homenode);

	yyparse();			/* read in link info */

	vprintf(stderr, "yyparse ends with %d, %d\n", nodecount, linkcount);
	if (Vflag > 1)
		hashanalyze();
	vprintf(stderr, "%d vertices, %d edges\n", Ncount, Lcount);
	vprintf(stderr, "allocation is %uk after parsing\n", allocation());
	vprintf(stderr, "hashanalyze ends with %d, %d\n", nodecount,
								linkcount);

	Cfile = "[backlinks]";	/* for tracing back links */
	Lineno = 0;

	mapit();			/* compute shortest path tree */
	vprintf(stderr, " mapit ends with %d, %d\n", nodecount, linkcount);
	vprintf(stderr, "allocation is %uk after mapping\n", allocation());

	/* traverse tree and print paths */

	if (Cflag)
		pstat = system(PRINTITC);
	else
		pstat = system(PRINTIT);
	vprintf(stderr, "printit ends with status %d\n", pstat);

#ifndef DEBUG
	unlink(NTEMPFILE);
	unlink(LTEMPFILE);
#endif

	exit(0);
}
