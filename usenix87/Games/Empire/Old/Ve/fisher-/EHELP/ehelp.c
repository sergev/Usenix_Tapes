/* Copyright (c) 1985 by Thomas S. Fisher - Westminster, CO 80030 */

/*
	There are a variety of fflush and sleep calls in here to fix(?) 
	some of the problems with terminals running at 9600 baud on an
	Amdahl UTS system.
*/

#include	<stdio.h>
#include	<ctype.h>
#include	<curses.h>
#include	"emp.h"
#include	"ship.h"

#define	MAXLINE	132	/* Max command line (at bottom of screen) length */
#define	MAXPATH	64	/* Max path for nav and mov */

/* Miscellaneous constants */

#define	OK	1
#define	RESET	0
#define	TRUE	1
#define	FALSE	0

/* Special characters while cursor is on the map */

#define	DELETE	077
#define	CR	015
#define	LF	012
#define	SPACE	' '
#define	PLUS	'+'
#define	MINUS	'-'
#define	TAB	011

/*	Screen motion characters	*/

#define	UP	'e'
#define	UPRIGHT	'r'
#define	RIGHT	'd'
#define	DNRIGHT	'c'
#define	DOWN	'x'
#define	DNLEFT	'z'
#define	LEFT	's'
#define	UPLEFT	'w'
#define VILEFT	'h'
#define	VIDN	'j'
#define	VIUP	'k'
#define	VIRIGHT	'l'

#define	CENSW	15	/* Width of census display area */

/* Valid command codes */

#define	CEN	1
#define	DES	2
#define	MOV	3
#define	DEL	4
#define	MAP	5
#define	SHP	6
#define	NAV	7
#define	NCMDS	7

FILE	*fopen(), *fdsec, *fdscrpt, *fdship;
struct	sector	s;
struct	shpstr	Ship;
int	Func = CEN;
char	Stype[] = { 'p', 'm', 'd', 's', 'f', 't', 'b', 'c' };
char	*Sorder = "cbdtspmf";
char	Shiplist[128];
int	Shpnum = 0;
char    *Fchar[] = { " ", "cen", "des", "mov", "del", "map", "shi", "nav" };
char	*Deldir[] = { ".",  "$",  "",   "",
		      "",   "",   "",   "",
		      "n",  "ne", "e",  "se",
		      "s",  "sw", "w",  "nw" };
char	*Movdir[] = {	"\\l",	"l",	"/l",	"u",
		"e",	"d",	"/r",	"r",	"\\r" };


int	Rtxlat[] = { 15, 14, 13, 8, 0, 12, 9, 10, 11 };
char	Rtdirl[] = { ' ', '$', ' ', ' ', ' ', ' ', ' ', ' ',
		     '^', ' ', ' ', ' ', 'v', '/', '<', '\\' };
char	Rtdirr[] = { ' ', '$', ' ', ' ', ' ', ' ', ' ', ' ',
		     '^', '/', '>', '\\', 'v', ' ', ' ', ' ' };
char	Desig = 'g';
int	Thresh = 0;
int	Rsrc = ORE;
int	Quant = 0;
char	Rchar[] = { 'c', 'm', 's', 'g', 'p', 'o', 'b' };
int	Pathl = 0;
int	Pathx[MAXPATH], Pathy[MAXPATH];
int	Xcen = 0, Ycen = 0;
int	Xcur = 0, Ycur = 0;
int	Xl, Yl, Xh, Yh, Xlast, Ylast;
char	linebuf[MAXLINE];

int	Censtrt;
int	Foe;

main() {
	char	*prompt();
	char	funcstr[4], *getenv(), *foestr;
	int	i;

	if( (fdsec = fopen("empsect", "r")) == NULL ) {
		fprintf(stderr, "Can't open empsect\n");
		exit(1);
	}
	if( (fdscrpt = fopen(".yout", "a+")) == NULL ) {
		fprintf(stderr, "Can't open output file\n");
		exit(2);
	}
	fdship = fopen("empship", "r");
	if( (foestr = getenv("FOES")) == NULL ) {
		Foe = 0;
	} else {
		Foe = atoi(foestr);
	}
	initscr();
	Censtrt = COLS - CENSW;
	clear();
	prfunc();
	prcoord();
	strcpy(Shiplist,",");
	while( prompt(">") != NULL ) {
		if( linebuf[0] == '>' ) { 
			fputs(&linebuf[1], fdscrpt);
			continue;
		}
		if( sscanf(linebuf, "%d,%d%*[^\n]\n", &Xcen, &Ycen) == 2 ) {
			prcoord();
			Xcur = Xcen;
			Ycur = Ycen;
			continue;
		}
		if( sscanf(linebuf, "%3s%*[^\n]\n", funcstr) != 1 ) continue;
		for( i = 1; i <= NCMDS; i++ ) {
			if( strncmp(funcstr, Fchar[i], 3) != 0 ) continue; 
			Func = i;
			prfunc();
			refresh();
			switch( Func ) {
			case CEN:
				docen();
				break;
			case DES:
				dodes();
				break;
			case MOV:
				domov();
				break;
			case DEL:
				dodel();
				break;
			case MAP:
				prmap();
				break;
			case SHP:
				doship();
				break;
			case NAV:
				donav();
				break;
			}
			break;
		}
		refresh();
	}
	endwin();
	exit(0);
}

char	*
prompt(s)
char	*s;
{
	static	int	plength, ilength;
	int	i, blanks;
	char	*ret;

	blanks = plength + ilength;
	for( i = 0; i < blanks; i++ ) {
		linebuf[i] = ' ';
	}
	linebuf[blanks] = '\0';
	mvaddstr(LINES-1, 1, linebuf);
	mvaddstr(LINES-1, 1, s);
	refresh();
	plength = strlen(s);
#ifdef DONTDEF
	ret = fgets(linebuf, MAXLINE, stdin);
	ilength = strlen(linebuf);
	return(ret);
#else ~DONTDEF
	getstr(linebuf);
	ilength = strlen(linebuf);
	return(ilength? linebuf: (char *)0);
#endif DONTDEF
}

docen()
{
	char	c, onmap();

	while( prompt("cen>") != NULL ) {
		if( linebuf[0] == '.' ) break;
		if( linebuf[0] != '\n' ) continue;
		while( (c = onmap()) != '\n' ) {
			if( c == SPACE ) {
				fprintf(fdscrpt, "cen %d,%d\n", Xcur, Ycur);
			}
			if( c == TAB ) prfcen();
		}
	}
}

dodes()
{
	char	c, desstr[4], onmap();

	prdes();
	while( prompt("des>") != NULL ) {
		if( linebuf[0] == '.' ) break;
		if( sscanf(linebuf, "%1[abcdfghimrtuwx#*!)+-]%*[^\n]\n",
			desstr) == 1 ) {
			if( desstr[0] != '\0' ) {
				Desig = desstr[0];
				prdes();
				linebuf[0] = '\n';
			}
		}
		if( linebuf[0] != '\n' ) continue;
		while( (c = onmap()) != '\n' ) {
			if( c == SPACE ) {
				fprintf(fdscrpt, "des %d,%d %c\n",
					Xcur, Ycur, Desig);
				addch( Desig );
			}
			if( c == TAB ) prfcen();
		}
	}
}

domov()
{
	int	 ret;
	char	c, onmap();

	Pathl = 0;
	prrsrc();
	prquant();
	while( prompt("mov>") != NULL ) {
		if( linebuf[0] == '.' ) break;
		if( sscanf(linebuf, "%d%*[^\n]\n", &Quant) == 1 ) {
			prquant();
			linebuf[0] = '\n';
		}
		if( (ret = getrsrc()) != ERROR ) {
			Rsrc = ret;
			prrsrc();
			linebuf[0] = '\n';
		}
		if( linebuf[0] != '\n' ) continue;
		while( (c = onmap()) != '\n' ) {
			switch( c ) {
			case TAB:
				prfcen();
				break;
			case PLUS:
				addsec();
				break;
			case MINUS:
				delsec();
				getsec(&s, secno(Xcur, Ycur));
				break;
			case SPACE:
				if( Pathl == 0 && Quant > 0 ) break;
				if( addsec() != OK ) break;
				fprintf(fdscrpt, "mov %c %d,%d %d ",
					Rchar[Rsrc], Pathx[0], Pathy[0], Quant);
				move((Pathy[0]-Yl+1), (Pathx[0]-Xl)*3+2);
				if( Quant == 0 ) {
					fprintf(fdscrpt, "\n");
					delsec();
					break;
				}
				moveit();
				fprintf(fdscrpt, "v\ne\n");
				break;
			}
		}
	}
}

dodel()
{
	int	ret, index, ix, iy;
	char	c, onmap();

	prrsrc();
	prthresh();
	while( prompt("del>") != NULL ) {
		if( linebuf[0] == '.' ) break;
		if( sscanf(linebuf, "%d%*[^\n]\n", &Thresh) == 1 ) {
			prthresh();
			linebuf[0] = '\n';
		}
		if( (ret = getrsrc()) != ERROR ) {
			Rsrc = ret;
			prmap();
			prrsrc();
			prthresh();
			linebuf[0] = '\n';
		}
		if( linebuf[0] != '\n' ) continue;
		while( (c = onmap()) != '\n' ) {
			switch( c ) {
			case TAB:
				prfcen();
				break;
			case PLUS:
				addsec();
				break;
			case SPACE:
				if( adjacent(Xcur, Ycur, Xlast, Ylast) != OK ) break;
				fprintf(fdscrpt, "del %c %d,%d\n%d,%d (%d)\n",
					Rchar[Rsrc], Xlast, Ylast, Xcur, Ycur, Thresh);
				ix = Xcur - Xlast;
				if( ix < -1 ) ix += XYMAX;
				if( ix >  1 ) ix -= XYMAX;
				iy = Ycur - Ylast;
				if( iy < -1 ) iy += XYMAX;
				if( iy >  1 ) iy -= XYMAX;
				index = (ix + 1)*3 + (iy + 1);
				mvaddch(Ylast - Yl + 1, (Xlast - Xl)*3 + 1, Rtdirl[Rtxlat[index]]);
				mvaddch(Ylast - Yl + 1, (Xlast - Xl)*3 + 3, Rtdirr[Rtxlat[index]]);
				addsec();
				break;
			}
		}
	}
}

getrsrc()
{
	int	i;
	char	rstr[4];

	if( sscanf(linebuf, "%1[cmsgpob]%*[^\n]\n", rstr) != 1 ) {
		return(ERROR);
	}
	if( rstr[0] == '\0' ) return(ERROR);
	for( i = 0; i <= BAR; i++ ) {
		if( rstr[0] == Rchar[i] ) return(i);
	}
	return(ERROR);
}

doship()
{
	int	x, y, row, col, i;
	int	shipok;
	char	c, sorder[TMAXNO+1];

	if( fdship == NULL ) return;
	prorder();
	prshpnum();
	while( prompt("ship>") != NULL ) {
		shipok = ERROR;
		for( i = 0; i <= TMAXNO; i++ ) {
			sorder[i] = ' ';
		}
		if( linebuf[0] == '.' ) break;
		if( sscanf(linebuf, "%d%*[^\n]\n", &Shpnum) == 1 ) {
			prshpnum();
			shipok = OK;
		}
		sscanf(linebuf, "%8[pmdsftbcPMDSFTBC]%*[^\n]\n", sorder);
		for( i = 0; i <= TMAXNO; i++ ) {
			if( sorder[i] == '\0' || sorder[i] == ' ' ) break;
			Sorder[i] = tolower(sorder[i]);
		}
		if( sorder[0] != '\0' && sorder[0] != ' ' ) {
			Sorder[i] = '\0';
			prorder();
			shipok = OK;
		}
		if( linebuf[0] == '*' && linebuf[1] == '\n' ) {
			strcpy(Sorder, "cbdtspmf");
			prorder();
			shipok = OK;
		}
		if( linebuf[0] == '?' && linebuf[1] == '\n' ) {
			rewind(fdship);
			while( fread(&Ship, sizeof(struct shpstr), 1, fdship) == 1 ) {
				if( Ship.shp_effc == 0 ) continue;
				if( Foe != 0 && Ship.shp_own != 0 &&
					Ship.shp_own != Foe ) continue;
				for( i = 0; i <= TMAXNO && Sorder[i]!='\0'; i++ ) {
					if( Sorder[i] == Stype[Ship.shp_type])  break;
				}
				if( Sorder[i] != Stype[Ship.shp_type]) continue;
				x = Ship.shp_xp;
				if( (Xcen - x) >= XYMAX/2 ) x += XYMAX;
				if( (x - Xcen) >= XYMAX/2 ) x -= XYMAX;
				if( x < Xl || x > Xh ) continue;
				y = Ship.shp_yp;
				if( (Ycen - y) >= XYMAX/2 ) y += XYMAX;
				if( (y - Ycen) >= XYMAX/2 ) y -= XYMAX;
				if( y < Yl || y > Yh ) continue;
				row = y - Yl + 1;
				col = (x - Xl)*3 + 1;
				if( Ship.shp_own == 0 ) col += 2;
				mvaddch(row, col, toupper(Stype[Ship.shp_type]));
			}
			shipok = OK;
		}
		if( shipok == OK ) linebuf[0] = '\n';
		if( linebuf[0] != '\n' ) continue;
		while( (c = onmap()) != '\n' ) {
			switch( c ) {
			case '\t':
				prship();
				break;
			case 'i':
				prshplst();
				break;
			case 'I':
				prshpsum();
				break;
			}
		}
	}
}

donav()
{
	int	shipok;
	char	c, onmap();

	Pathl = 0;
	prorder();
	prshpnum();
	while( prompt("nav>") != NULL ) {
		shipok = ERROR;
		if( linebuf[0] == '.' ) break;
		if( linebuf[0] == '\n' )	;
		else if( linebuf[0] == ',' ) {
			strcpy(Shiplist,",");
			shipok = OK;
		}
		else if( isalpha(linebuf[0]) ) {
			Shiplist[0] = linebuf[0];
			Shiplist[1] = '\0';
			shipok = OK;
		}
		else if( sscanf(linebuf,"%120[0123456789/]",Shiplist) == 1 ) {
			if( Shiplist[0] != '\0' &&
				sscanf(Shiplist,"%d",&Shpnum) == 1 ) shipok=OK;
		}
		if( shipok == OK ) {
			linebuf[0] = '\n';
			prnav();
		}
		if( linebuf[0] != '\n' ) continue;
		while( (c = onmap()) != '\n' ) {
			switch(c) {
			case TAB:
				prship();
				break;
			case 'i':
				prshplst();
				break;
			case 'I':
				prshpsum();
				break;
			case PLUS:
				addsec();
				break;
			case MINUS:
				delsec();
				break;
			case SPACE:
				if( Pathl == 0 ) break;
				if( addsec() != OK ) break;
				if( strchr(Shiplist, ',') != NULL ) {
					sprintf(Shiplist, "%d,%d",
						Pathx[0], Pathy[0]);
				}
				fprintf(fdscrpt,"nav %s ", Shiplist);
				moveit();
				fprintf(fdscrpt, "\ne\n");
				break;
			}
		}
	}
}

moveit()
{
	int	i, ix, iy, row, col;

	row = Pathy[0] - Yl + 1;
	col = (Pathx[0] - Xl)*3 + 2;
	mvaddch(row, col, '@');
	move(row, col);
	for( i = 1; i < Pathl; i++ ) {
		ix = Pathx[i] - Pathx[i-1];
		if( ix < -1 ) ix += XYMAX;
		if( ix >  1 ) ix -= XYMAX;
		iy = Pathy[i] - Pathy[i-1];
		if( iy < -1 ) iy += XYMAX;
		if( iy >  1 ) iy -= XYMAX;
		fprintf(fdscrpt, "%s", Movdir[(ix+1)*3 + (iy+1)]);
		row = Pathy[i] - Yl + 1;
		col = (Pathx[i] - Xl)*3 + 2;
		mvaddch(row, col, '@');
		move(row, col);
	}
}

prfunc()
{
	mvprintw(1, Censtrt, "Func: %-3s", Fchar[Func]);
}

prcoord()
{
	mvprintw(2, Censtrt, "Center: %3d,%-3d", Xcen, Ycen);
}

prdes()
{
	mvprintw(3, Censtrt, "Desig: %c      ", Desig);
}

prrsrc()
{
	mvprintw(3, Censtrt, "Resource: %c   ", Rchar[Rsrc]);
}

prthresh()
{
	mvprintw(4, Censtrt, "Thresh: %-3d", Thresh);
}

prquant()
{
	mvprintw(4, Censtrt, "Quant: %-4d", Quant);
}

prorder()
{
	mvprintw(3, Censtrt, "Order %-8s", Sorder);
}

prshpnum()
{
	mvprintw(4, Censtrt, "Ship: %-5d", Shpnum);
}

prnav()
{
	char	*p, out[8];
	int	i;

	move(4, Censtrt);
	p = Shiplist;
	for( i = 0; i < 7; i++ ) {
		if( *p == '\0' || *p == '/' ) break;
		out[i] = *p++;
	}
	out[i] = '\0';
	printw("Ship: %-5s", out);
}

prship()
{
	short	mbl;

	if( getship(&Ship, Shpnum) == ERROR ) return;
	mvaddstr(5, Censtrt, "               ");
	mvprintw(6, Censtrt, "%c%5d,%-3d %3d ",
		toupper(Stype[ Ship.shp_type ]),
		Ship.shp_xp,	Ship.shp_yp,
		Ship.shp_own);
	move(7, Censtrt);
/* kluge to handle unsigned characters (so ship.h doesn't have to change) */
	if( (mbl = Ship.shp_mbl) > 127 ) mbl -= 256;
	printw("%3d%%  %c    %3d ",
		Ship.shp_effc,
		Ship.shp_fleet,
		mbl);
	move(8, Censtrt);
	if( Ship.shp_type == S_CAR ) {
		printw("%3d %3d %2d %3d ", 
			Ship.shp_crew,	Ship.shp_shels,
			Ship.shp_gun,	Ship.shp_plns);
	} else {
		printw("%3d %3d %2d      ", 
			Ship.shp_crew,	Ship.shp_shels,
			Ship.shp_gun);

	}
}

prshpsum()
{
	int	num, frlist[TMAXNO+1], foelist[TMAXNO+1];
	int	i;

	rewind( fdship );
	num = 0;
	for( i = 0; i <= TMAXNO; i++ ) {
		frlist[i] = foelist[i] = 0;
	}
	while( getship(&Ship, num++) != ERROR ) {
		if( Ship.shp_effc == 0 ) continue;
		if( (Ship.shp_xp - Xcur)%XYMAX != 0 ) continue;
		if( (Ship.shp_yp - Ycur)%XYMAX != 0 ) continue;
		if( Foe != 0 && Ship.shp_own != 0 &&
			Foe != Ship.shp_own ) continue;
		if( Ship.shp_own == 0 ) {
			frlist[Ship.shp_type]++;
		} else {
			foelist[Ship.shp_type]++;
		}
	}
	mvprintw(5, Censtrt, "%2.0dc %2.0db %2.0dd %2.0dt",
		frlist[S_CAR], frlist[S_BAT], frlist[S_DES], frlist[S_TEN]);
	mvprintw(6, Censtrt, "%2.0ds %2.0dp %2.0dm %2.0df",
                frlist[S_SUB], frlist[S_PT ], frlist[S_MIN], frlist[S_FRE]);
	mvprintw(7, Censtrt, "%2.0dc %2.0db %2.0dd %2.0dt",
                foelist[S_CAR], foelist[S_BAT], foelist[S_DES], foelist[S_TEN]);
	mvprintw(8, Censtrt, "%2.0ds %2.0dp %2.0dm %2.0df",
                foelist[S_SUB], foelist[S_PT ], foelist[S_MIN], foelist[S_FRE]);
}

prshplst()
{
	int	num, field, frfld, foefld;
	int	i;

	rewind(fdship);
	num = 0;
	frfld = 0;
	foefld = 21;
	while( getship(&Ship, num++) != ERROR ) {
		if( Ship.shp_effc == 0 ) continue;
		if( (Ship.shp_xp - Xcur)%XYMAX != 0 ) continue;
		if( (Ship.shp_yp - Ycur)%XYMAX != 0 ) continue;
		if( Foe != 0 && Ship.shp_own != 0 &&
			Foe != Ship.shp_own ) continue;
		for( i=0; i<=TMAXNO && Sorder[i]!='\0'; i++ ) {
			if( Sorder[i] == Stype[Ship.shp_type])  break;
		}
		if( Sorder[i] != Stype[Ship.shp_type])  continue;
		if( Ship.shp_own == 0 ) {
			field = (frfld > 20) ? 20 : frfld++;
		} else {
			field = (foefld > 41) ? 41 : foefld++;
		}
		mvprintw(10 + field/3, Censtrt + (field%3)*5, "%c%-4d",
			Stype[Ship.shp_type], num-1); 
	}
	for( ; frfld <= 20; frfld++ ) {
		mvaddstr(10 + frfld/3, Censtrt + (frfld%3)*5, "     ");
	}
	for( ; foefld <= 41; foefld++ ) {
		mvaddstr(10 + foefld/3, Censtrt + (foefld%3)*5, "     ");
	}
}

addsec()
{
	int	i;

	if( Func == MOV || Func == NAV ) {
		i = (Pathl == 0) ? 0 : Pathl - 1;
		if( Pathl != 0 &&
		    adjacent(Xcur,Ycur,Pathx[i],Pathy[i]) != OK ) {
			return(ERROR);
		}
		if( Pathl == 0 || Xcur != Pathx[i] || Ycur != Pathy[i] ) {
			Pathx[Pathl] = Xcur;
			Pathy[Pathl] = Ycur;
			Pathl++;
			addch('@');
		}
	}
	Xlast = Xcur;
	Ylast = Ycur;
	return(OK);
}

delsec()
{
	if( Func == MOV || Func == NAV ) {
		getsec(&s, secno(Xcur, Ycur));
		addch(s.s_des);
		if( Pathl <= 0 ) return;
		Pathl--;
		if( Pathl <= 0 ) return;
		Xcur = Pathx[Pathl-1];
		Ycur = Pathy[Pathl-1];
	}
}

adjacent(x, y, xl, yl)
int	x, y, xl, yl;
{
	int	xdiff, ydiff;

	xdiff = x - xl;
	if( xdiff < 0 ) xdiff = -xdiff;
	if( xdiff == (XYMAX-1) ) xdiff = 1;
	ydiff = y - yl;
	if( ydiff < 0 ) ydiff = -ydiff;
	if( ydiff == (XYMAX-1) ) ydiff = 1;
	if( (xdiff == 0 || xdiff == 1) &&
	    (ydiff == 0 || ydiff == 1) ) return(OK);
	return(ERROR);
}

prmap()
{
	int	height, width, xm, ym, row, col, nosect;

	clear();
	prfunc();
	prcoord();
	refresh();
	sleep(1); /* to make some terminals work at 9600 baud */
	height = LINES - 3;
	width = COLS - CENSW - 4;
	row = 1;
	Yl = Ycen - height/2;
	Yh = Ycen + height/2;
	Xl = Xcen - width/6;
	Xh = Xcen + width/6;
	for( ym = Yl; ym <= Yh; ym++ ) {
		col = 1;
		nosect = TRUE;
		for( xm = Xl; xm <= Xh; xm++ ) { 
			getsec(&s, secno(xm, ym));
			if( s.s_des != '\0' && s.s_des != ' ' ) {
				if( nosect == TRUE ) {
					move(row, col);
					nosect = FALSE;
				}
				if( s.s_coun != 0 && s.s_coun != 99 ) {
					if( s.s_des != '?' ) {
						if(Foe==0 || s.s_coun==98) {
							addch('?');
						} else {
							printw("%1d",
								s.s_coun%10);
						}
					} else {
						addch(' ');
					}
					addch(s.s_des);
					addch(' ');
				} else if( s.s_coun == 99 ) {
					addch('~');
					addch(s.s_des);
					addch(' ');
				} else if( s.s_coun == 0 ) {
					if( Func == DEL ) {
						addch(Rtdirl[(int)(s.s_del[Rsrc]&017)]);
					} else {
						addch(' ');
					}
					if( s.s_des == '.' && s.s_rsrc[SHL] != 0 ) {
						addch(':');
					} else {
						addch(s.s_des);
					}
					if( Func == DEL ) {
						addch(Rtdirr[(int)(s.s_del[Rsrc]&017)]);
					} else {
						addch(' ');
					}
				}
			} else {
				nosect = TRUE;
			}
			col += 3;
		}
	row++;
	}
	move((Ycen - Yl), (Xcen - Xl)*3);
}

prcen()
{
	int	xcur, ycur;

	xcur = Xcur;  ycur = Ycur;
	if( Xcur < -XYMAX/2 ) xcur = Xcur + XYMAX;
	if( Xcur >  XYMAX/2 ) xcur = Xcur - XYMAX;
	if( Ycur < -XYMAX/2 ) ycur = Ycur + XYMAX;
	if( Ycur >  XYMAX/2 ) ycur = Ycur - XYMAX;
	move(9, Censtrt);
	if( s.s_des == '\0' ) s.s_des = ' ';
	printw("%c%5d,%-3d %3d", s.s_des, xcur, ycur, s.s_coun);
}

prfcen()
{
	int	i, thrsh;
	char	amntstr[8], thstr[8];

	mvaddstr(10, Censtrt, "               ");
	mvprintw(11, Censtrt, "%3d%%%10d", s.s_eff, s.s_mob);
	mvprintw(12, Censtrt, "m%4d   gm%4d", s.s_min, s.s_gold);
	mvaddstr(13, Censtrt, "               ");
	for( i = 0; i <= BAR; i++ ) {
		move(14+i, Censtrt);
		sprintf(amntstr, "%d", s.s_rsrc[i]);
		if( *amntstr == '0' ) *amntstr = '.';
		thrsh = (s.s_del[i] >> 4) & 017;
		thrsh *= 8;
		if( s.s_des == 'u' && i == CIV ) thrsh *= 10;
		if( s.s_des == 'b' && i == BAR ) thrsh *= 4;
		if( s.s_des == 'w' && (i==SHL || i==GUN ||
			i==ORE) ) thrsh *= 10;
		sprintf(thstr, "%d ", thrsh);
		if( *thstr == '0' ) *thstr = '.';
		printw("%c%5s%3s%5s ", Rchar[i], amntstr, Deldir[(int)(s.s_del[i]&017)], thstr);
	}
	mvprintw(22, Censtrt, "prod%4d    %c%c", s.s_prod, s.s_ckpt, s.s_def);
}

char
onmap()
{
	int	row, col;
	char	c;

	cbreak();
	noecho();
	if( Func != SHP ) {
		getsec(&s, secno(Xcur, Ycur));
		prcen();
	}
	row = Ycur - Yl + 1;
	col = (Xcur - Xl)*3 + 2;
	move(row, col);
	refresh();
	while( c = getch() ) {
		switch( c ) {
		case LF:
		case CR:
			nocbreak();
			echo();
			return('\n');
		case TAB:
		case 'i':
		case 'I':
		case PLUS:
		case MINUS:
		case SPACE:
			return(c);
		}
		switch( c ) {
		case UPLEFT:
		case LEFT  :
		case DNLEFT:
		case VILEFT:
			if( Xcur <= Xl ) break;
			Xcur--;
			col -= 3;
			break;
		case UPRIGHT:
		case RIGHT  :
		case DNRIGHT:
		case VIRIGHT:
			if( Xcur >= Xh ) break;
			Xcur++;
			col += 3;
			break;
		case DOWN:
		case UP:
		case VIDN:
		case VIUP:
			break;
		default:
			continue;
		}
		switch( c ) {
		case UPLEFT:
		case UP    :
		case UPRIGHT:
		case VIUP:
			if( Ycur <= Yl ) break;
			Ycur--;
			row--;
			break;
		case DNLEFT:
		case DOWN  :
		case DNRIGHT:
		case VIDN:
			if( Ycur >= Yh ) break;
			Ycur++;
			row++;
			break;
		}
		getsec(&s, secno(Xcur, Ycur));
		prcen();
		move(row, col);
		refresh();
	}
	return('\n');
}
