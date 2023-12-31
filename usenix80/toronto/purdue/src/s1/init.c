#define XSTTY	/* use Purdue extended STTY call */
#define	tabsize	127
#define	all	p = &itab[0]; p < &itab[tabsize]; p++
#define	ever	;;
#define	single	0173030
#define	reboot	0173040
#define singlex	0000070	/* run one single user shell, then go multi-user */
# ifdef XSTTY
#define BLITZ 02000	/* stty BLITZ bit in speed word */
# endif
char	shell[]	"/bin/sh";
char	minus[]	"-";
char	runc[]	"/etc/rc";
char	init[]	"/etc/init";
char	ifile[]	"/etc/ttys";
char	utmp[]	"/etc/utmp";
char	wtmpf[]	"/usr/adm/wtmp";
char	ctty[]	"/dev/tty8";
int	fi;
struct
{
	int	flag;
	int	line;
	char	coms[2];
} line;
struct	tab
{
	int	pid;
	int	line;
	int	comn;
	int     flg[2];
} itab[tabsize];

struct {
	char	name[8];
	char	tty;
	char	fill;
	int	time[2];
	int	wfill;
} wtmp;

# ifdef XSTTY
int	modes[3];	/* for stty blitz on open */
# endif
main()
{
	register i;
	register struct tab *p, *q;
	int reset();

/* If Switches == singlex, handle as single user first time only -ghg */

	signal(1,1);
	if(getcsw() == singlex)
		singleusr();

	/*
	 * if not single user,
	 * run shell sequence
	 */

	if(getcsw() != single) {
		i = fork();
		if(i == 0) {
			open("/", 0);
			dup(0);
			dup(0);
			execl(shell, shell, runc, 0);
			exit();
		}
		while(wait() != i);
		close(creat(utmp, 0644));
		if ((i = open(wtmpf, 1)) >= 0) {
			seek(i, 0, 2);
			wtmp.tty = '~';
			time(wtmp.time);
			write(i, &wtmp, 16);
			close(i);
		}
	}

	/*
	 * main loop for hangup signal
	 * close all files and
	 * check switches for magic values
	 */

	setexit();
	signal(1, reset);
	for(i=0; i<10; i++)
		close(i);
	switch(getcsw()) {

	case single:
	error:
		singleuser();

	case reboot:
		termall();
		execl(init, init, 0);
		reset();
	}

	/*
	 * open and merge in init file
	 */

	fi = open(ifile, 0);
	q = &itab[0];
	while(rline()) {
		if(line.flag == '0')
			continue;
		for(all)
			if(p->line==line.line || p->line==0) {
				if(p >= q) {
					i = p->pid;
					p->pid = q->pid;
					q->pid = i;
					p->flg[0] = q->flg[0];
					p->line = q->line;
					p->comn = q->comn;
					q->line = line.line;
					q->coms[0] = line.comn;
					q->flg[0] = line.flag;
					q++;
				}
				break;
			}
	}
	close(fi);
	if(q == &itab[0])
		goto error;
	for(; q < &itab[tabsize]; q++)
		term(q);
	for(all)
		if(p->line != 0 && p->pid == 0)
			dfork(p);
	for(ever) {
		i = wait();
		for(all)
			if(p->pid == i) {
				rmut(p);
				dfork(p);
			}
	}
}

termall()
{
	register struct tab *p;

	for(all)
		term(p);
}

term(ap)
struct tab *ap;
{
	register struct tab *p;

	p = ap;
	if(p->pid != 0) {
		rmut(p);
		kill(p->pid, 9);
	}
	p->pid = 0;
	p->line = 0;
}

rline()
{
	static char c[4];

	if(read(fi, c, 4) != 4 || c[3] != '\n')
		return(0);
	line.flag = c[0];
	line.line = c[1];
	line.comn = c[2];
	return(1);
}

dfork(ap)
struct tab *ap;
{
	register i;
	register char *tty;
	register struct tab *p;

	p = ap;
	i = fork();
	if(i == 0) {
		signal(1, 0);
		tty = "/dev/ttyx";
		tty[8] = p->line;
		chown(tty, 0);
		chmod(tty, 0602);
		while(open(tty, 2) < 0)
			sleep(15);
# ifdef XSTTY
		gtty(0,&modes);		/* blitz terminal */
		modes[0] =| BLITZ;
		stty(0,&modes);
		close(0);
		while(open(tty, 2) < 0)
			sleep(15);
# endif
		dup(0);
		execl("etc/getty", minus, p->coms, p->flg, 0);
		exit();
	}
	p->pid = i;
}

rmut(p)
struct tab *p;
{
	register i, f;
	static char zero[16];

	f = open(utmp, 1);
	if(f >= 0) {
		i = p->line;
		seek(f, i*16, 0);	/* change mapping 10/17/77 --ghg */
		write(f, zero, 16);
		close(f);
	}
	f = open(wtmpf, 1);
	if (f >= 0) {
		wtmp.tty = p->line;
		time(wtmp.time);
		seek(f, 0, 2);
		write(f, &wtmp, 16);
		close(f);
	}
}

singleusr()
{
	register i;

	termall();
	i = fork();
	if(i == 0) {
		open(ctty, 2);
		dup(0);
		execl(shell, minus, 0);
		exit();
	}
	while(wait() != i);
}
