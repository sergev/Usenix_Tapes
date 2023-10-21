#

/*
 *      l_csh - csh server (login)
 */

#include <stdio.h>

FILE    *fpass;
int     useruid 13;
char    userdir[]       "/usr/user";
char    command[256];

struct {
	char	name[8];
	char	tty;
	char	ifill;
	int	time[2];
	int	ufill;
} utmp;

char	mbuf[512];
int	now[2];

#define ROOT	0



main(argc,argv)
char **argv;
{
	login(argv[1]);
	fgets(command,sizeof command,stdin);
	execl("/bin/sh","sh_csh","-c",command,0);
	exit();
}

login(n)
{
	char pbuf[128];
	register char *namep, *np;
	char *np2;
	char pwbuf[9];
	int t, sflags, c, f, uid, gid, pri;
	FILE *fout;

	utmp.tty = 'x';
	time(now);
	namep = utmp.name;
	np = n;
	while ((c = *np++) != '\n') {
		if (c <= 0)
			exit();
		if (namep < utmp.name+8)
			*namep++ = c;
	}
	if(namep == utmp.name) {
		while(getchar() != '\n');
		setuid(useruid);
		chdir(userdir);
		nice(0);
		return;
	}
	while (namep < utmp.name+8)
		*namep++ = ' ';
	if (getpwentry(utmp.name, pbuf))
		goto none;
	np = colon(pbuf);
	if (*np!=':') {
		namep = pwbuf;
		while ((c=getchar()) != '\n') {
			if (c <= 0)
				exit();
			if (namep<pwbuf+8)
				*namep++ = c;
		}
		*namep++ = '\0';
		namep = crypt(pwbuf);
		while (*namep++ == *np++);
		if (*--namep!='\0' || *--np!=':') {
			goto bad;
		}
	} else {
		while(getchar() != '\n');
	}
	np = colon(np);
	uid = 0;
	while (*np != ':')
		uid = uid*10 + *np++ - '0';
	np++;
	gid = 0;
	while (*np != ':')
		gid = gid*10 + *np++ - '0';
	np++;
/* get the priority from the GCOS field */
	pri = 0;
	while (*np != ':')
		pri = pri*10 + *np++ - '0';
	np++;
	namep = np;
	np = colon(np);
	if (chdir(namep)<0) {
		write(1, "No directory.\n", 14);
		exit();
	}
	time(utmp.time);
	if (uid == ROOT && fork() == 0) {
		if ((fout = fopen("/dev/tty8", "w")) > 0) {
			fprintf(fout,"%s: root logged in on tty%c\n",
			    clock(utmp.time), utmp.tty);
		}
		exit();
	}
	nice(-pri);
	if ((f = open("/usr/adm/wtmp", 1)) >= 0) {
		seek(f, 0, 2);
		write(f, &utmp, 16);
		close(f);
	}
	setuid(uid);
	return;
bad:
	if (*pwbuf) {
		np = colon(np);
		uid = 0;
		while (*np != ':')
			uid = uid*10 + *np++ - '0';
		if (uid == ROOT && fork() == 0) {
			if ((fout = fopen("/dev/tty8", "w")) > 0) {
				time(utmp.time);
				fprintf(fout,"%s: attempted root login: \"%s\" at tty%c\n",
				    clock(utmp.time), pwbuf,
				    utmp.tty);
			}
			exit();
		}
	}
none:
	write(1, "Login incorrect.\n", 17);
	exit();
}

getpwentry(name, buf)
char *name, *buf;
{
	FILE *fin;
	int fi, r, c;
	register char *gnp, *rnp;

	r = 1;
	if ((fin = fopen("/etc/6passwd", "r")) < 0)
		goto ret;
loop:
	gnp = name;
	rnp = buf;
	while ((c=getc(fin)) != '\n') {
		if (c <= 0)
			goto ret;
		*rnp++ = c;
	}
	*rnp++ = '\0';
	rnp = buf;
	while (*gnp++ == *rnp++);
	if ((*--gnp!=' ' && gnp<name+8) || *--rnp!=':')
		goto loop;
	r = 0;
ret:
	fclose(fin);
	return(r);
}

colon(p)
char *p;
{
	register char *rp;

	rp = p;
	while (*rp != ':') {
		if (*rp++ == '\0') {
			write(1, "Bad password file.\n", 19);
			exit();
		}
	}
	*rp++ = '\0';
	return(rp);
}

clock(at) {
	register *p;
	register char *c;
	static char tbuf[12];

	p = localtime(at) + 4 * sizeof p;
	c = tbuf;

	*c++ = ++*p / 10 + '0';
	*c++ = *p % 10 + '0';
	*c++ = '/';
	*c++ = *--p / 10 + '0';
	*c++ = *p % 10 + '0';
	*c++ = ' ';
	*c++ = *--p / 10 + '0';
	*c++ = *p % 10 + '0';
	*c++ = ':';
	*c++ = *--p / 10 + '0';
	*c++ = *p % 10 + '0';
	*c = '\0';
	return(tbuf);
}
