#include <stdio.h>
#include <signal.h>
#include <setjmp.h>

void (*signal())();

FILE *fd1,*fd2;

void (*oldsig)();
char *fgets();
jmp_buf catenv;
catintr()
{
	signal(SIGINT,SIG_IGN);			/* ignore signals */
	fclose(fd1);
	fclose(fd2);
	signal(SIGINT,oldsig);			/* restore shell interrupt */
	longjmp(catenv,-1);
}

cat(argc,argv)
	char *argv[];
{
	char *intmsg = "Interrupt received\n";

	FILE *fdopen(), *fopen();
	if (-1==setjmp(catenv))
	{
		write(2,intmsg,strlen(intmsg));
		return -1;
	}
	oldsig = signal(SIGINT,catintr);	/* trap interrupts from keyboard */
	/* get standard output opened for business */
	if (NULL == (fd2 = fdopen(1,"w")))
	{
		perror("cat : Can't open stdout");
	}

	/* handle pipes */
	if (argc == 1)
	{
		if (NULL == (fd1 = fdopen(0,"r")))
		{
			perror("cat : Can't open stdin");
		}
		_cat();
		fclose(fd1);fclose(fd2);
	}
	/* handle specified files */
	else
	{
		while(--argc)
		{
			if (NULL == (fd1 = fopen(*(++argv),"r")))
			{
				fprintf(stderr,"can't open %s\n",*argv);
				continue;
			}
			_cat();
			fclose(fd1);
		}
	}
	fclose(fd2);
	signal(SIGINT,oldsig);				/* restore old int catcher		*/
}

_cat()
{
	char buffer[512];
	while (NULL != fgets(buffer,512,fd1))
		fputs(buffer,fd2);
}
