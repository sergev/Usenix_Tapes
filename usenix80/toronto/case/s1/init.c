#
/*
 *	init - Unix Initialization Program
 */

#define	tabsize	49
#define	all		p = &itab[0]; p < &itab[49]; p++
#define	ever		;;
#define	single		0173030
#define	reboot		0173040
char	shell[]		"/bin/sh";
char	login[]		"/bin/login";
char	minus[]		"-";
char	runc[]		"/etc/rc";
char	init[]		"/etc/init";
char	ifile[]		"/etc/ttys";
char	utmp[]		"/etc/utmp";
char	wtmpf[]		"/usr/adm/wtmp";
char	ctty[]		"/dev/tty8";
int	fi;
int	status;
struct{
	int flag;
	int port;
	char coms[2];
}line;

struct tab{
	int	pid;
	int	line;
	int	comn;
	int	flg;
} itab[tabsize];

struct{
	char	name[8];
	char	tty;
	char	fill;
	int	time[2];
	int	wfill;
} wtmp;

main()
{
	register i;
	register struct tab *p, *q;
	int reset();

/*
 *	If not single user
 *	run shell sequence
 */
	if(getcsw() != single) {
		i = fork();
		if(i == 0) {
			open(ctty, 2);	/* was open("/", 0);	*/
			dup(0);
			dup(0);
			execl(shell, shell, runc, 0);
			exit(0);
		}
		while(wait(&status) != i);
		close(creat(utmp, 0644));
		wtmprec('~', 0);
	}
/*
 *	Main loop for hangup signal
 *	close all files and
 *	check switches for magic values
 */
	setexit();
	signal(1, reset);
	for(i=0; i<15; i++)
		close(i);
	switch(getcsw()) {

	case single:
	error:
		termall();
		i = fork();
		if(i == 0) {
			open(ctty, 2);
			dup(0);
			dup(0);
			execl(login, "login", "root", 0);
			exit(0);
		}
		while(wait(&status) != i);

	case reboot:
		termall();
		execl(init, minus, 0);
		reset();
	}
/*
 *	Open and merge in init file
 */
	fi = open(ifile, 0);
	q = &itab[0];
	while(rline()) {
		if(line.flag == '0')
			continue;
		for(all)
			if(p->line == line.port || p->line == 0) {
				if(p >= q) {
					i = p->pid;
					p->pid = q->pid;
					q->pid = i;
					p->line = q->line;
					p->comn = q->comn;
					q->line = line.port;
					q->coms[0] = line.comn;
					p->flg = q->flg;
					q->flg = line.flag;
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
		i = wait(&status);
		for(all)
			if(p->pid == i) {
				rmut(p);
				if((status & 0377) == i)
					wtmprec(p->line, 'h');
				else
					wtmprec(p->line, 0);
				dfork(p);
			}else if(p->pid == -1)
				dfork(p);
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
	p->flg = '0';
}

rline()
{
static char c[4];

	if(read(fi, c, 4) != 4 || c[3] != '\n')
		return(0);
	line.flag = c[0];
	line.port = c[1];
	line.comn = c[2];
	return(1);
}

dfork(ap)
struct tab *ap;
{
	register i;
	register char *tty;
	register struct tab *p;
	char *getty;

	p = ap;
	i = fork();
	if(i == 0) {
		signal(1, 0);
		tty = "/dev/ttyx";
		tty[8] = p->line;
		chown(tty, 0);
		chmod(tty, 0622);
		open(tty, 2);
/***************************
 *	Removed 12/18/78
 *
		wtmprec(p->line, 'o');
 ********************************************************/
		dup(0);
		dup(0);
		switch(p->flg) {
		case 'c':
			getty = "etc/cgetty";
			break;
		case '1':
		default:
			getty = "etc/getty";
			break;
		}
		execl(getty, minus, p->coms, 0);
		exit(0);
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
		seek(f, (i - '0')*16, 0);
		write(f, zero, 16);
		close(f);
	}
}

wtmprec(c, r)
char c, r;
{
register int f;

	f = open(wtmpf, 1);
	if( f >= 0 ) {
		wtmp.tty = c;
		time(wtmp.time);
		wtmp.fill = r;
		seek(f, 0, 2);
		write(f, &wtmp, 16);
		close(f);
	}
}
