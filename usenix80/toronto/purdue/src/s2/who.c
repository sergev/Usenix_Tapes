#
#define	UTMP	2048 /* at 16 bytes each, we can have up to 128 ttys */

/*
 * who [-s] [-h] [-rnn] [who-file] [am i]
 *
 *
 *    -h:history file "usr/adm/wtmp"
 *    -rnn:the last nn entries of the history file,default is 192
 *    --KT 02/12/78.
 *    -tx: who is at ttyx(if anyone)
 *    -- HDS 02/17/78.
 *    fixed so if /u/adm/wtmp is too short, it will not go into
 *    an infinite loop.  Instead, it prints the whole file. 
 *    --kt 02/26/78
 *    -s: print out who listing in alphabetically-sorted order
 *    -- HDS
 */

int	fout;
int	buf[256];
char ubuff[UTMP];
int nttys, comp(), readin(), rewrite();

main(argc, argv)
char **argv;
{
	char *s, *cbuf;
	int n,fi,nm,i,seeker,tflag,tty, badflag, sflag;
	struct {
		char name[8];
		char tty;
		char pad1;
		int time[2];
		char pad2[2];
	} *p;

	sflag = 0;
	s = "/etc/utmp";
	seeker = badflag = 0;
	tflag=0;
	if(argc == 2) {
		s = argv[1];
		if(s[0] == '-' && (s[1] == 'h' || s[1] == 'r')){
			if(s[1]=='r'){
				nm=192;
				seeker=1;
				if(s[2])
					nm=num(&s[2]);
			}
			s = "/usr/adm/wtmp";
		}
		else
			if(s[0] == '-' && s[1] == 't'){
				tty=s[2];
				s = "/etc/utmp";
				tflag++;
			}
			else
				if(s[0] == '-' && s[1] == 's'){
					nttys = readin();
					qsort(ubuff,nttys,16,&comp);
					s = mktemp("/tmp/whoaXXXXX");
					rewrite(s);
					sflag++;
				}
	}
	nm = nm *16;
	fi = open(s, 0);
	if(sflag)
		unlink(s);
	if(fi < 0) {
		printf("cannot open history file: %s\n", s);
		exit();
	}
	if (seeker)
		if(seek(fi,-nm,2)<0)
			printf("cannot seek on %s\n",s);
	fout = dup(1);
	close(1);
	if(argc == 3)
		tty = ttyn(0);
loop:
	n = read(fi, buf, 512);
	if(n<0){
		if(seek(fi,0,0)<0){
			printf("can't seek on %s\n",s);
			exit(1);
		}
		if(badflag){
			printf("can't read file\n");
			exit(1);
		}
		badflag++;
		goto loop;
		}
	if(n == 0) {
		flush();
		if (argc==3)
			write(fout, "Nobody.\n", 8);
		exit();
	}

	p = &buf;
	for(p = &buf; (n =- 16)>=0; p++) {
		if ((argc==3 || tflag) && tty!=p->tty)
			continue;
		if(p->name[0] == '\0' && (argc==1 || tflag || sflag))
			continue;
	    if(p->tty > '{') {
		switch(p->tty) {
			case '~': printf("system  boot"); break;
			case '|': printf("old sys date"); break;
			case '}': printf("new sys date");
		}
		putchar(' ');
	    } else {
		for(i=0; i<8; i++) {
			if(p->name[i] == '\0')
				p->name[i] = ' ';
			putchar(p->name[i]);
		}
		putchar(' ');
		for(i=0; i<3; i++)
			putchar("tty"[i]);
		putchar(p->tty);
	    }
		cbuf = ctime(p->time);
		for(i=3; i<16; i++)
			putchar(cbuf[i]);
		putchar('\n');
		if (argc==3 || tflag) {
			flush();
			exit();
		}
	}
	goto loop;
}
num(as)
char *as;
{
	register n;
	n=0;

	while ('0' <= *as && *as <= '9')
		n = n * 10 + *as++ - '0';
	if (*as) {
		write(2, "bad number\n",11);
		n=0;
	}
	return(n);
}
readin()
{
	int utmp,x;

	if((utmp = open("/etc/utmp",0)) < 0){
		printf("can't open utmp file\n");
		exit(1);
	}
	x = read(utmp,ubuff,UTMP);
	return(x / 16);
}
comp(a,b)
register char *a, *b;
{
	register i;

	for(i = 0;i < 8;i++)
		if(a[i] < b[i])
			return(-1);
		else
			if(a[i] > b[i])
				return(1);
	return(0);
}
rewrite(tfile)
char *tfile;
{
	int tf;

	if((tf = creat(tfile,0604)) < 0){
		printf("can't create tempfile\n");
		exit(1);
	}
	write(tf,ubuff,nttys * 16);
	close(tf);
}
