/*
** mars.c -	corewar machine interpreter
**
**	[cw by Peter Costantinidis, Jr. @ University of California at Davis]
*/

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include "cw.h"

static	char	rcsid[] = "$Header: mars.c,v 6.0 85/12/13 00:31:16 ccohesh001 Dist $";

char	*argv0;
int	jobcnt = 0;		/* # of jobs */
int	jobsleft;		/* # of jobs left running */
int	min_sep = PAGESEP;	/* initial min. dist. between jobs */
int	max_cyc = CYCLES;	/* min. # of process cycles */
int	cycle;			/* # of cycles */
static	process	pcs[MAX_USERS];

/*
** the following variables are used globally and rarely passed
** to any other functions
*/
long	octrl = DEF_CTRLS;	/* output control variable */
memword	memory[MAX_MEM+1];

extern	void	fini(), init(), randomize(), trap(), zeromem();
extern	int	pcscompar();
extern	char	*diedstr();

/*
** main()	- the main program
**		- parse arguments, call initialization routines, start game
**		  and call de-initialization routines
*/
main (argc, argv)
reg	int	argc;
reg	char	**argv;
{
	reg	int	i;
	auto	int	arg;
	extern	int	optind;

	argv0 = *argv;
	while ((arg = getopt(argc, argv, "s:c:dlipvr:")) != EOF)
	{
		switch (arg)
		{
			extern	char	*optarg;

			when 'r':	/* randomize */
				if (*optarg == '0')
					randomize(0);
				else
					randomize(atoi(optarg));
			when 's':	/* separation */
				if ((min_sep = atoi(optarg)) < 0)
					min_sep = 1;
				else if (min_sep > (MAX_PAGE-1))
				{
					fprintf(stderr, "%s: separation too large (must be < %d)\n", argv0, MAX_PAGE-1);
					usage();
				}
			when 'c':	/* cycles */
				max_cyc = atoi(optarg);
			when 'd':	/* status window display */
				TOGGLE(STATUS);
			when 'l':	/* print starting stats */
				TOGGLE(SIZE_START);
			when 'i':	/* print instructions */
				TOGGLE(PINST);
			when 'p':	/* don't print cycle # */
				TOGGLE(PASSNUM);
			when 'v':	/* visual display */
				TOGGLE(VISUAL);
			otherwise:
			case '?':
				usage();
		}
	}
	argc -= optind, argv += optind;
	if (argc < 2)
	{
		fprintf(stderr, "%s: missing file arguments\n", argv0);
		usage();
	}
	if (ON(VISUAL) && ON(PINST))
		TOGGLE(PINST);
	if (ON(PINST))
		octrl |= PASSNUM;
	zeromem();
	if (ON(SIZE_START))
		printf("%d words of memory available\n", MAX_MEM);
	if (argc > MAX_USERS)
	{
		fprintf(stderr, "%s: only %d jobs at once\n", argv0, MAX_USERS);
		exit(1);
	}
	for (i=jobcnt=0; *argv && i<argc; i++, argv++, jobcnt++)
	{
		pcs[i].pname = *argv;
		pcs[i].pdied = EALIVE;
		pcs[i].picnt = 0;
		pcs[i].plstmem = (memword *) NULL;
		pcs[i].pid = jobcnt;
		if ((pcs[i].pc = get_start()) < 0)
		{
			fprintf(stderr, "%s: not all jobs will fit\n", argv0);
			exit(1);
		}
		if (!(pcs[i].psize = load(pcs[i].pname, pcs[i].pc)))
		{
			fprintf(stderr, "%s: couldn't read in player \"%s\"\n",
				argv0, pcs[i].pname);
			exit(1);
		}
		if (ON(SIZE_START))
			printf("Player \"%s\" @ %4d, size: %d\n",
				pcs[i].pname, pcs[i].pc, pcs[i].psize);
	}
	if (ON(SIZE_START))
	{
		auto	char	buf[BUFSIZ];

		printf("Hit return to continue");
		gets(buf);
	}
	init();
	fini(scheduler(pcs));
	exit(0);
}

/*
** init()	- perform various initializations
*/
static	void	init ()
{
	jobsleft = jobcnt;
	if (ON(VISUAL))
	{
		if (vinit())
		{
			fprintf(stderr, "%s: visual not working\n", argv0);
			exit(1);
		}
		if (ON(STATUS) && statinit(pcs))
		{
			msg("Status window not working\n");
			TOGGLE(STATUS);
		}
	}
	trap(0);
	if (!ON(VISUAL))
	{
		auto	char	buf[BUFSIZ];

		printf("Press return to start ");
		(void) gets(buf);
		return;
	}
	msg("Press any key to start");
	getchar();
	vupdate(pcs);
}

/*
** trap()	- catches signals
**		- signum is zero for the initial call, non-zero
**		  when an interrupt is recieved
**		- when interrupt is received, call fini()
*/
static	void	trap (signum)
reg	int	signum;
{
#ifndef	LINT	/* keeps lint from complaining */
	signal(SIGINT, SIG_IGN);	/* i realize that these 2 statements */
	signal(SIGHUP, SIG_IGN);	/* could follow the if */
#endif
	if (!signum)
	{
#ifndef	LINT
		signal(SIGINT, trap);
		signal(SIGHUP, trap);
#endif
		return;
	}
	fini(EINTERRUPT);
	exit(1);
}

/*
** fini()	- misc. things to be done before termination
**		- print status at termination of processes
*/
void	fini (status)
reg	int	status;
{
	reg	int	i;
	auto	process	*spcs[MAX_USERS];

	if (ON(VISUAL))
	{
		msg("Hit return to continue");
		getchar();
		if (ON(STATUS))
			statfini();
		vfini();
	}
	printf("Halted: %s\n", diedstr(status));
	for (i=0; i < jobcnt; i++)
	{
		if (!pcs[i].pdied)
			pcs[i].picnt = cycle;
		spcs[i] = &(pcs[i]);
	}
	qsort(spcs, jobcnt, sizeof(spcs[0]), pcscompar);
	printf("%-20s\t#Inst\tPC\tReason for termination\n", "Process");
	for (i=0; i<jobcnt; i++)
		printf("%-20s\t%5d\t%4d\t%s\n",
			spcs[i]->pname, spcs[i]->picnt, spcs[i]->pc,
			diedstr(spcs[i]->pdied));
}

/*
** pcscompar()	- compare function used by qsort
*/
int	pcscompar (p1, p2)
reg	process	**p1, **p2;
{
	return((*p2)->picnt - (*p1)->picnt);
}

/*
** diedstr()	- return a string describing the reason for termination
*/
char	*diedstr (status)
reg	int	status;
{
	switch (status)
	{
		when EALIVE:
			return("Still alive");
		when ECYCLE:
			return("Cycle limit exceeded");
		when EWIN:
			return("Winner found");
		when EILLINST:
			return("Bad instruction");
		when EBADMODE:
			return("Bad addressing mode");
		when EINTERRUPT:
			return("Interrupted");
		otherwise:
			return("Unknown reason");
	}
	/*NOTREACHED*/
}

/*
** get_start()	- get a starting location for process
**		- starting locations on page boundaries
**		- return negative on error
**
**	Assumption:
**		If start location can't be found, then there are too many jobs.
*/
int	get_start ()
{
	reg	int	i;
	reg	int	start;

	start = rand() % MAX_MEM;
	start -= start % PAGESIZE;
	for (i = 0; i < MAX_USERS; i++)
	{
		if (!collision(start))
			return(start);
		start = sum(start, (min_sep * PAGESIZE));
	}
	return(-1);
}

/*
** collision()	- return TRUE if address is taken, else FALSE
*/
int	collision (addr)
reg	int	addr;
{
	reg	int	i;
	auto	int	ll,	/* lower limit to range */
			ul;	/* upper limit to range */

	for (i=0; i<jobcnt; i++)
	{
		ll = sum(pcs[i].pc, -min_sep);
		ul = sum(sum(pcs[i].pc, pcs[i].psize), min_sep);
		if (pcs[i].pc >= ul || pcs[i].pc <= ll)
		{	/* wrapped around memory */
			if (addr <= ul || addr >= ll)
				return(TRUE);
			continue;
		}
		if (addr >= ll && addr <= ul)
			return(TRUE);
	}
	return(FALSE);
}

/*
** load()	- load the player
**		- return the size of the player (in instructions), 0 on error
*/
int	load (fname, start)
char	*fname;
reg	int	start;
{
	reg	int	i, status;
	reg	FILE	*fp;

	if (!(fp = fopen(fname, "r")))
	{
		perror(fname);
		return(0);
	}
	for (i=start;
	     (status=fread(&(memory[i]),sizeof(memory[i]),1,fp)) == 1; i++)
		continue;
	fclose(fp);
	if (status < 0)
		return(0);
	return(i - start);
}

/*
** zeromem()	- set all memory locations to zero
*/
void	zeromem ()
{
	reg	int	i;

	for (i=0; i<MAX_MEM; i++)
		memory[i].op = memory[i].val = 0;
}

/*
** randomize()	- randomize the random number generator
*/
static	void	randomize (seed)
reg	int	seed;
{
	extern	long	getpid();
	extern	long	time();

	if (seed)
	{
		srand(seed);
		return;
	}
	srand(seed = (int) ((time(0) + getpid()) & 0x0000ffffL));
	printf("seed: %d\n", seed);
}

/*
** usage() -	print a usage message and exit
*/
static	void	usage ()
{
	fprintf(stderr, "Usage: %s [-dilpv] [-rn] [-sn] [-cn] file ...\n", argv0);
	exit(1);
}
