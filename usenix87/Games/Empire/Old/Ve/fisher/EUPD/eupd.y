%{
/* Copyright (c) 1985 by Thomas S. Fisher - Westminster, CO 80030 */

#include	<stdio.h>
#include	"emp.h"
#include	"ship.h"

FILE	*fopen(), *fdsec, *fdship;
struct	sector	s;
struct	shpstr	Ship;

extern	char	Stype[], Cenflags[];
extern  char    Rscan[MAXSCN][2*MAXSCN], Raddes[]; 
extern	int	Nrows, Xrad, Yrad, Rngrad;
int	Radflag;

int	Maphdr = 0;
int	Mapx, Mapy, Maplcol, Maptens;
int	Colno = 0;
int	Line = 0;
int	Debug = 0;
int	loo_corm[2], i, n;
int	Xfly, Yfly;

main(argc, argv)
int	argc;
char	**argv;
{
	extern	FILE	*fopen(), *fdsec;
	extern	int	yydebug, Debug;
	extern	struct	sector	s;
	extern	int	optind;
	extern	char	*optarg;

	int	c, x, y, n;
	int	pflg, errflg;

	if( (fdsec=fopen("empsect","r+")) == NULL ) {
		fprintf(stderr,"Can't open empsect\n");
		exit(1);
	}
	if( (fdship=fopen("empship","r+")) == NULL ) {
		fprintf(stderr,"Can't open empship\n");
	}

	while(( c = getopt(argc, argv, "pd")) != EOF ) {
		switch( c ) {
		case 'p':
			pflg++;
			break;
		case 'd':
			Debug++;
			break;
		case '?':
			errflg++;
		}

		if( errflg ) {
			fprintf(stderr, "Usage: %s [-p]\n", argv[0] );
			exit(2);
		}
		if( pflg ) {
			fprintf(stderr, "Purging world\n");
			for( y = 0; y < XYMAX; y++ ) {
				for( x = 0; x < XYMAX; x++ ) {
					getsec(&s, (n = secno(x, y)));
					if( s.s_coun == 0 &&
					  s.s_des != '.' && s.s_des != ' ' &&
					  s.s_des != '\0' && 
					  (s.s_rsrc[CIV]>0 || s.s_rsrc[MIL]>0)){
						s.s_coun = 98;
						putsec(&s, n);
					}
				}
			}
			fprintf(stderr, "World purged\n");
			exit(0);
		}
	}
	yyparse();
}
%}
%token	EOL SECTLET SECTNLET ANYCHAR
%token	NUM CMD EX
%token	SPY DPRTD
%token	SHIP SECT
%token	STPD SUB AT
%token	EFF WITH APPRX CIVTKN MILTKN
%token	SHLTKN GUNTKN PLNTKN ORETKN BARTKN
%token	DLVR FROM TO THRSH
%token	YOUR UNOWN
%token	BAROM RANGE RADAR
%token	CMSGPOB CENFLGS SECTORS
%token	NWOVR FLYOVR CMPRTLY CNSTRCTD
%token	MAPHDR
%start	session
%%

session		:	cmdout
		|	session	cmdout
		;
cmdout		:	cprompt
		|	EOL
		|	cenout
		|	mapout
		|	ship_rec
		|	nav_rec
		|	nav_info
		|	loo_rec
		|	radout
		|	del_rec
		|	spy_rec
		|	fly_rec
		|	error EOL
			{
				if( Debug ) {
					printf("%dp\n", Line);
				}
			}
		;
cprompt		:	'[' NUM ':' NUM ']' cmd_or_ex ':'
				{printf("[%d:%d]\n", $2, $4);
				}
		;
cmd_or_ex	:	CMD
		|	EX
		;
cenout		:	error CMSGPOB
			error EOL
			cen_recs NUM SECTORS EOL
			{	printf("%d sectors\n", $6);	}
		;
cen_recs	:	cen_rec
		|	cen_recs cen_rec
		;
cen_rec		:	error EOL
		|	NUM ',' NUM CENFLGS SECT NUM '%'
			NUM NUM NUM NUM NUM NUM NUM NUM NUM NUM NUM EOL
			{
				getsec(&s, (n = secno($1, $3)));
				s.s_coun = 0;
				s.s_ckpt = Cenflags[0];
				for( i = 0; i <= 6; i++ ) {
					setdel(Cenflags[i+1], &s.s_del[i]);
				}
				s.s_def  = Cenflags[8];
				s.s_des  = $5;
				s.s_eff  = $6;
				s.s_min  = $8;
				s.s_gold = $9;
				s.s_mob  = $10;
				s.s_rsrc[0] = $11;
                                s.s_rsrc[1] = $12;
                                s.s_rsrc[2] = $13;
                                s.s_rsrc[3] = $14;
                                s.s_rsrc[4] = $15;
                                s.s_rsrc[5] = $16;
                                s.s_rsrc[6] = $17;
				s.s_prod = $18;
				putsec(&s, n);
			}
		;
mapout		:	MAPHDR EOL MAPHDR EOL map_recs MAPHDR EOL MAPHDR EOL
		;
map_recs	:	map_rec
		|	map_recs map_rec
		;
map_rec		:	NUM
			{ Mapy = $1; }
			map_chars NUM EOL
		|	NUM NUM EOL
		;
map_chars	:	map_char
			{	if( Colno < 6 || Colno > Maplcol ) YYERROR;
				if( Colno%2 != 0 ) YYERROR;
				domap(Mapx, Mapy, Colno, (char)$1);
			}
		|	map_chars map_char
			{	if( Colno < 6 || Colno > Maplcol ) YYERROR;
				if( Colno%2 != 0 ) YYERROR;
				domap(Mapx, Mapy, Colno, (char)$2);
			}
		;
map_char	:	SECTLET
		|	SECTNLET
		|	'#'
			{ $$ = (int)'#'; }
		|	')'
			{ $$ = (int)')'; }
		| 	'.'
			{ $$ = (int)'.'; }
		;
ship_rec	:	NUM SHIP NUM ',' NUM ship_fleet NUM '%'
			{	if( $4 != 20 || $8 != 30 ) YYERROR;
			}
			NUM NUM NUM NUM NUM NUM NUM EOL
			{	Ship.shp_type = $2;
				Ship.shp_own  = 0;
				Ship.shp_xp   = $3;
				Ship.shp_yp   = $5;
				Ship.shp_fleet= $6;
				Ship.shp_effc = $7;
				Ship.shp_crew = $10;
				Ship.shp_shels= $11;
				Ship.shp_gun = $12;
				Ship.shp_plns = $13;
				Ship.shp_or   = $14;
				Ship.shp_gld  = $15;
				Ship.shp_mbl  = $16;
				putship(&Ship, $1);
			}
		;
ship_fleet	:	SECTLET
		|	ANYCHAR
		|
			{	$$ = ' ';		}
		;
nav_rec		:	SHIP '#' NUM STPD AT NUM ',' NUM EOL
			{	printf("Ship %d stopped at %3d,%-3d\n",
					$3, $6, $8);
				if( getship(&Ship, $3) == ERROR ) {
					Ship.shp_fleet = ' ';
					Ship.shp_crew  = 0;
					Ship.shp_shels = Ship.shp_gun = 0;
					Ship.shp_plns  = Ship.shp_or  = 0;
					Ship.shp_gld   = Ship.shp_mbl = 0;
					Ship.shp_effc = 100;
				}
				Ship.shp_own  = 0;
				Ship.shp_xp   = $6;
				Ship.shp_yp   = $8;
				putship(&Ship, $3);
			}
		;
nav_info	:	 '<' NUM '.' NUM ':' NUM '.' NUM ':' NUM ',' NUM '>' 
		;
loo_rec         :       loo_count SHIP '#' NUM '@' NUM ',' NUM EOL 
			{	if( getship(&Ship, $4) == ERROR ) {
					Ship.shp_fleet = ' ';
					Ship.shp_crew  = 0;
					Ship.shp_shels = Ship.shp_gun = 0;
					Ship.shp_plns  = Ship.shp_or  = 0;
					Ship.shp_gld   = Ship.shp_mbl = 0;
				}
				Ship.shp_effc = 100;
				Ship.shp_own  = $1;
				Ship.shp_type = $2;
				Ship.shp_xp   = $6;
				Ship.shp_yp   = $8;
				putship(&Ship, $4);
			}
		|	SUB AT NUM ',' NUM EOL
			{	printf("Sub at %3d,%-3d\n", $3, $5);
			}
		|	loo_count SECT NUM '%' EFF
			{	loo_corm[0] = -1;
				loo_corm[1] = -1;
			}
			loo_cntnt '@' NUM ',' NUM EOL
			{
				if( $1 != 0 ) {
					getsec(&s, (n = secno($9, $11)));
					s.s_coun = $1;
					s.s_des = $2;
					s.s_eff = $3 * 2;
					if( loo_corm[0] > 0 ) {
						s.s_rsrc[CIV]=loo_corm[0];
						if( s.s_des == 'u' ) {
							s.s_rsrc[CIV] *= 10;
						}
					}
					if( loo_corm[1] > 0 ) {
						s.s_rsrc[MIL]=loo_corm[1];
					}
					putsec(&s, n);
				}
			}
		;
loo_count	:	UNOWN
			{	$$ = 99	;	}
		|	YOUR
			{	$$ = 0;		}
		|	error '(' '#' NUM ')'
			{	$$ = $4;	}
		;
loo_cntnt	:
		|	WITH NUM loo_rsrc
			{	loo_corm[$3] = $2;	}
		|	WITH APPRX NUM loo_rsrc
			{	loo_corm[$4] = $3 * 2;	}
		|	WITH NUM loo_rsrc '&' NUM loo_rsrc
			{
				loo_corm[$3] = $2;
				loo_corm[$6] = $5;
			}
		|	WITH APPRX NUM loo_rsrc WITH APPRX NUM loo_rsrc
			{
				loo_corm[$4] = $3 * 2;
				loo_corm[$8] = $7 * 2;
			}
		;
loo_rsrc	:	CIVTKN
			{	$$ = 0;		}
		|	MILTKN
			{	$$ = 1;		}
		;
radout		:	rad_source NUM ',' NUM EOL
			{
				Xrad = $2;
				Yrad = $4;
			}
			EFF NUM '%' ',' BAROM AT NUM ','
			RANGE NUM '.' 
			{
				Rngrad = $16;
				Radflag = 1;
				Nrows = 0;
			}
			NUM EOL
		;
rad_source	:	RADAR AT
		|	SHIP NUM AT
		;
del_rec		:	DLVR del_rsrc FROM SECT '@' NUM ',' NUM
			TO NUM ',' NUM del_thrsh EOL
			{
				i = $13/8;
				getsec(&s, (n = secno($6, $8)));
				if( $4 == 'u' && $2 == CIV ) i /= 10;
				if( $4 == 'b' && $2 == BAR ) i /= 4;
				if( $4 == 'w' && ($2==SHL || $2==GUN ||
					$2==ORE) ) i /= 10;
				s.s_del[$2] &= 017;
				s.s_del[$2] |= (i & 017) << 4;
				putsec(&s, n);
			}
		;
del_rsrc	:	CIVTKN
		|	MILTKN
		|	SHLTKN
		|	GUNTKN
		|	PLNTKN
		|	ORETKN
		|	BARTKN
		;
del_thrsh	:
			{	$$ = 0;			}
		|	'(' THRSH NUM ')'
			{	$$ = $3;		}
		;
spy_rec		:	NUM ',' NUM NUM spy_sect NUM '%' 
			{
				if( $2 != 4 || $7 != 17 ) YYERROR;
			}
			NUM NUM NUM NUM NUM NUM EOL
			{

				getsec(&s, (n = secno($1, $3)));
				s.s_coun = ($4==0 ? 99 : $4);
				s.s_des = (char)$5;
				s.s_eff = 2 * $6;
				s.s_rsrc[CIV] = 2 * $9;
				s.s_rsrc[MIL] = 2 * $10;
				s.s_rsrc[SHL] = 2 * $11;
				s.s_rsrc[GUN] = 2 * $12;
				s.s_rsrc[ORE] = 2 * $13;
				s.s_rsrc[PLN] = 2 * $14;
				putsec(&s, n);
			}
		|	SPY DPRTD FROM NUM ',' NUM '.' EOL
			{
				getsec(&s, (n = secno($4, $6)));
				if( s.s_coun == 0 ) {
					s.s_coun = 98;
					putsec(&s, n);
				}
			}
		;
spy_sect	:	SECTLET
		|	SECTNLET
		|	'#'
			{ $$ = (int)'#'; }
		|	')'
			{ $$ = (int)')'; }
		;
fly_rec		:	fly_info fly_rec
		|	fly_info NWOVR CMPRTLY CNSTRCTD SECT '.' EOL 
			{
				getsec(&s, n = secno(Xfly, Yfly));
				s.s_des = $5;
				putsec(&s, n);
			}
		|	fly_info FLYOVR SECT '.' EOL
			{
				getsec(&s, n = secno(Xfly, Yfly));
				s.s_des = $3;
				putsec(&s, n);
			}
		;
fly_info	:	'<' NUM '.' NUM ':' NUM ':' NUM ':' NUM ',' NUM '>'
			{
				Xfly = $10;
				Yfly = $12;
			}
		;
%%
#include	"lex.yy.c"

yyerror(s)
char	*s;
{
}
