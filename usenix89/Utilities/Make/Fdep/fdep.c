
#include <stdio.h>
#include <ctype.h>
#include <string.h>
static char SCCSID[] = "@(#)fdep.c	Ver. 2.1, 86/06/06 15:20:35";
char *progname, *filename;
char line[80];

main(argc,argv)
int argc;
char **argv;
{
	FILE *fp, *efopen();

	progname = *argv;

	if(argc == 1) {
		filename = "";
		process(stdin);
	}
	else
		while(--argc) {
			filename = *++argv;
			fp = efopen(filename,"r");
			process(fp);
			efclose(fp);
		}
	exit(0);
}

process(fp)
FILE *fp;
{
	char *endline;
	while(fgets(line,80,fp) != NULL) {
		for(endline=line; isspace(*endline); endline++) ; /* skip */
		if( !strncmp("include", endline, 7) ) /* include begins line */
			expand(endline+7);
	}
}
expand(string)	/* expand includes */
char *string;
{
	char *a, *z;
	if((a = strchr(string, '\'')) == (z = strrchr(string, '\'')))
		errors("bad format: %s", string);
	else {
		printf("%s < ", filename);
		while( ++a<z )
				putchar(*a);
		putchar('\n');
	}
}
