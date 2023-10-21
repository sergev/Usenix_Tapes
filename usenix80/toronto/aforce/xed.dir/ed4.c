#
#include	"/doc/source/prog/s1.afdsc/xed.dir/externals.h"
/*
 * Get process ID routine if system call is unavailable.
getpid()
{
	register f;
	int b[1];

	f = open("/dev/kmem", 0);
	if(f < 0)
		return(-1);
	seek(f, 0140074, 0);
	read(f, b, 2);
	seek(f, b[0]+8, 0);
	read(f, b, 2);
	close(f);
	return(b[0]);
}
 */
ilsubst() {
	register int *a1, nl;
	extern int getsub();
	for(a1=addr1; a1<=addr2; a1++) {
		getline(*a1);
		ildo();
		*a1=putline();
		nostty++;
		nl=append(getsub,a1);
		a1=+ nl;
		addr2=+ nl;
	}
}


ildo() {
	char *t1, *t2, *tp;
	int c,tmp,n,minus,tmp2;
restart:
	n=0; tp= &linebuf[0];
	while(*tp) {
		n++;
		tp++;
	}
	fr= &genbuf[LBSIZE-1];
	*fr = '\0';
	while(n--) *--fr = *--tp;
	to = &genbuf[0];
	for(;;) {
		n=1; minus=0; c=ilgetchar();
		if(c=='-') minus++; else peekc=c;
		if((c=ilgetchar())>='0' && c<='9') {
			n=0;
			do {
				n=* 10;
				n=+ c-'0';
			} while((c=ilgetchar())>='0' && c<='9');
		}
		else if (c=='$') {
			n = &genbuf[LBSIZE]-fr;
			c = ilgetchar();
			}
		if(minus) n= -n;
		switch(c) {
		case '#':
		case CTLH:
			n= -n;
		case ' ':
			if(n<0) while(n++ && ilmove(LEFT,COPY));
			else while(n-- && ilmove(RIGHT,COPY));
			break;
		case 'e':
		case 'E':
		case CTLA:
			n= -n;
		case 'd':
		case 'D':
			if(n<0) while(n++ && ilmove(LEFT,NOCOPY));
			else while(n-- && ilmove(RIGHT,NOCOPY));
			break;
		case 'c':
		case 'C':
			if(n<0) n= -n;
			while(n--) {
				tmp=ilgetchar();
				if(! *fr) break;
				if(tmp==EOT) break;
				*fr = tmp;
				ilmove(RIGHT,COPY);
			}
			break;
		case 'r':
		case 'R':
			if(n<0) while(n++ && ilmove(LEFT,NOCOPY));
			else while(n-- && ilmove(RIGHT,NOCOPY));
		case 'i':
		case 'I':
			while((tmp=ilgetchar())!=EOT) {
				if(to==fr) break;
				if (tmp == CTLH) ilmove(LEFT,NOCOPY);
				else {
					*--fr = tmp;
					ilmove(RIGHT,COPY);
				}
			}
			break;
		case '@':
		case CTLX:
			puts("");	/* output a new line */
			goto restart;
		case 'f':
		case 'F':
			show();
			goto finish;
		case '\n':
			show();
			while(ilmove(RIGHT,COPY));
		finish:
			*to = 0;
			to= &linebuf[0];
			fr= &genbuf[0];
			while(*to++ = *fr++);
			write(1,"\n",1);
			hiding=0;
			return;
		case 'p':
		case 'P':
			show();
			tp=fr; while(*tp) write(1,tp++,1);
			write(1,"\n",1);
			tp = &genbuf[0];
			while(tp != to) write(1,tp++,1);
			break;
		case 'l':
		case 'L':
			show();
			tp=fr; while(*tp) write(1,tp++,1);
			write(1,"\n",1);
			n = to - &genbuf[0];
			while(n--) *--fr = *--to;
			break;
		case 's':
		case 'S':
			tmp=ilgetchar();
			if(n<0) while(n++) {
				ilmove(LEFT,COPY);
				while(to != &genbuf[0] && *(to-1) != tmp)
				ilmove(LEFT,COPY);
			} else while(n--) {
				ilmove(RIGHT,COPY);
				while(*fr && *fr!= tmp)
				ilmove(RIGHT,COPY);
			}
			break;
		case 'w':
		case 'W':
			if(n<0) {
				while(n++) {
					while(*fr && *fr != ' ')
						ilmove(LEFT,NOCOPY);
					ilmove(LEFT,NOCOPY);
				}
			}
			else {
				while(n--) {
					while(*fr && *fr != ' ')
						ilmove(RIGHT,NOCOPY);
					ilmove(RIGHT,NOCOPY);
				}
			}
			break;
		case 'k':
		case 'K':
			tmp=ilgetchar();
			t1=fr; t2=to;
			if(n<0) {
				while(n++) {
					--t2;
					while(t2 != &genbuf[0] && *--t2 != tmp);
					if(*t2==tmp) {
						tmp2=to-t2;
						while(tmp2--) ilmove(LEFT,NOCOPY);
					}
				}
			} else {
				while(n--) {
					t1++;
					while(*t1 && *t1!=tmp) t1++;
					if(*t1) {
						tmp2=t1-fr;
						while(tmp2--) ilmove(RIGHT,NOCOPY);
					}
				}
			}
			break;
		}
	}
}

char ilmove(dir,cp) int dir,cp; {
	char c;
	if(dir==LEFT) {
		if(to == &genbuf[0]) return(0);
		hide();
		c = *--to;
		write(1,&c,1);
		if(cp==COPY) *--fr = c;
		return(c);
	} else {
		if(! *fr) return(0);
		if(cp==COPY) show(); else hide();
		c = *fr++;
		write(1,&c,1);
		if(cp==COPY) *to++ = c;
		return(c);
	}
}

ilgetchar() {
	char c,d;
	c = getchar();
	if(c == CTLC) {
		putchar('\n');
		c = '\n';
		hiding=0;
		ERROR;
	}
	if(c == '\\') {
		d = getchar();
		if(d=='@' || d=='#' || d==CTLH || d==CTLA || d=='\\')return(d);
		if(ttmode[2] &  04) {
			/* half-ascii terminal escapes */
			if(d >= 'a' && d <= 'z') return(d-'a'+'A');
			switch(d) {
			case '!':	return('|');
			case '\'':	return('`');
			case '^':	return('~');
			case '(':	return('{');
			case ')':	return('}');
			}
		}
		peekc = d;
		return(c);	/* ASSERT: c == backslash */
	}
}

show() {
	if(hiding) {
		write(1,"\\",1);
		hiding=0;
	}
}

hide() {
	if(!hiding) {
		write(1,"\\",1);
		hiding=1;
	}
}

printlines()
{
	register char *p,*s;
	register int *a1;
	nonzero();
	for (a1 = addr1; a1 <= addr2; a1++) {
		if (listf == 2) {
			count[1] = (a1-zero) & 077777;
			putd(&count);
			putchar('\t');
			}
		p = getline(*a1);
		if (zflag) for (s=p; *s; s++) if (*s==NP) {
			*s = 0;
			puts(p);
			dot = a1+1;
			goto exit;
			}
		puts(p);
		}
	dot = addr2;
	exit:
	listf = 0;
	}

readfile(addr)
{
	extern getfile();

	if ((io = open(file, 0)) < 0) errfunc("Can't open");
	ninbuf = 0;
	fchanged = 0100000; /* don't flush during read */
	nostty++;
	append(getfile, addr);
	flushio();
	exfile();
}

next() 
{
	int f;
	if((f = open(nextfile,0)) < 0)  {
		ERROR;			/* tell user */
		return;
	}
	savin = dup(0);			/* save old input */
	close(0);			/* free descriptor */
	dup(f);				/* make new file std input */
	close(f);			/* free extra version */
	savvflag = vflag;		/* save prompt status */
	vflag  = 0;			/* suppress prompts */
	push(&curdepth);
	commands(2);			/* process next file commands */
	pop(&curdepth);
}
setmargin()
{
	char c, ymode;
	while((c = getchar()) == ' ');
	if((c == '+') || (c == '-')) {
		ymode = c;
		c = getchar();
	}
	else {
		ymode = '+';
	}
	peekc = c;
	switch(ymode) {
		case '-':
			margin = LBSIZE - 40;
			yflag = 0;
			break;
		case '+':
			margin = LENGTH - 20;
			yflag++;
			break;
	}
}

setxonoff() {
	char c, bmode;
	while((c = getchar()) == ' ');
	if((c == '+') || (c == '-')) {
		bmode = c;
		c = getchar();
	}
	else
		bmode = '+';
	peekc = c;
	switch(bmode) {
		case '-':
			bflag = 0;
			break;
		case '+':
			bflag++;
			break;
	}
}

help()
{
	char   c;
	char   *p;
	char   *s;
	char   cprhsbuf[LBSIZE/2];
	int    *i;
	int    zcount[2];

	while((c = getchar()) == ' ');
	switch(c)
	{
		case  '/':
		case  'g':
		case  'G':
		case  'v':
		case  'V':
			write(1,"/",1);
			lftstring();
			write(1,"/\n",2);
			break;
		case  'w':
		case  'W':
		case  'r':
		case  'R':
		case  'f':
		case  'F':
			p = &savedfile;
			puts(p);
			break;
		case  'x':
		case  'X':
			p = &nextfile;
			puts(p);
			break;
		case  's':
		case  'S':
			write(1,"s",1);
			c = delimit;
			if(c < 040 || c > 0172)		c = '/';
			write(1,&c,1);
			lftstring();
			p = &rhsbuf;
			s = &cprhsbuf;
			while(*s++ = *p++);
			*--s = '\0';
			write(1,&c,1);
			s = &cprhsbuf;
			while(*s != '\0')
			{
				if(*s < 0)
				{
					write(1,"\\",1);
					*s =& 0177;
				}
				c = *s++;
				if(c == '\t')	write(1,"\\t",2);
				else	write(1,&c,1);
			}
			c = delimit;
			if(c < 040 || c > 0172)		c = '/';
			write(1,&c,1);
			write(1,"\n",1);
			break;
		case  'y':
		case  'Y':
			if(yflag)	puts("y+");
			else	puts("y-");
			break;
		case  'z':
		case  'Z':
			zcount[1] = zlength;
			putd(&zcount);
			putchar('\n');
			break;
		default:
			ERROR;
			break;
	}
	while((c = getchar()) != '\n');
}

lftstring()
{
	char   *p;
	char   c;
	int    n;

	p = &expbuf;
	if(circfl)	write(1,"^",1);
	if(*p == '\0')		return;
	while(*p != CEOF)
	{
		switch(*p)
		{
			case  CBRA:
				p++;
				write(1,"\\(",2);
				p++;
				break;
			case  CCHR:
				p++;
				c = *p++;
				if (c == '*' || c == '.' || c == '$' || c == '[' || c == ']' || c == '^' || c == '\\')
				{
					write(1,"\\",1);
					write(1,&c,1);
				}
				else	if(c == '\t')	write(1,"\\t",2);
				else	write(1,&c,1);
				break;
			case  CCHR|STAR:
				p++;
				c = *p++;
				write(1,&c,1);
				write(1,"*",1);
				break;
			case  CDOT:
				p++;
				write(1,".",1);
				break;
			case  CDOT|STAR:
				p++;
				write(1,".*",2);
				break;
			case  CCL:
				p++;
				write(1,"[",1);
				n = *p++;
				n--;
				for(; n > 0; n--)
				{
					c = *p++;
					if(c == '\t')	write(1,"\\t",2);
					else	write(1,&c,1);
				}
				write(1,"]",1);
				break;
			case  CCL|STAR:
				p++;
				write(1,"[",1);
				n = *p++;
				n--;
				for(; n > 0; n --)
				{
					c = *p++;
					if(c == '\t')	write(1,"\\t",2);
					else	write(1,&c,1);
				}
				write(1,"]*",2);
				break;
			case  NCCL:
				p++;
				write(1,"[^",2);
				n = *p++;
				n--;
				for(; n > 0; n--)
				{
					c = *p++;
					if(c == '\t')	write(1,"\\t",2);
					else	write(1,&c,1);
				}
				write(1,"]",1);
				break;
			case  NCCL|STAR:
				p++;
				write(1,"[^",2);
				n = *p++;
				n--;
				for(; n > 0; n --)
				{
					c = *p++;
					if(c == '\t')	write(1,"\\t",2);
					else	write(1,&c,1);
				}
				write(1,"]*",2);
				break;
			case  CDOL:
				p++;
				write(1,"$",1);
				break;
			case  CKET:
				p++;
				write(1,"\\)",2);
				p++;
				break;
			default:
				p++;
				break;
		}
	}
}
