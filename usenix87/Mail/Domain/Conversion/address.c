/*
 * address - run opath to see what a translated RFC822 address will come
 * out as.
 *
 * By E. Roskos 1/16/85
 */

#include <stdio.h>

char *opath();

main(argc,argv)
int argc;
char **argv;
{
char *p;
int uswitch;

	if (argc < 2)
	{
		fprintf(stderr,"usage: %s rfcaddress [...]\n",
			argv[0]);
		exit(1);
	}

	while (--argc)
	{
		p = *++argv;
		if (*p=='-')
		{
			switch(*++p)
			{
			case 'u': uswitch++;
				  continue;
			default:  printf("unknown switch: %c\n",*p);
				  continue;
			}
			continue;
		}
		printf("%s: %s\n",p,uswitch?oupath(p):opath(p));
	}

	exit(0);
}
