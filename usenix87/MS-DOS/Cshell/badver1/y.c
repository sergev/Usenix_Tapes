#include <stdio.h>
#include <signal.h>
#include <setjmp.h>

/* y and t
 * y reads the standard input to standard output, then invokes cat
 * to put one or more files to standard output
 * tee copies standard input to output and puts a copy
 * into a file specified on the command line
 */

void (*signal())();
void (*teesig)();
jmp_buf y_env;
static FILE *in,*out;

FILE *fopen(),*fdopen();

void y_intr()
{
	signal(SIGINT,SIG_IGN);
	longjmp(y_env,-1);
}

y(argc,argv)
	int argc;
	char *argv[];
{
	register int c;

	/* handle interrupts */
	if (-1 == setjmp(y_env))
	{
		static char *intmsg = "Interrupted\r\n";
		write(2,intmsg,strlen(intmsg));
		fclose(in);
		fclose(out);
		signal(SIGINT,teesig);
		return -1;
	}

	/* set signal catcher */
	teesig = signal(SIGINT,y_intr);

	if (NULL == (in = fdopen(0,"r")))
	{
		fprintf(stderr,"can't open stdin\n");
	};
	if (NULL == (out = fdopen(1,"w")))
	{
		fprintf(stderr,"can't open stdout\n");
	};

	while(EOF != (c = agetc(in)))
		aputc(c,out);
	if (argc > 1)
		cat(argc,argv);
	fclose(in);
	fclose(out);
	signal(SIGINT,teesig);
	return 0;
}

jmp_buf t_env;

void t_intr()
{
	signal(SIGINT,SIG_IGN);
	longjmp(t_env,-1);
}

t(argc,argv)
	int argc;
	char *argv[];
{
	register int c;
	register FILE *tfile;
	FILE *fopen();

	/* handle interrupts */
	if (-1 == setjmp(t_env))
	{
		static char *intmsg = "Interrupted\r\n";
		write(2,intmsg,strlen(intmsg));
		fclose (tfile);
		fclose(in);
		fclose(out);
		signal(SIGINT,teesig);
		return -1;
	}

	/* set signal catcher */
	teesig = signal(SIGINT,t_intr);

	if (NULL == (tfile = fopen(*(++argv),"w")))
	{
		fprintf(stderr,"can't open %s\n",*argv);
	};
	if (NULL == (in = fdopen(0,"r")))
	{
		fprintf(stderr,"can't open stdin\n");
	};
	if (NULL == (out = fdopen(1,"w")))
	{
		fprintf(stderr,"can't open stdout\n");
	};

	while(EOF != (c = agetc(in)))
	{
		aputc(c,out);
		if (tfile)
			aputc(c,tfile);
	}
	fclose(in);
	fclose(out);
	fclose(tfile);
	signal(SIGINT,teesig);
	return 0;
}
