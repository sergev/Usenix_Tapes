/* Copyright (c) 1985 by Thomas S. Fisher - Westminster, CO 80030 */

#include	<stdio.h>
#include	"emp.h"

FILE	*fopen(), *fdnew, *fdold;
struct	sector	s;
extern	int	optind;
extern	char	*optarg;

main(argc, argv)
int	argc;
char	**argv;
{
	int	c, errflg;
	int	capx, capy, capflg;

	capx = capy = capflg = 0;
	while( (c = getopt(argc, argv, "x:y:")) != EOF ) {
		switch( c ) {
		case 'x':
			capx = atoi(optarg);
			capflg++;
			break;
		case 'y':
			capy = atoi(optarg);
			capflg++;
			break;
		case '?':
			errflg++;
			break;
		}
	}
	if( errflg ) {
		printf("Usage: %s [-x new_capx] [-y new_capy]\n", argv[0]);
		exit(1);
	}
	if( capflg ) {
		movwrld(capx, capy);
	} else {
		newwrld();
	}
}

newwrld()
{
	int	x, y;

	s.s_des = ' ';
	if( (fdnew = fopen("newsect", "w")) == NULL ) {
		fprintf(stderr, "Can't create newsect\n");
		exit(1);
	}
	for( y = 0; y < XYMAX; y++ ) {
		for( x = 0; x < XYMAX; x++ ) {
			if( ptsec(fdnew, &s, sctno(x,y)) == ERROR ) {
				fprintf(stderr, "ptsec: ERROR\n");
				exit(3);
			}
		}
	}
}

movwrld(capx, capy)
int	capx, capy;
{
	int	x, y;

	if( (fdold = fopen("empsect", "r")) == NULL ) {
		fprintf(stderr, "Can't access empsect\n");
		exit(1);
	}
	if( (fdnew = fopen("newsect", "w")) == NULL ) {
		fprintf(stderr, "Can't create newsect\n");
		exit(2);
	}
	for( y = 0; y < XYMAX; y++ ) {
		for( x = 0; x < XYMAX; x++ ) {
			gtsec(fdold,  &s, sctno(x, y));
			ptsec(fdnew,  &s, sctno(x-capx, y-capy));
		}
	}
}

sctno(x, y)
int	x, y;
{
	return((x + XYMAX) % XYMAX + ((y+XYMAX) % XYMAX) * XYMAX );
}

ptsec(fd, buf, n)
FILE	*fd;
struct	sector	*buf;
int	n;
{
	if( fseek(fd, ((long)n)*sizeof(struct sector), 0) != 0 ) return(ERROR);
	if( fwrite(buf, sizeof(struct sector), 1, fd) != 1 ) return(ERROR);
}
gtsec(fd, buf, n)
FILE	*fd;
struct	sector	*buf;
int	n;
{
	if( fseek(fd, ((long)n)*sizeof(struct sector), 0) != 0 ) return(ERROR);
	if( fread(buf, sizeof(struct sector), 1, fd) != 1 ) return(ERROR);
}
