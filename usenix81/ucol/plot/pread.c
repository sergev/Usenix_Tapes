/*	pread is the guts of the original cplot routine.  I have made this
	a c callable routine which is called by all of the c plot drivers.
	This way we need to change only one routine when we introduce
	new plot commands for a particular device.		*/

struct plact {
	int a;	/* symbol */
	int act;	/* action */
	int n;	/* arg count */
}
z[] = {
	'm',	1,	2,
	'n',	1,	2,
	'p',	1,	2,
	'l',	1,	4,
	't',	2,	1,
	'a',	1,	6,
	'c',	1,	3,
	'e',	4,	0,
	'f',	3,	1,
	's',	1,	4,
	'b',	5,	10,	/* for hp only - set graph limits  */
	'o',	1,	1,	/* for hp only - set penspeed */
	'g',	5,	8,	/* for hp only - scale map */
	'h',	1,	1,	/* for hp only - select new pen */
	'i',	1,	1,	/* select new font */
	'j',	5,	6,	/* set character size and slant */
	'k',	5,	2,	/* set direction of labelling */
	'q',	1,	1,	/* define reference point for string labeling */
	'd',	6,	1,	/* unknown UNIX plot command */
	'\0',	0,	0 };

char *style[] = {
	"dotted",
	"solid",
	"longdashed",
	"shortdashed",
	"dotdashed",
	"" };

int pread(x, xfp, s, sl, l, fin)
int *x;
float *xfp;
long l;
char *s;
long *sl;
int fin;
{
	struct plact *t;
	int i, j, f;
	char c, *p;
	char b[30];

	if( read(fin, &c, 1) != 1 ) {
		return(-1);
	}
	i = c;
	f = 1;
	*sl = 0l;

	for( t=z; t->a != '\0'; f++, t++)
		if( t->a == i ) switch( t->act ) {
		case 1:
			j = read(fin, (char *)x, 2*t->n);
			return(f);
		case 2:
			for( j=0; read(fin, s, 1), *s != '\n'; s++ ) {
				if( ++j >= l ) return( -2 );
			}
			*sl = (long) j;
			return( f );
		case 3:
			for( j=0, p=b; read(fin,p,1), *p != '\n'; p++ ) {
				if( ++j >= 30 ) return( -3 );
			}
			*p = '\0';
			switch (b[3]) {
				case 't':
					*x = 1;
					break;
				case 'i':
					*x = 2;
					break;
				case 'g':
					*x = 3;
					break;
				case 'r':
					*x = 4;
					break;
				case 'd':
					*x = 5;
					break;
				default:
					*x = 6;
					break;
			}
			return(f);
		case 4:
			return(8);
		case 5:
			j = read(fin, (char *)xfp, 2*t->n);
			return(f);
		case 6:
			j = read(fin, x, 8);
			i = *(x+6);
			j = read(fin, x+8, i);
			return(f);
		default:
			return( -5 );
		}
	return( -6 );
}
