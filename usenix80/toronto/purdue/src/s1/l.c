#
/* l    list file on tty pausing at the end of each screen and
	waiting for a carriage return.

purdue changes:
10/9/78 Ron Reeves   add -t switch to select multiple terminal types
		-ta selects adm3a terminal
		-tt selects tektronix 4014 terminal
		add routine to perform clear-screen
		add done routine to ensure exit status of 0
comment: this program is a big kludge taken from part of the
rand editor distribution

2/20/79 Scott Deerwester
		modify to exit at end of last file without requiring
		a return.
*/


#define TERM    /* define to allow multiple terminal types */

#define WIDTH 80
#define NUMW 8          /* width of numbers */

int page 23;
int first 0;
int numop 0;
int vtunix 0;   /* assume we have virtual terminal unix */
int lastfile 0;
int eof;

int fi; /* file descriptor for the terminal input device */
int envir;


struct
{       int fildes;
	int nleft;
	char *nextp;
	char buffer[512];
} finput,fout;

struct window
{   char firstcol, firstrow;
    char lastcol,lastrow;
};

struct
{   struct window curw;
    struct window echow;
    struct window maxw;
    int flags[2];
} args;

#ifdef TERM
int ttype 0;    /* terminal type 0=adm3a, 1=tek4014 */

char *clrtbl[] {        /* define clearscreen sequences */
	"\032\036",     /* adm3a */
	"\033\014",      /* tek4014 */
	"\014"		/* Ann-Arbor terminal (ttyw at Potter) */
	};

int clrlen[] {		/* define number of characters in clrlen */
	2,              /* adm3a */
	2,               /* tek4014 */
	1
	};

#endif


intrap()                /* got a  bad system call                           */
{       vtunix=0;       /* flag it for later                                */
}                       /* and return to after the gvty                     */

main(argc, argv)
int argc;
char *argv[];
{       register char c;
	register int i,k;
	int j,int2(),err,ldone();

	signal(12,&intrap);     /* bad arg to sys call? */
/*      err=gvty(2,&args,8); */ /* virtual terminal params */
	if (vtunix && err != -1)/* running off a virtual terminal */
		page=args.curw.lastrow-args.curw.firstrow-3;
	else if (vtunix) printf("gvty error\n");
	if (page<=0) page=1;    /* rather small window */
	k=0;
	for( i=1; i<argc; i++ )  /* look for - args */
	{	if( (c=argv[i][0]) == '-' )
		{	switch( argv[i][1] )
			{       case 'f': first=num(&argv[i][2]); /* set first Pine */
					  break;
				case 'n': numop=1; /* set number option */
					  break;
				case 'l':
				case 'p': page =num(&argv[i][2]); /* set page size */
					  if(page<=0) page=30000;
					  page--;
					  break;
#ifdef TERM     /* terminal definition switch */
				case 't': switch(argv[i][2]) {
					case 'a':       /* adm3a */
						ttype = 0;
						page = 23;
						break;
					case 't':       /* tek4014 */
						ttype = 1;
						page = 55;
						break;
					case 'w':	/* Potter ttyw */
						ttype = 2;
						page = 38;
						break;
					}
					break;

#endif

		}	}
		else argv[k++] = argv[i];
	}

	fout.fildes = dup(1);   /* JJG: for buffered tty output */
	signal(2,&int2); signal(3,&ldone);
	if(k == 0) /* filter */
	{       lastfile = 1;
		if (envsave(&envir)) pfile();
		ldone();
	}
	j = 0;
	envsave(&envir);
	while((i=j++) < k)
	{       if (finput.fildes) close(finput.fildes);
		if (k>1)
		{       clrscr();
			puts("\n\n\npress <RETURN> to list ");
			puts(argv[i]);
			nextpage(1);
		}
		lastfile = (i == k - 1);
		if (fopen(argv[i],&finput) == -1)  {
			puts("\n\ncannot open for reading!");
			nextpage(0);
			envrestore(&envir);
		}
		pfile();
	}
}

pfile()
{       register char c;
	register int line,col;
	int ct;
	clrscr();
	line = 1;
	col=WIDTH;
	if (numop) { prtnum(first+1); col=-NUMW;}
	ct = page;
/**/    while (!(eof = ((c = getc(&finput)) == -1)))
	{       if(line>first && --col>0)
		{       if(c == '\t') col =- 8 - ((WIDTH-col)&07);
			putchar(c);
		}
		if (c=='\n')
		{	if(line++ > first)
			{       if(col <= 0) putchar('\n');
				col = WIDTH;
				if(--ct == 0)
				{       nextpage(1);
					ct = page;
				}
				if(numop) { prtnum(line); col=-NUMW;}
			}
		}
	}
	nextpage(0);
}

num(s)	 /* computes the internal form of a number */
char *s; /* are bad chars are ignored */
{	register int c,i,sign;
	sign=1;  i=0;
	while(c = *s++)
	{	if(c=='-' && sign==1) sign = -1;
		c =- '0';
		if(c>=0 && c<=9) i=i*10+c;
	}
	return(i*sign);
}

char ds[] "00000 | ";
prtnum(line)       /* prints the line number */
int line;
{	register char *s;
	register int d,n;   n=line;
	d=n%10; ds[4]=(n?d+'0':' '); n=/10;
	d=n%10; ds[3]=(n?d+'0':' '); n=/10;
	d=n%10; ds[2]=(n?d+'0':' '); n=/10;
	d=n%10; ds[1]=(n?d+'0':' '); n=/10;
	d=n%10; ds[0]=(n?d+'0':' '); n=/10;
	for(s=ds;*s;) putchar(*s++);
}

nextpage (clearpage)
{       char c;
	flush();
	if (!eof || !lastfile) {
		do if(!read(2,&c,1))ldone(); while (c!='\n');
		if (clearpage) clrscr();
	}
	return;
}

int2()
{       signal(2,&int2);
	fout.nleft = fout.nextp = 0;
	envrestore(&envir);
}

puts(ss) char *ss;
{       register char *s, *t;
	s = t = ss;
	while (*t++);
	write(2,s,t-s);
}

/* clear the terminal screen
* the clear string is in clrtbl
* the length of the string is in clrlen
* and the tty type is in ttype */

clrscr()
{
#ifdef TERM
	write(2,clrtbl[ttype],clrlen[ttype]);

	/* if tektronix, wait for erase to occur */
	if(ttype == 1) sleep(2);
#endif

#ifndef TERM
	write(2,"\032\036",2);
#endif

	return;
}

/*
* exit routine for l.c
* all exits via this routine to ensure
* exit status 0
*/

ldone() {

	exit(0);
}
