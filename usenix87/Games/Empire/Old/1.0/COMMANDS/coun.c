#define D_NATSTAT
#define D_NATSTR
#define D_FILES
#include	"empdef.h"

coun()
{
	register char	*cp;
	register	i;
	char	*ctime();

	prdate();
	printf("  #   last access       time\tstatus\t country name\n");
	for( i=0; ; i++ ) {
		if( getnat(i) == -1 ) break;
		if( nat.nat_stat == 0 ) continue;
		if( nat.nat_stat == 0 ) continue;
		printf("%3d  %-16.16s   [%d]", i, (nat.nat_playing)?" Now logged on":ctime(&nat.nat_date), nat.nat_btu);
		switch( nat.nat_stat ) {
		case STAT_VISITOR:
			cp = "Visitor";
			break;
		case STAT_NEW:
			cp = "New";
			break;
		case STAT_NOCAP:
			cp = "In flux";
			break;
		case STAT_NORMAL:
			cp = "Active";
			break;
		case STAT_GOD:
			cp = "DEITY";
			break;
		}
		printf("\t%-7.7s\t %s\n", cp, nat.nat_cnam);
	}
	return(NORM_RETURN);
}
