#include <stdio.h>
rm(argc,argv)
	int argc;
	char *argv[];
{
	extern int _echo;
	int oldecho = _echo;
	char ask = 0;

	if (argv[1][0] == '-' && toupper(argv[1][1]) == 'Q')
	{
		ask++;
		_echo = 1;
		++argv; --argc;
	}
	while(--argc) 
	{
		++argv;
		if (ask)
		{
			fprintf(stderr,"delete %s? (y or n) : ",*argv);
			if (toupper(scr_getc()) != 'Y')
			{
				write(2,"\r\n",2);
				continue;
			}
			write(2,"\r\n",2);
		}
		if (-1 == unlink(*(argv)))
		{
			perror("rm");
		}
	}
	_echo = oldecho;
}


