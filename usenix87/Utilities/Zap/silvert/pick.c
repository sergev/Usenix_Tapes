/* pick: Kernighan & Pike p. 187 */
static char SCCSID[] = "@(#)pick.c	Ver. 1.1, 85/03/04 14:45:27";
#include <stdio.h>
char	*progname;
main(argc,argv)
	int argc;
	char *argv[];
{	int i;
	char buf[BUFSIZ];
	progname = argv[0];
	if(argc == 2 && strcmp(argv[1],"-") == 0)
		while(fgets(buf,sizeof buf,stdin) != NULL) {
			buf[strlen(buf)-1] = '\0';
			pick(buf);
		}
	else
		for(i=1;i<argc;i++)
			pick(argv[i]);
	exit(0);
}

pick(s)
	char *s;
{	fprintf(stderr,"%s? ",s);
	if(ttyin() == 'y')	/* p. 185 */
		printf("%s\n",s);
}
