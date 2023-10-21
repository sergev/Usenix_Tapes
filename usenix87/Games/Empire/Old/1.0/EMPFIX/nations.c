#define	D_NATSTR
#define	D_FILES
#include	"empdef.h"

extern int	xflg, wflg, mflg;

nations()
{
	register	num, i, k;
	char	*cp, *getstri();

	goto X1052;
X12:	
	num = atoi(cp);
	k = num * sizeof(nat);
	lseek(natf, (long)k, 0);
	i = read(natf, &nat, sizeof(nat));
	if( i >= sizeof(nat) ) goto X130;
	printf("Only %d bytes in that one...\n", i);
	goto X1052;
X130:	
	chfix(nat.nat_cnam);
	chfix(nat.nat_pnam);
	wordfix("btu", &nat.nat_btu, 0);
	wordfix("nuid", &nat.nat_nuid, 0);
	wordfix("playing", &nat.nat_playing, 0);
	wordfix("tgms", &nat.nat_tgms, 0);
	wordfix("xcap", &nat.nat_xcap, 0);
	wordfix("ycap", &nat.nat_ycap, 0);
	wordfix("stat", &nat.nat_stat, 0);
	wordfix("dayno", &nat.nat_dayno, 0);
	wordfix("minused", &nat.nat_minused, 0);
	i = 0;
X372:	
	if( wflg != 0 ) goto X422;
	if( xflg != 0 ) goto X422;
	printf("nat_b[%d]\n", i);
X422:	
	wordfix(" xl", &nat.nat_b[i].b_xl, 0);
	wordfix(" xh", &nat.nat_b[i].b_xh, 0);
	wordfix(" yl", &nat.nat_b[i].b_yl, 0);
	wordfix(" yh", &nat.nat_b[i].b_yh, 0);
	i++;
	if( i < 4 ) goto X372;
	longfix("date", &nat.nat_date, 0L);
	longfix("money", &nat.nat_money, 0L);
	wordfix("relate[0]", &nat.nat_relate[0], 0);
	wordfix("relate[1]", &nat.nat_relate[1], 0);
	floatfi("t_level", &nat.nat_t_level, 0.);
	floatfi("r_level", &nat.nat_r_level, 0.);
	if( num != 0 ) goto X770;
	floatfi("up_off", &nat.nat_up_off, 0.);
X770:	
	if( mflg == 0 ) goto X1052;
	lseek(natf, (long)k, 0);
	write(natf, &nat, sizeof(nat));
	printf("Rewritten\n");
X1052:	
	wflg = xflg = mflg = 0;
	cp = getstri("#? ");
	if( *cp == '\0' ) goto X1116;
	goto X12;
X1116:	
	return;
}
