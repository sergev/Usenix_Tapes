339a
   size:
	if (compar (s, "*1") || compar (s, "*2") ||
	    compar (s, "*4") || compar (s, "*8"))
		s =+ 2;
	goto loop;
.
330c
	else if( compar(s,"complex") ){ s =+ 7; goto size; }
.
325,327c
	else if( compar(s,"real") ){ s =+ 4; goto size; }
	else if( compar(s,"integer") ){ s =+ 7; goto size; }
	else if( compar(s,"logical") ){ s =+ 7; goto size; }
.
106c
		fout = creat("ratjunk", 0600);
.
19c
char	*ratfor "/usr/bin/ratfor";
.
16,17c
int	fflag	0;
int	cflag	0;
.
13c
int	rflag	0;
.
9c
char	*llist[100];
.
6,7c
char	*av[100];
char	*rlist[100];
.
1c
/*
 * rc.c - Ratfor/Fortran command processor
 *
 *	modified 27-Apr-1980 by D A Gwyn:
 *	1) /usr/bin/ratfor, not /usr/lib/ratfor;
 *	2) tables extended;
 *	3) ratjunk created in mode 0600;
 *	4) declaration bytecount spec now supported.
 */
.
w
q
