#define D_NEWSVERBS
#define D_FILES
#include	"empdef.h"

decl()
{
	register char	*cp;
	char	*getstar();
	int	rel, oldrel, n;

	cp = getstar(argp[1], "alliance, neutrality or war? ");
	if( *cp != 'a' ) goto X46;
	rel = ALLIED;
	goto X110;
X46:	
	if( *cp != 'n' ) goto X62;
	rel = NEUTRAL;
	goto X110;
X62:	
	if( *cp != 'w' ) goto X100;
	rel = AT_WAR;
	goto X110;
X100:	
	printf("That's no declaration!");
	return(SYN_RETURN);
X110:	
	n = natarg(argp[2], "with which country? ");
	if( n == -1 ) return(SYN_RETURN);
	oldrel = getrel(cnum, n);
	if( oldrel == rel ) {
		printf("No change required for that!");
		return(FAIL_RETURN);
	}
	putrel(cnum, n, rel);
	if( oldrel == ALLIED ) {
		nreport(cnum, N_DIS_ALLY, n);
	} else if( oldrel == AT_WAR ) {
		nreport(cnum, N_DIS_WAR, n);
	}
	if( rel == ALLIED ) {
		printf("Congratulations.\n");
		nreport(cnum, N_DECL_ALLY, n);
	} else if( rel == NEUTRAL ) {
		printf("Neutrality declared\n");
		nreport(cnum, N_DECL_NEUT, n);
	} else if( rel == AT_WAR ) {
		printf("Declaration made (give 'em hell)\n");
		nreport(cnum, N_DECL_WAR, n);
	}
	return(NORM_RETURN);
}
