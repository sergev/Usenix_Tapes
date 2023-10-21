
static char *SCCS_id = "@(#) xod.c (1.2) 8/3/86 21:35:14";
/*
 * xod.c - hex/octal/decimal dump utility
 *
 * xod [-xodwv] file ...
 *	x	selects hex base
 *	o	selects octal base
 *	d	selects decimal base
 *	s	sets the beginnig offset in the file (decimal)
 *
 * Displays file(s) in the following format
 *
 * offset: ......numeric...... |...ascii...|
 *
 * where the offset and numeric sections are in the specified base
 * 'record' length is the same as the chose base (e.i., hex prints
 *	16 bytes per line, octal prints 8 bytes per line, decimal
 *	prints 10 bytes per line.
 */
#include <stdio.h>
#ifdef SYS5
#include <memory.h>
#endif

#define	TRUE	1
#define	FALSE	0

FILE	*infile;
unsigned char	inrec[16];
unsigned char	savrec[16];
unsigned	recno;
long 		offset;
int	duplicate;

int	base = 16;	/* default to hexadecimal */

main(argc, argv)
int argc;
char *argv[];
{
#ifdef BSD4.2
    char *memset(),*memcpy();
#endif
	extern int optind;
	extern char *optarg;
	int	c;

	offset = 0;

	while ((c = getopt(argc,argv,"xods:")) != EOF) {
		switch (c) {
		case 'x':
			base = 16;
			break;
		case 'o':
			base = 8;
			break;
		case 'd':
			base = 10;
			break;
		case 's':
			if(!strlen(optarg))
			    exit(1);
			offset = atol(optarg);
			break;
		default:
			fprintf(stderr,"%s: unknown option (%c)\n",
			    argv[0], c);
			fprintf(stderr,"Usage: xod [-x -o -d -snnn] fn\n");
			fprintf(stderr,"\twhere nnn is a decimal starting offset into the file\n");
			exit(1);
		}
	}
	if (optind == argc && offset) {
	    fprintf(stderr,"-s option invalid with stdin\n");
	    exit(1);
	}
	if (optind == argc) {
		infile = stdin;
		printf("\n\nDump: standard input\n\n");
		dump(stdin);
	} 
	else {
		for ( ; optind < argc; optind++) {
			if ((infile = fopen(argv[optind], "r")) == NULL) {
				fprintf(stderr,
				    "Unable to open %s\n\n", argv[optind]);
				continue;
			}
			printf("\n\nDump: %s\n\n", argv[optind]);
			if(offset)
			    fseek(infile,offset,0);
			dump(infile);
			if (duplicate) {
				printrec(savrec);
				duplicate = FALSE;
			}
			fclose(infile);
		}
	}
}



printrec(rec)
unsigned char *rec;
{
	unsigned char *i;

	switch (base) {
	case 16:
		printf("%08x: ", recno);
		break;
	case 8:
		printf("%012o: ", recno);
		break;
	case 10:
		printf("%10d: ", recno);
		break;
	}
	for (i=rec; i<&rec[base]; i++) {
		switch (base) {
		case 16:
			printf("%02x ", *i);
			break;
		case 8:
			printf(" %03o ", *i);
			break;
		case 10:
			printf("%3d ", *i);
			break;
		}
	}
	printf("| ");
	for(i=rec; i<&rec[base]; i++) {
		if (*i > 0x1f && *i < 0x7f)
			printf("%c", *i);
		else
			printf(".");
	}
	printf(" |\n");
}

dump(src)
FILE	*src;
{
	recno = offset;
	switch (base) {
	case 16:
		printf(
"Offset:    0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f   0123456789abcdef\n\n");
		break;
	case 8:
		printf(
"Offset:          0    1    2    3    4    5    6    7   01234567\n\n");
		break;
	case 10:
		printf(
"Offset:       0   1   2   3   4   5   6   7   8   9   0123456789\n\n");
		break;
	}

	while (fread(inrec, 1, base, src) != 0) {
		if (memcmp(inrec,savrec,base) == 0) {
			duplicate = TRUE;
			recno += base;
			continue;
		}
		if (duplicate) {
			printf("*\n");
			duplicate = FALSE;
		}
		printrec(inrec);
		recno += base;
		memcpy(savrec,inrec,base);
		memset(inrec,'\0',base);
	}
}
#ifdef BSD4.2
int memcmp(s1,s2,n)
char *s1,*s2;
int n;
{
    register int i,j;

    for(i = 0; i < n && !(j = (*(s1+i) - *(s2+i))); i++);
    return(j);
}
char *memcpy(s1,s2,n)
char *s1,*s2;
int n;
{
    register int i;

    for(i=0; i < n; i++)
	*(s1+i) = *(s2+i);
    return(s1);
}
char *memset(s,c,n)
    char *s;
int c,n;
{
    register int i;

    for(i=0; i < n; i++)
	*(s+i) = c;
    return(s);
}
#endif

