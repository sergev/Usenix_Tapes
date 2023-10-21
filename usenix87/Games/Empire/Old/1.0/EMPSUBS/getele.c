#include	"empdef.h"
#include	<stdio.h>

getele(toname, buf, letterfi)
int	toname;
char	buf[], *letterfi;
{
	register char	*cp;
	register	n, fh;
	char	c;

	if( letterfi == 0 ) goto X162;
	if( (fh = open(letterfi, O_RDONLY)) >= 0 ) goto X56;
	printf("Can't open \"%s\"", letterfi);
	return(2);
X56:	
	n = read(fh, buf, 512);
	n--;
	if( buf[n] == '\n' ) goto X142;
	printf("Max telegram size is %d", 511);
	return(2);
X142:	
	buf[n] = '\0';
	close(fh);
	return(n);
X162:	
	printf("Enter telegram for %s; end with ^D\n", toname);
	fflush(stdout);
	cp = buf;
X212:	
	*cp = '\04';
	n = read(0, cp, &buf[512] - cp);
	if( cp + n >= &buf[512] ) {
		printf("Too long; try that last line again...\n");
		fflush(stdout);
		if( n == &buf[512] - cp &&
		    buf[511] != '\n' ) {  /* flush input up to newline */
			do {
				if( read(0, &c, 1) != 1 ) {
					*cp = '\04';
					break;
				}
			} while( c != '\n' );
		}
	} else {
		cp += n;
	}
	if( *(cp - n) != '\04' ) goto X212;
	*cp = '\0';
	n = cp - buf;
	return(n);
}
