#include	<stdio.h>
/*
 *	getnumber.c
 *
 *	get one or more numbers in sequence from a file
 *	and update the file for the next invocation
 *
 *	History:
 *		04/15/87 Initial version
 *
 *	By:	Glenn R. Sogge
 *		Systems Support Manager
 *		Datamension Corporation
 *		615 Academy Drive
 *		Northbrook, IL 60062
 *		(312) 564-5060
 *
 *	UUCP:	...!ihnp4!icom!datamen!glenn
 *
 *
*/

extern char *optarg ;
extern int optind ;

char line[BUFSIZ] ;

static char version[] = "getnumber.c version 1.00 04/15/87" ;

main(argc,argv)
int argc;
char *argv[] ;
{

	int c ;
	char *filename ;
	int count ;
	int nextnumber ;
	int init ;
	int errflag ;
	int initflag ;
	FILE *iofile ;

	errflag = 0 ;
	filename = ".COUNTER" ;	/* default file in current directory */
	count = 1 ;
	init = 1 ;
	initflag = 0 ;

	while ((c=getopt(argc,argv,"f:n:i:")) != EOF) {
		switch(c) {
			case 'f':
				filename = optarg ;
				break ;
			case 'n':
				count = atoi(optarg) ;
				break ;
			case 'i':
				init = atoi(optarg) ;
				initflag++ ;
				break ;
			case '?':
				errflag++ ;
				break ;
		}
	}
	if (errflag) {
		fprintf(stderr,"usage: %s [-f filename] [-n count] [-i initial]\n",argv[0]) ;
		exit(errflag) ;
	}

	if (count<=0) {
		fprintf(stderr,"%s: count must positive and non-zero\n",argv[0]) ;
		exit(1) ;
	}

	if (init<=0) {
		fprintf(stderr,"%s: initial must positive and non-zero\n",argv[0]) ;
		exit(1) ;
	}

	iofile = fopen(filename,"r+") ;	/* for r/w */
	if (iofile==NULL) {
		fprintf(stderr,"%s: count file <%s> does not exist, create?",argv[0],filename) ;
		c = getc(stdin) ;
		switch(c){
			case 'y':
			case 'Y':
			case '\n':
				if ((iofile=fopen(filename,"w+"))==NULL) {
					fprintf(stderr,"%s: error creating <%s>\n",argv[0],filename) ;
					exit(1) ;
				}
				fprintf(iofile,"%05d\n",init) ;	/* initialize to next available */
				fseek(iofile,0L,0) ;	/* rewind file pointer */
				break ;
			default:
				break ;
		}
	}
	/* file may exist and is open now */
	if (iofile) {
		fgets(line,BUFSIZ,iofile) ;
		nextnumber = atoi(line) ;
	} else {
		nextnumber = init ;
	}
	if (nextnumber<=0) {	/* "can't happen" */
		fprintf(stderr,"%s: invalid number in file\n",argv[0]) ;
		if (iofile)	fclose(iofile) ;
		exit(1) ;
	}

	if (initflag)	nextnumber = init ;	/* override if specified */
	/* unnecessary if file was created, but needed if it exists */
	for ( ; count ; count-- , nextnumber++ )
		printf("%5d\n",nextnumber) ;

	if (iofile) {
		fseek(iofile,0L,0) ;	/* rewind */
		fprintf(iofile,"%05d\n",nextnumber) ;
		fclose(iofile) ;
	}
	exit(0) ;
}
