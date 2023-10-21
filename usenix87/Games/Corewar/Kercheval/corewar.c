/*
 *	CoreWar
 *
 *	Programs battle for supremacy in a simulated arena!  Based on A.K
 *	Dewdeney's May 1984 column in Scientific American.
 *
 *	Usage:
 *		corewar [options] progs
 *			progs must be ascii redcode files (at least two).
 *		    -l: print listing while loading the redcode programs.
 *		    -t: print verbose execution trace.
 *		    -d: print post-mortem memory dump.
 *		    -s: print statistics.
 *		    -f: print memory dump locally at final pc's.
 *		    -x: extended redcode: allow fork. [AEB]
 *		    -o: stop when no more memory owned.
 *		    -i: stop when not_own instruction executed.
 *		    -m20: set maturity limit to 20 (default 0).
 *		    -n20: set max number of processes to 20 (default 20).
 *		    -d20: set minimum distance from other processes for a
 *				newly spawned process.
 *
 *	This program assumes your C compiler supports structure assignment.
 *
 *	Copyright 1984, Berry Kercheval.  All rights reserved.  Permission
 *	for distribution is granted provided no direct commercial
 *	advantage is gained, and that this copyright notice appears 
 *	on all copies.
 *	Extensively changed by mcvax!play . You get the changes for free!
 */

#include <stdio.h>
#include <signal.h>
#include "corewar.h"

memword	mem[MEMSIZE];		/* the simulated memory 'arena' */
int	nr_of_own_locs[MAXPROCNUM+1];	/* and how many each of us controls */

char	*pr_inst();		/* returns formatted string representation of a
				 * redcode instruction suitable for %s */
char	letter();		/* returns " ABCD..."[arg] */
int (*oldintrup)();

int	dumpflag = 0;		/* TRUE --> print post-mortem dump */
int	traceflag = 0;		/* TRUE --> print execution trace */
int	listflag = 0;		/* TRUE --> print listing of loaded program */
int	statflag = 0;		/* TRUE --> print various statistics */
int	localdumpflag = 0;	/* TRUE --> print local dump at termination */
int	forkflag = 0;		/* TRUE --> allow forks */
int	opto;			/* stop when no more own locs */
int	opti;			/* stop when not_own instruction executed */
int	min_fork_dist = 1;
int	max_no_of_procs = 20;	/* Note: not the same as MAXPROCNUM ! */
int	mature_at = 0;		/* Age at which fork is allowed */

long int cyclect = 0;		/* global time */
int	procct;			/* total nr of processes alive */
int	procno = 1;		/* 1 + total nr of procs without ancestor */
int	running = 1;
struct proc *head_procs, *procs[MAXPROCNUM+1];
char 	*file[MAXPROCNUM+1];

main (argc, argv)
int argc;
char **argv;
{
	address	pc;		/* address of program starting point.
				 * filled in by load() */
	address load();		/* loads a file */
	int	draw();		/* what happens if we time out */
	int	intrup();	/* or are interrupted */

	/* allow both "corewar -f -s" and "corewar -fs" style options */
	while (argc > 1 && argv[1][0] == '-' && argv[1][1]) {
		argc--;
		argv++;
		while(*++*argv) switch (**argv) {
		/* first look at the options that may have a numerical arg */
		case 'm':	/* maturity threshold */
			mature_at = atoi(*argv + 1);
			printf("Set maturity age to %d.\n", mature_at);
		skipdigits:
			while(digit(argv[0][1])) ++*argv;
			break;
		case 'n':	/* num of procs */
			max_no_of_procs = atoi(*argv + 1);
			printf("Max number of processes: %d\n",
				max_no_of_procs);
			goto skipdigits;
		case 'd':
			if(digit(argv[0][1])) {	/* min fork dist */
				min_fork_dist = atoi(*argv + 1);
				printf("Min dist to others when forking: %d\n",
					min_fork_dist);
				goto skipdigits;
			}
			dumpflag = 1;	/* print memory dump at end */
			break;
		case 'f':	/* print local memory dump at termination */
			localdumpflag = 1;
			break;
		case 'l':	/* print listing at load-time */
			listflag = 1;
			break;
		case 't':	/* print execution trace */
			traceflag = 1;
			break;
		case 's':	/* print statistics */
			statflag = 1;
			break;
		case 'x':	/* allow forks */
			forkflag = 1;
			break;
		case 'o':
			opto = 1;
			break;
		case 'i':
			opti = 1;
			break;
		default:
		    printf("Unknown flag '%s'\n", *argv);
		    printf("Flags are:\n");
		    printf("\t-l\tprint listing at load time\n");
		    printf("\t-d\tprint memory dump at termination\n");
		    printf("\t-f\tprint local memory dump at termination\n");
		    printf("\t-t\tprint execution trace\n");
		    printf("\t-s\tprint statistics\n");
		    printf("\t-x\textend redcode with FORK instruction\n");
		    printf("\t-o\tstop process that owns no more memory\n");
		    printf("\t-i\tstop process that executes foreign instr\n");
		    printf("\t-m20\tset maturity age to 20\n");
		    printf("\t-n25\tset max number of processes to 25\n");
		  printf("\t-d1\tset min dist to others while forking to 1\n");
		    exit(1);
		}
	}

	/* now the last two arguments should be names of two redcode files */
	if (argc < 3) {
		printf("Usage: corewar [options] file1 file2 ...\n");
		exit(1);
	}

	(void) srand(getpid());
	clear_mem();			/* Clear the arena! Clear the arena! */

	while(argc > 1) {
		argc--;
		argv++;
		if ( access (*argv, 4) != 0) { /* check read access */
			perror(*argv);
			exit (-1);
		}
		if(procno > MAXPROCNUM) {
			printf("We cannot run more than %d", MAXPROCNUM);
			printf(" processes simultaneously.\n");
			printf("Subsequent ones are ignored.\n");
			break;
		}
		file[procno] = *argv;
		pc = load(*argv, procno);
		printf("Executing %s at %d.\n\n", *argv, pc);
		procs[procno] = makeproc((struct proc *) 0, pc);
		procno++;
	}

	outmemcontrol();

	(void) signal (SIGALRM, draw);	/* set up for timeout */
	(void) alarm (TIMEOUT);		/* give up after TIMEOUT seconds */

	/* if we are not ignoring interrupts then catch them */
	oldintrup = signal (SIGINT, SIG_IGN);
	if (oldintrup == SIG_DFL)
		(void) signal (SIGINT, intrup);

	if(traceflag && procno > 3)
		traceflag = 0;		/* I am lazy */
	if(traceflag)
	{
		printf ("       %10s                                 %10s\n",
			file[2], file[1]);
		printf ("-----------------------------------------------------------\n");
	}
	while (running) {
		cyclect++;
		xeq_all();
		if(opto) {
			register int j, ct = 0;

			for(j = 1; j < procno; j++)
				if(nr_of_own_locs[j] > 0) ct++;
			if(ct < 2)
				running = 0;
		}
	}
	done();
}

done()
{
	(void) alarm (0);			/* turn off the alarm */
	(void) signal(SIGINT, oldintrup);	/* reset intrup handling */
	if(forkflag) {
		register struct proc *p;

		for(p = head_procs; p; p = p->next_proc)
			if(p->ancestor) {
				p->ancestor->no_of_alive_offspring +=
					p->no_of_alive_offspring;
				p->ancestor->no_of_dead_offspring +=
					p->no_of_dead_offspring;
			}
	}
	{ long int omax, ohere;
	  register int j, oct;
	  int bestj[MAXPROCNUM+1];
		omax = 0;
		for(j = 1; j < procno; j++) {
			ohere = procs[j]->no_of_alive_offspring;
			if(procs[j]->status) ohere++;
			if(ohere > omax) {
				omax = ohere;
				oct = 0;
				bestj[oct++] = j;
			} else if(omax == ohere) {
				bestj[oct++] = j;
			}
		}
		if(!omax) {
			printf("Draw - all died.\n");
		} else if(oct == 1) {
			j = bestj[0];
			printf("%s(%d) wins.\n",
				file[j], j);
		} else if(procno == oct+1)
			printf("DRAW!\n");
		else {
			printf("Tie between: ");
			for(j=0; j<oct; j++)
			    printf("%s%s(%d)", j ? " and " : "",
				file[bestj[j]], bestj[j]);
			printf("\n");
		}
	}
	if (statflag)			/* statistics if requested */
		statistics();
	if (dumpflag || localdumpflag)	/* post-mortem dump if requested */
		dump();
	exit(0);			/* Th-th-that's all folks */
}

/*
 *	clear_mem:	clear the simulated memory
 */

clear_mem()
{
	address i;

	for ( i=0 ; i < MEMSIZE; i++ )
	{
		mem[i].op = 0;
		mem[i].a_mode = 0;
		mem[i].b_mode = 0;
		mem[i].a = 0;
		mem[i].b = 0;
		mem[i].owner = 0;
	}
	nr_of_own_locs[0] = MEMSIZE;
}

/*
 *	load:
 *		Load a redcode file at a random place im mem.
 *		Avoid placing files too close together.
 */

address
load(filename, owner)
char *filename;
short owner;
{
	address	r;		/* where to load current instruction */
	FILE	*f;		/* stream pointer for input file */
	char	buf[256];	/* line buffer */
	char 	*ptr;		/* pointer into the line; used in parsing */
	char	*ip;		/* pointer into the line; used in parsing */
	char	*index();
	char	error;		/* error flag printed at beginning of each 
				 * listing line; currently either ' ' or 'E' */
	address	start;		/* address of first executable instruction */
	int	op;		/* op-code value */
	int	i;		/* counter for for loops */

	/* select starting address */

	start = -1;		/* undefined */
	{
		register int mindist = DISTANCE+1;
		register int j;

	try_r:
		r = (rand() >> 12) % MEMSIZE;	/* 4.1 rand() sucks */
		mindist--;
		/*
		 * Second program must be at least DISTANCE addresses from
		 * first; I just make their starting-points about that much
		 * apart.
		 */
		for(j = 1; j < procno; j++)
			if(dist(r, procs[j]->pc) < mindist)
				goto try_r;
	}
	printf ("Loading %s at %d\n", filename, r);

	/* now do load */

	f = fopen (filename, "r");
	if (f == NULL ) {
		perror (filename); exit(-1);
	}
	/*
	 * There now follows a moderately crufty ad-hoc redcode assembler.
	 * It's not modular or very structured, but it seems to work, and
	 * redcode was so simple I didn't want to use YACC or LEX or SSL
	 */

	while ( fgets ( buf, 256, f) != NULL)	/* for each line in the file */
	{
		error = ' ';			/* no error yet */
		/* zap trailing newline to make listing generation easier */
		if ( (ptr = index(buf, '\n')) != NULL ) *ptr = '\0';

		/* zap comment */
		if ( (ptr = index(buf, '/')) != NULL ) *ptr = '\0';
		 
		/* decode instruction */

		ip = buf;	/* start at the beginning of the line */
		op = -1;	/* Invalid op-code */

		/* skip leading whitespace */
		while ( *ip && (*ip == ' ' || *ip == 011)) ip++;	
		if ( ip == ptr ) {	/* it's a 'blank' line */
			if ( ptr != NULL ) *ptr = '/';	/* put comment back */
			if (listflag)
				printf ("%c                      '%s'\n",
					error, buf);
			break;
		}
		if ( strncmp(ip, op_str[FORK], 4) == 0) {
			op = FORK;
			ip += 4;
		} else
		for ( i = 0; i <= CMP; i++)	/* CMP is highest op-code */
		{
			if ( strncmp(ip, op_str[i], 3) == 0){
				op = i;
				ip += 3;
				break;
			}
		}
		if ( op == -1 ) 		/* didn't find it! */
		{
			printf ("SYNTAX ERROR '%s' -- Bad opcode\n", buf);
			error = 'E';
			ip++;
		}
		mem[r].op = op;

		/* skip whitespace */
		while ( *ip && (*ip == ' ' || *ip == 011)) ip++;

		/* figure out addressing mode for operand A */
		if ( op != DAT ) { 	/* DAT has only B operand */
			if ( *ip == '#'){
				mem[r].a_mode = IMMEDIATE; 
				ip++;
			}
			else if ( *ip == '@') {
				mem[r].a_mode = INDIRECT; 
				ip++;
			}
			else 
			{
				mem[r].a_mode = DIRECT;
			}
			mem[r].a = atoi ( ip );
			if ( mem[r].a < 0 ) mem[r].a += MEMSIZE;
			while ( *ip && (*ip != ' ' && *ip != 011)) ip++;
		}
		/* skip whitespace */
		while ( *ip && ( *ip == ' ' || *ip == 011)) ip++;	

		if ( op != JMP && op != FORK) {	/* these have only A operand */
			if ( *ip == '#'){
				mem[r].b_mode = IMMEDIATE; 
				ip++;
			}
			else if ( *ip == '@') {
				mem[r].b_mode = INDIRECT; 
				ip++;
			}
			else 
			{
				mem[r].b_mode = DIRECT;
			}
			mem[r].b = atoi ( ip );
			if ( mem[r].b < 0 ) mem[r].b += MEMSIZE;
		}

		/* check for start of executable code */
		if ( op != DAT && start == -1) start = r;

		/* DAT has zero modes... */
		if ( op == DAT ) mem[r].b_mode = mem[r].a_mode = 0;

		nr_of_own_locs[mem[r].owner]--;
		nr_of_own_locs[owner]++;
		mem[r].owner = owner;

		/* Do listing stuff... */

		if ( ptr != NULL ) *ptr = '/';		/* put comment back */
		if (listflag)
			printf ("%c%4d %s  '%s'\n", error, r, pr_inst(mem[r]),
				buf);
		r++;			/* Advance to next memory location */
		if ( r >= MEMSIZE ) r %= MEMSIZE;
	}

	if (listflag)
		(void) fflush (stdout);
	(void) fclose (f);
	/* return starting address */
	return start;
}

xeq_all()
{
	register struct proc *p;

	for(p = head_procs; p; p = p->next_proc)
		if(p->status)
			execute(p);
}

execute (p)
register struct proc *p;
{

	memword inst;
	address addr,final;
	short me;

	/* fetch instruction */
	addr = p->pc;
	inst = mem[addr];
	me = inst.owner;

	/* separate the two instruction traces */
	if (traceflag && (p == procs[1] || p == procs[2])) {
		if(p == procs[1])
			printf ("     ||     ");
		printf ("@ %d  %s    ", addr, pr_inst(inst));
		if(p == procs[1] || !procs[1]->status)
			printf ("\n");
		(void) fflush (stdout);
	}

	switch ( inst.op )
	{
		case MOV:
		case ADD:
		case SUB:
			if (do_mas(addr, inst, me))
				goto badinstr;
			addr++;
			break;
		case JMP:
			addr += inst.a;
			break;
		case JMZ:
			final = (addr + inst.b) % MEMSIZE;
			if ( mem[final].op == DAT && mem[final].b == 0 )
				addr += inst.a;
			else
				addr++;
			break;
		case JMG:
			final = (addr + inst.b) % MEMSIZE;
			if ( mem[final].op == DAT && mem[final].b > 0 )
				addr += inst.a;
			else
				addr++;
			break;
		case DJZ:
			final = (addr + inst.b) % MEMSIZE;
			if(mem[final].b == 0)
				mem[final].b = MEMSIZE-1;
			else
				mem[final].b -= 1;
			if ( mem[final].b == 0 )
				addr += inst.a;
			else
				addr++;
			{ register int o = mem[final].owner;

			  if(o && o != me) {
				mem[final].owner = 0;
				nr_of_own_locs[o]--;
				nr_of_own_locs[0]++;
			  }
			}
			break;
		case CMP:
			addr += do_cmp(addr, inst);
			break;
		case FORK:
			if(forkflag) {
				final = (addr + inst.a) % MEMSIZE;
				(void) makeproc(p, final);
				addr++;
				break;
			}
			/* fall into next case */
		case DAT:
		default:
		badinstr:
			if(!p->ancestor) {
			    if(traceflag && p == procs[2]) printf("\n");
			    printf ("%s(%d): Illegal instruction %s @ %d\n",
				file[p->from], p->from,
				pr_inst(inst), addr);
			    if(traceflag && p == procs[2])
				printf("\t\t\t    ");
			}
			procdied(p);
			return;
	}

	/* test first now - we prefer "Illegal instr" above "Not own instr" */
	if (opti && p->from != me) {
		if(!p->ancestor) {
		    if(traceflag && p == procs[2]) printf("\n");
		    printf ("%s(%d): Not own instruction %s @ %d\n",
			file[p->from], p->from,
			pr_inst(inst), p->pc);
		    if(traceflag && p == procs[2])
			printf("\t\t\t    ");
		}
		procdied(p);
		return;
	}

	p->pc = addr % MEMSIZE;
}

/*
 * CMP: compare a and b, return 1 if same, 2 if different 
 */

do_cmp(addr, inst)
address addr;
memword inst;
{
	address source, destination;
	memword data;

	data.op = 0;
	data.a_mode = data.b_mode = 0;
	data.a = data.b = 0;

	switch (inst.a_mode)
	{
		case IMMEDIATE:
			data.b = inst.a;
			break;
		case DIRECT:
			data = mem[(addr + inst.a) % MEMSIZE];
			break;
		case INDIRECT:
			source = mem[(addr + inst.a) % MEMSIZE].b;
			data =   mem[(source + addr + inst.a) % MEMSIZE];
			break;
		default: /* ERROR */
			printf ("do_cmp: illegal addressing mode\n");
			return 1;
	}
	switch (inst.b_mode)
	{
		case IMMEDIATE: /* error */
			if (data.b == inst.b) return 1;
			else return 2;
		case DIRECT:
			if ( data.b == mem[(addr + inst.b) % MEMSIZE].b)
				return 1;
			else return 2;
		case INDIRECT:
			destination = mem[(addr + inst.b) % MEMSIZE].b;
			if(data.b == mem[(destination+addr+inst.b)%MEMSIZE].b)
				return 1;
			else return 2;
		default: /* ERROR */
			printf ("do_cmp: illegal addressing mode\n");
			return 1;
	}	
}

do_mas(addr, inst, me)
address addr;
memword inst;
{
	address source, destination, final;
	memword data;

	data.op = 0;
	data.a_mode = data.b_mode = 0;
	data.a = data.b = 0;
	data.owner = me;

	switch (inst.a_mode)
	{
		case IMMEDIATE:
			data.b = inst.a;
			break;
		case DIRECT:
			data = mem[(addr + inst.a) % MEMSIZE];
			break;
		case INDIRECT:
			source = mem[(addr + inst.a) % MEMSIZE].b;
			data =   mem[(source + addr + inst.a) % MEMSIZE];
			break;
		default: /* ERROR */
			printf ("do_mas: illegal addressing mode\n");
			return 1;
	}
	switch (inst.b_mode)
	{
		case IMMEDIATE: /* error */
			printf ("do_mas: illegal immediate destination\n");
			return 1;
		case DIRECT:
			final = (addr + inst.b) % MEMSIZE;
			break;
		case INDIRECT:
			destination = mem[(addr + inst.b) % MEMSIZE].b;
			final = (destination + addr + inst.b) % MEMSIZE;
			break;
		default: /* ERROR */
			printf ("do_mas: illegal addressing mode\n");
			return 1;
	}
	switch(inst.op) {
	case SUB:
		data.b = -data.b;
		/* fall into next case */
	case ADD:
		mem[final].b = (mem[final].b + data.b) % MEMSIZE;
		{ register int o = mem[final].owner;
		  if(o && o != me){
			nr_of_own_locs[o]--;
			nr_of_own_locs[0]++;
			mem[final].owner = 0;
		  }
		}
		break;
	case MOV:
		{ register int o = mem[final].owner;
		  register int od = data.owner;
		  if(od != me)
			data.owner = od = 0;
		  if(o != od){
			nr_of_own_locs[o]--;
			nr_of_own_locs[od]++;
		  }
		}
		mem[final] = data;
	}
	return 0;
}

dump()
{
	int r;
	int flag = 0;

	printf ("\n\n----------%s MEMORY DUMP -------------\n",
		localdumpflag ? " LOCAL" : "");

	for ( r = 0; r < MEMSIZE; r++) {
		if(localdumpflag) {
			register struct proc *p;
			int here = 0, rmin = MEMSIZE+1, local = 0;

			/* find dist 0-LDW with wraparound, larger without */
			for(p = head_procs; p; p = p->next_proc) {
				if(r > p->pc + LDW) continue; /* too far */
				if(r == p->pc) {
					here++;
					goto localx;
				}
				if(dist(r, p->pc) <= LDW)
					local++;
				else if(!local && p->pc < rmin)
					rmin = p->pc - LDW;
			}
			if(local)
				goto localx;
			r = rmin-1;
			flag = 0;
			continue;
		localx:
			if(here) flag = 0;
			if(!flag || !iszero(mem[r]))
				printf(here ? ">" : " ");

		}

		if ( iszero(mem[r]) && flag == 0) {
			printf ("  %5d 0\n", r);
			flag = 1;
		}
		else if (iszero(mem[r]) && flag == 1) {
			printf ("   *\n");
			flag = 2;
		}
		else if (iszero(mem[r]) && flag == 2) {
			/* skip it */
		}
		else
		{
			printf ("%c %5d %s\n", letter(mem[r].owner),
				r, pr_inst(mem[r]));
			flag = 0;
		}
	}
}

iszero(x)
memword x;
{
	if (x.op == 0 && x.owner == 0 && x.a_mode == 0 && 
	    x.b_mode == 0 && x.a == 0 && x.b == 0 )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

draw()
{
	printf ("Timed out after %d seconds\n", TIMEOUT);
	done();
}

intrup()
{
	printf("Interrupted.\n");
	done();
}

char
letter(n)
int n;
{
	return( (n == 0) ? ' ' : (1 <= n && n <= 26) ? '@'+n : 'a'+n-27);
}

char *
pr_inst(x)
memword x;
{
	char buf[200];

	(void) sprintf(buf,"%s%s%c%04d, %c%04d",
		op_str[x.op], (x.op == FORK) ? "" : " ",
		mode_char[x.a_mode], x.a,
		mode_char[x.b_mode], x.b);
	return buf;
}

outmemcontrol()
{
	register int j;

	printf("Memory control: ");
	for(j = 1; j < procno; j++) {
		if(j % 5 == 0) printf("\n\t");
		printf("%s: %d, ", file[j], nr_of_own_locs[j]);
	}
	printf("no_one: %d\n", nr_of_own_locs[0]);
	(void) fflush(stdout);
}

statistics()
{
	register int j;

	printf("Nr of cycles: %ld\n", cyclect);

	outmemcontrol();

	printf("Final pc's:");
	for(j = 1; j < procno; j++) {
		if(j % 5 == 0) printf("\n\t");
		printf(" %s: %ld", file[j], procs[j]->pc);
	}
	printf("\n");

	if(forkflag) {
		register struct proc *p;

		for(j = 1; j < procno; j++)
			descstat(procs[j], file[j]);
		for(p = head_procs; p; p = p->next_proc) if(p->status)
			printf("\t%c at %d was born at time %ld\n",
				letter(p->from), p->pc, p->time_of_birth);
	}
}

descstat(p,name)
register struct proc *p;
register char *name;
{
	printf("%s: %s.", name, p->status ? "alive" : "dead");
	if(p->no_of_alive_offspring + p->no_of_dead_offspring)
		printf(" Descendants: %ld (alive) %ld (dead).\n",
			p->no_of_alive_offspring,
			p->no_of_dead_offspring);
	else
		printf("\n");
}	

dist(r1, r2)
address r1,r2;
{
	register int d = (r1 > r2) ? r1-r2 : r2-r1;
	if(d > MEMSIZE-d)
		d = MEMSIZE-d;
	return d;
}

struct proc *
makeproc(ancestor, place)
struct proc *ancestor;
address place;
{
	register struct proc *proc;
	extern char * malloc();

	/*
	 * We need some means of controlling the number of processes;
	 * here I have chosen for max_no_of_procs and min_fork_dist. Note that
	 * some code gets rather inefficient (and should be rewritten)
	 * if max_no_of_procs is increased too much.
	 */
	if(ancestor) {
		if(procct >= max_no_of_procs ||
		    ancestor->time_of_birth + mature_at > cyclect)
			return((struct proc *) 0);
		for(proc = head_procs; proc; proc = proc->next_proc)
		    if(dist(proc->pc, place) < min_fork_dist)
			return((struct proc *) 0);
	}
	proc = (struct proc *) malloc(sizeof(struct proc));
	if(!proc) {
		if(!ancestor) {
			/* cannot create one of the two parent processes ? */
			printf("malloc fails.");
			exit(1);
		}
		return((struct proc *) 0);  /* child asphyxates immediately */
	}
	procct++;
	proc->next_proc = head_procs;
	head_procs = proc;
	proc->ancestor = ancestor;
	proc->pc = place;
	proc->status = 1;	/* alive */
	proc->time_of_birth = cyclect;
	proc->no_of_alive_offspring = proc->no_of_dead_offspring = 0;
	if(ancestor){
		ancestor->no_of_alive_offspring++;
		proc->from = ancestor->from;
	} else {
		proc->from = procno;
	}
	return(proc);
}

procdied(proc)
register struct proc *proc;
{
	register int who = proc->from;

	procct--;
	if(!proc->ancestor)
		proc->status = 0;	/* dead */
	else {
		register struct proc *p;

		proc->ancestor->no_of_alive_offspring +=
			proc->no_of_alive_offspring - 1;
		proc->ancestor->no_of_dead_offspring +=
			proc->no_of_dead_offspring + 1;
		if(head_procs == proc)
			head_procs = proc->next_proc;
		else
			for(p = head_procs; p; p = p->next_proc)
			    if(p->next_proc == proc) {
				p->next_proc = proc->next_proc;
				break;
			    }
		for(p = head_procs; p; p = p->next_proc)
			if(p->ancestor == proc)
				p->ancestor = proc->ancestor;
		free((char *) proc);
	}

	if(!procs[who]->status && !procs[who]->no_of_alive_offspring){
		register int ct = 0;

		/* check whether there are any others left */
		for(who = 1; who < procno; who++)
			if(procs[who]->status ||
			   procs[who]->no_of_alive_offspring)
				ct++;
		if(ct < 2)
			running = 0;
	}

	/* perhaps we want to report something here? */
	/* note that we probably just freed proc */
}

digit(c)
char c;
{
	return('0' <= c && c <= '9');
}
