#include <stdio.h>
#include <sgtty.h>
#include <debug.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
void (*signal())();
void (*moresig)();
FILE *fdopen();

long int size, position;
char filename[30];
char isaconsole = '\0';
char isapipe = '\0';
int tabsize = 8;
#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define ESC '\033'
#define IBMPC			/* display code dependent on PC hardware/bios */
#ifdef IBMPC
extern void scr_putc();
#endif
void std_out(c)
{
	write(1,&c,1);
}
void (*output)();
#ifdef MAIN
crlf()
{
	write(2,"\r\n",2);
}
#endif
jmp_buf moreenv;
moreintr()
{
	signal(SIGINT,SIG_IGN);	/* ignore signals until we finish putting stuff
	                           back in order */
	longjmp(moreenv,-1);
}
#ifdef MAIN
main
#else
more
#endif
(argc,argv)
	int argc;
	char *argv[];
{
	FILE *fp, *fopen();
	long ftell();
	long int i;
	long int fsize();

	if (-1 == setjmp(moreenv))
	{
		write(2,"Interrupted\r\n",13);
		fclose(fp);
		signal(SIGINT,moresig);
		return -1;
	}

	moresig = signal(SIGINT,moreintr);
	isaconsole = isatty(STDOUT);
	isapipe = !isatty(STDIN);
#ifdef IBMPC
	if (isaconsole)
	{
		output = scr_putc;
		scr_echo(0);
	}
	else
#endif
		output = std_out;
	if ( (*(++argv))[0] == '-' && isdigit((*argv)[1]) )
	{
		tabsize = atoi( (*argv+1) );
		--argc;
	} else
	{
		--argv;
	}
	if (argc == 1)
	{
		if (NULL == (fp = fdopen(0,"r")))
		{
			return -1;
		}
		display(fp);
		crlf();
		return 0;
	}
	while(--argc) 
	{
		if (NULL == (fp = fopen(*(++argv),"r")) )
		{
			fprintf(stderr,"more - can't open %s\n",*argv);
			continue;
		}
		strncpy(filename,*argv,30);
		if (filename[29])
			filename[29] = '\0';
		if (!isapipe)
			size = fsize(fp);
		position = 0;
		if (-2 == display(fp))
			argc = 0;		/* force completion of command */
		fclose(fp);
	}
bugout:
#ifdef IBMPC
	if (isaconsole)
		scr_echo(0);
#endif
	crlf();
	tabsize = 8;
	isaconsole = isapipe = '\0';
	signal(SIGINT,moresig);
	return 0;
}

long fsize(fp) 
	FILE *fp;
{
	long ftell();
	long position, last;
	position = ftell(fp);
	if (-1 == fseek(fp,0L,2))
	{
		fprintf(stderr,"more - error on fseek\n");
	}
	last = (ftell(fp));
	fseek(fp,position,0);
	return last;
}
#define LBUFSIZE 160
char linebuffer[LBUFSIZE];
int lines;

display(fp)
	FILE *fp;
{
	FILE *fgets();
	long ftell();
	char c;
	lines = 1;
	while ( NULL != fgets(linebuffer,LBUFSIZE,fp))
	{
		if (isaconsole)
		{
			if (lines == 1)		/* top of display */
			{
#ifdef IBMPC
				scr_clear();
#else
/* ansi terminal screen clear */
				printf("%c[2J",ESC);	/* clear display */
#endif
			}
		}
		lines += localputs(linebuffer);
		if (isaconsole)
		{
			if (lines >=  24)		/* bottome of display */
			{
				char tst;
				position = ftell(fp);
#ifdef IBMPC
				scr_curs(24,0);
#else
/* ansi terminal positioning */
				printf("%c[25;1H%c[7m",ESC,ESC);
#endif
				if (!isapipe)
#ifdef IBMPC
					scr_printf(
#else
					fprintf(stderr,
#endif
				"%s - %ld bytes - %d%% displayed - <ESC> = skip to next file",
					filename,size,percent(position,size) );
				else
#ifdef IBMPC
					scr_printf(
#else
					fprintf(stderr,
#endif
									"-more-");
				switch (bdos(7,0,0))	/* get a character no echo */
				{
				case ESC :
					return 0;
				case 3 :
					return -2;
				default:
					break;
				}

				lines = 1;
			}
		}
	}
	if (isaconsole)
	{
		if (lines != 1)		/* bottome of display */
		{
#ifdef IBMPC
			scr_curs(24,0);
#else
/* ansi terminal positioning */
				printf("%c[25;1H%c[7m",ESC,ESC);
#endif
			if (!isapipe)
			{
				position = ftell(fp);
#ifdef IBMPC
				scr_printf(
#else
				fprintf(stderr,
#endif
						"%s - %ld characters - %d%% displayed",
					filename,size,percent(position,size) );
			}
			else
#ifdef IBMPC
				scr_printf(
#else
				fprintf(stderr,
#endif
								"-done-");
			bdos(7,0,0); /* console input no echo */
			lines = 1;
		}
	}
}

percent(x,y)
	long int x,y;
{	/* returns integer percentage of x into y */
#ifdef FLOAT
	float xf,yf;
	xf = x; yf = y;
	x = ((xf/yf)*100);
#endif
	x *= 100;
	if (y)
		x /= y;
	else
		x = 100;
	return (x);
}

localputs(lb)
	register char *lb;
{
	int lines, pos, tabstop;
	lines = 1;
	pos = 0;
	while (*lb)
	{
		switch (*lb)
		{
		case '\t':
			tabstop = pos + (tabsize - (pos % tabsize));	
			for (;pos <= tabstop; pos++)
				(*output)(' ');
			break;
		case '\n':
			(*output)('\r');
		default:
			(*output)(*lb);
			pos++;
		}
		if (pos == 79)
		{
			pos = 1;
			(*output)('\r');
			(*output)('\n');
			++lines;
		} else if (pos > 79)
		{
			pos -= 80;
			++lines;
		}
		++lb;
	}
	return lines;
}

#ifdef IBMPC
scr_printf(fmt,args)
char *fmt; unsigned args;
{
	format(scr_putc,fmt,&args);
}
#endif
