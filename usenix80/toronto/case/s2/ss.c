#

/*
 *      ss - process status
 *	examine and print certain things about processes
 *
 *              written by Bob Greenberg
 *		mods by greg ordy, for 300 baud usage
 *		(addition of "+" option: without alot of stuff)
 */

# define SWAPPER "/dev/rk0"     /* name of swapper special device */

#include "/usr/sys/param.h"
#include "/usr/sys/proc.h"
#include "/usr/sys/tty.h"

struct {char low; char high; };
struct { int integer; };

struct proc proc[NPROC];
char procnames[NPROC][62];      /* bss stuff must stick together */
int ttys[64];                   /* from procnames to mode */
int sort[NPROC];
char one[8];
int texts[32];
int mode;
int wflg  -1;		/* flag for reduced output */

struct tty tty;
char partab[1];		/* kludge for new compiler, partab isn't used */
int stbuf[257];
int swap, mem;

char *states[] { "    ", "slp ", "wait", "RUN*", "idle", "zomb", "stpd" };

main(argc, argv)
char **argv;
{
	extern int fout;
	register int i, *n;
	register struct proc *p;
	int tsz;
	char c;

	nice(-120);
	fout = dup(1);
	if (argc>1)
	{
		switch (i = *argv[1]) {
		  case '-':
			switch (i = argv[1][1]) {
			  case '-':
				mode = -(getuid() & 0377);
				break;
			  case 0:
				mode = ttyn(2);
				break;
			  default:
				mode = i;
				break;
			}
			break;
		  case '+':
			wflg = 0; /* reduced output */
			break;
		  case '!':
			printin();
			return(0);
		  default:
			if (i >= 'a' && i <= 'z') mode = 1;
			else
			{
				fout = 1;
				printf("Type `ss !' for help.\n");
				return(1);
			}
			break;
		}
	}
	if ((swap = open(SWAPPER,0)) < 0 ||
	    (mem = open("/dev/mem",0)) < 0) err("can't open files\n");
	if (gprocs(0) != NPROC) err("NPROC mismatch--recompile ss.c\n");
	gprocs(proc);
	for (i = 1; i < NPROC; i++) if (proc[i].p_stat) gpname(i);
	if(wflg) printf("TTY PRI  SIZE STATE  USER     PID  COMMAND\n");
	if (mode == 1) getutmp(argv[1]);
	n = sort;
	for (i=0; i<NPROC; i++) if (c = ok(i)) *n++ = c | i<<8;
	if (mode <= 1) dosort();
	n = sort;
	if (!wflg)
	{
		while (c = n->low)
		{
			p = &proc[n->high];
			if(c == '%' || (p->p_uid & 0377) == 99)
			{
				n++;
				continue;
			}
			printf("%c:  ",c);
			goto inlp1;
			while(n->low == c)
			{
				printf("    ",c);
			  inlp1:p = &proc[n->high];
				printf("%.8s  %6l  %.40s\n",
					getln(p->p_uid&0377),
					p->p_pid,
					getname(n++ -> high));
			}
		}
		flush();
		return(0);
	}
	while (c = n->low)
	{
		printf("%c: ",c);
		goto inlp;
		while (n->low == c)
		{
			printf("   ");
		inlp:   p = &proc[n->high];
			if (tsz = textsz(p->p_textp))
				printf("%4d %2d+%2d %c%s %.6s %6l  %.44s\n",
				p->p_pri,tsz,(p->p_size+17)>>5,
				p->p_flag&SLOAD? '*' : ' ', states[p->p_stat],
				getln(p->p_uid&0377), p->p_pid,
				getname(n++ -> high));
			else printf("%4d %5d %c%s %.6s %6l  %.44s\n",
				p->p_pri, (p->p_size+37)>>5,
				p->p_flag&SLOAD? '*' : ' ', states[p->p_stat],
				getln(p->p_uid&0377), p->p_pid,
				getname(n++ -> high));
		}
		flush();
	}
	return(0);
}

getty(xx)
{
	register int *p, x;

	if ((x = xx) == 0) return('%');
	for (p=ttys; *p++;) if (*p++ == x) return(*(p-2));
	seek(mem,x,0);
	read(mem,&tty,sizeof tty);
	*p = x;
	return(*--p = gettynm(tty.t_dev));
}

/*
 *	get tty name based on major and
 *	minor device numbers.
 *	modified for Case Unix by W. A. Shannon
 */
gettynm(x)
{
	switch (x.high) {
	  case 0:
		return( x.low? '/' + x.low : '8');
	  case 4:
		return( x.low + 'a' );
	case 22:		/* pseudo-ttys */
		return( x.low + 'q' );
	  default:
		return('%');
	}
}

gpname(i)
{
	int laddr, baddr, mf;
	register int *ip, *cp, n;

	if (proc[i].p_stat == 5) return;
	baddr = 0;
	laddr = 0;
	if (proc[i].p_flag&SLOAD)
	{
		laddr = proc[i].p_addr;
		mf = mem;
	}
	else
	{
		baddr = proc[i].p_addr;
		mf = swap;
	}
	laddr =+ proc[i].p_size - 8;
	baddr =+ laddr>>3;
	laddr = (laddr&07)<<6;
	seek(mf, baddr, 3);
	seek(mf, laddr, 1);
	if (read(mf, stbuf, 512) != 512) return;
	for (ip = &stbuf[256]; ip > &stbuf[0];)
		if (*--ip == -1)
		{
			ip++;
			cp = procnames[i];
			n = 30;
			do *cp++ = *ip++; while (--n);
			return;
		}
}

err(s)
{
	printf(s);
	flush();
	exit(1);
}

getname(i)
{
	register char *p, *p2;
	register int c;
	int nbad;

	if (i==0) return("system scheduler");
	if (proc[i].p_stat == 5) return("(dead)");
	p2 = &(procnames[i][0]);
	if (*p2 == 0 && *(p2+1) == 0) return("?");
	nbad = 0;
	for (p = p2; p <= p2+60; p++)
		if ((c = *p) == 0) if (*(p+1) != 0) *p = ' '; else break;
		else if (c < ' ' || c > 0176)
		     {
			++nbad;
			*p = ' ';
		     }
	if (nbad > 9) return("?bad?");
	return(&(procnames[i][0]));
}

dosort()
{
	register int n, *p1, t;

	n = 1;
	while (n)
	{
		n = 0;
		p1 = sort;
		while (*(p1+1))
		{
			if (p1++ -> low > p1->low)
			{
				++n;
				t = *--p1;
				*p1++ = *(p1+1);
				*p1 = t;
			}
		}
	}
}

getln(x)
{
	register char *p;
	register int n;
	char buf[60];

	if ((n=x) == 0 || getpw(n,buf)) return("        ");
	p = buf;
	n = 0;
	while (*p++ != ':') n++;
	--p;
	while (n++ < 8) *p++ = ' ';
	*p = 0;
	return(buf);
}

textsz(tptr)
{
	register int tp, *p;

	if ((tp = tptr) == 0) return(0);
	for (p=texts; *p++;) if (*p++ == tp) return(*(p-2));
	*p = tp;
	seek(mem,tp + 4 /* x_size offset in text.h */,0);
	read(mem,--p,2);
	return(*p = (*p + 17) >> 5);
}

ok(ii)
{
	register int i, c;
	register char *p;

	i = ii;
	if (proc[i].p_stat == 0) return(0);
	c = getty(proc[i].p_ttyp);
	if (mode == 0 || (mode > 1 && c == mode) || -mode == proc[i].p_uid)
		return(c);
	if (mode != 1) return(0);
	p = one;
	while (*p) if (c == *p++) return(c);
	return(0);
}

printin()
{
	printf("\nArg Summary:\tss a1\n\na1\tMeaning\n");
	printf("blank\tfull listing\n");
	printf("-x\t`x' is a tty char- list just its processes\n");
	printf("+\tlisting without Nobody and system\n");
	printf("-\tlist just processes on one's own terminal\n");
	printf("--\tlist just processes owned by person running ss\n");
	printf("name\t`name' is a logname- list just processes on terminals");
	 printf("\n\ton which he is logged in\n");
	printf("!\tprint this summary\n");
   printf("\nAny other a1 causes a pointer to this message to be printed.\n");
	flush();
}

getutmp(x)
{
	register char *p2;
	register struct { char uname[8]; char utty; char pad[7]; } *p;
	register int n;
	int q;

	if ((q = open("/etc/utmp",0)) == -1) return;
	p2 = one;
	while (n = read(q,stbuf,512))
		for (p = stbuf; (n =- 16) >= 0; p++)
			if (equal(p->uname, x)) *p2++ = p->utty;
}

equal(aa,bb)
char *aa, *bb;
{
	register char *a, *b;

	a = aa;
	b = bb;
	while (*b++ == *a++);
	return(*--b == 0 && (*--a == ' ' || a-aa >= 8));
}

eq(aa,bb)
{
	register char *a, *b;
	a = aa; b = bb;
	while (*a++ == *b) if (*b++ == 0) return(1);
	return(0);
}


