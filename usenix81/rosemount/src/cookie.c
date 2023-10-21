/*
 *  Routine to print a random 'cookie'.
 */

char cfile[]	{ "/usr/lib/cookie" } ;
char cindex[]	{ "/usr/lib/cookie.index" } ;
int nindex ;
	long seekadr ;
int cfd, ifd ;

char gtchar() ;


main()
{
	int i ;
	char c ;

	if ((cfd = open(cfile,0)) < 0) {
		printf("Can't break cookie.\n") ;
		exit() ;
		}
	if ((ifd = open(cindex,0)) < 0) {
		printf("Baking cookies.\n") ;
		if ((ifd = creat(cindex,0644)) < 0) {
			printf("Can't create index.\n") ;
			abort() ;
			}

		seekadr = 0 ;
		nindex = 0 ;
		lseek(ifd,2L,0) ;

		while (1) {
			do {
				if ((c = gtchar()) == '\0') goto done ;
				} while (c!='~') ;
			write(ifd,&seekadr,4) ;
			nindex++ ;
			do {
				if ((c = gtchar()) == '\0') goto done ;
				} while (c!='~') ;
			}

		done:
		lseek(ifd,0L,0) ;
		write(ifd,&nindex,2) ;
		close(ifd) ;
		exit() ;
		}

	randomize() ;
	read(ifd,&nindex,2) ;
	i = (urand()%nindex)*4 ;
	lseek(ifd,(long) i,1) ;
	read(ifd,&seekadr,4) ;

	lseek(cfd,seekadr,0) ;
	while ((c = gtchar()) != '\0' &&
		c != '~') putchar(c) ;
	putchar('\n') ;

	exit() ;
	} /* main */





char gtchar()
{
	char c ;

	if (read(cfd,&c,1) < 1) return ('\0') ;
	else {
		seekadr++ ;
		return(c) ;
		}

	} /* gtchar */
