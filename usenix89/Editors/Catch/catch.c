
/* similar to tr -d, remove selected characters from input file(s) */
#include <stdio.h>
static char SCCSID[] = "@(#)catch.c	Ver. 1.2, 87/03/06 16:40:37";
char *progname, *filename;
int delete[64];	/* characters to delete */
int iplace = 0;

main(argc,argv)
int argc;
char *argv[];
{
	FILE *efopen(), *fp;
	int c, getopt();
	extern int optind;
	extern char *optarg;
	int value;

	progname = *argv;
	delete[0] = EOF;	/* flags end of deletion list */

	while((c = getopt(argc, argv, "hbfnrc:o:x:d:")) != EOF)
		switch(c) {
		case 'b':	/* delete backspaces */
			delete[iplace++] = '\b';
			delete[iplace] = EOF;
			break;
		case 'f':	/* delete formfeeds */
			delete[iplace++] = '\f';
			delete[iplace] = EOF;
			break;
		case 'n':	/* delete newlines */
			delete[iplace++] = '\n';
			delete[iplace] = EOF;
			break;
		case 'r':	/* delete carriage returns */
			delete[iplace++] = '\r';
			delete[iplace] = EOF;
			break;
		case 'c':	/* delete all characters in following string */
			for(c=0; optarg[c]; c++)
				delete[iplace++] = optarg[c];
			delete[iplace] = EOF;
			break;
		case 'o':	/* delete by octal code */
			if(sscanf(optarg, "%o", &value) != 1)
				errors("cannot find octal value in %s",
					optarg);
			delete[iplace++] = value;
			delete[iplace] = EOF;
			break;
		case 'x':	/* delete by hex code */
			if(sscanf(optarg, "%x", &value) != 1)
				errors("cannot find hex value in %s",
					optarg);
			delete[iplace++] = value;
			delete[iplace] = EOF;
			break;
		case 'd':	/* delete by decimal code */
			if(sscanf(optarg, "%u", &value) != 1)
				errors("cannot find decimal value in %s",
					optarg);
			delete[iplace++] = value;
			delete[iplace] = EOF;
			break;
		case 'h':
		default:
			help(c);
			exit(1);
		}
	switch(argc - optind) {
	case 0:
		filename = "";
		copy(stdin);
		break;
	default:
		for(; optind<argc; optind++) {
			filename = argv[optind];
			fp = efopen(filename, "r");
			copy(fp);
			efclose(fp);
		}
	}
	exit(0);
}

help(c)
char c;
{
	fprintf(stderr, "%s: Options are:\n", progname);
	fprintf(stderr, "\t-b\tdelete backspaces\n");
	fprintf(stderr, "\t-f\tdelete formfeeds\n");
	fprintf(stderr, "\t-n\tdelete newlines\n");
	fprintf(stderr, "\t-r\tdelete returns\n");
	fprintf(stderr, "\t-cCCC\tdelete character[s] CCC\n");
	fprintf(stderr, "\t-dNNN\tdelete decimal NNN\n");
	fprintf(stderr, "\t-bNNN\tdelete octal NNN\n");
	fprintf(stderr, "\t-oNNN\tdelete octal NNN\n");
	fprintf(stderr, "\t-xNN\tdelete hex NN\n");
}

copy(fp)
FILE *fp;
{
	int c;
	while ((c=getc(fp)) != EOF ) {
		for(iplace=0; delete[iplace] != EOF; iplace++)
			if(c == delete[iplace])
				break;
		if(delete[iplace] == EOF)
			putchar(c);	/* only if we reach the end */
	}
}
