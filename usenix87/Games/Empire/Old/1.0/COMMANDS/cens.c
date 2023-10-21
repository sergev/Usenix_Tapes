#define D_UPDATE
#define D_NATSTAT
#define D_SECTDES
#define D_SCTSTR
#define D_DCHRSTR
#define D_ICHRSTR
#define D_NSCSTR
#define D_FILES
#include	"empdef.h"

cens()
{
	char	*del, *splur();
	int	nsects, dpkg, i;
	struct	nstr	nsct;

	if( snxtsct(&nsct, argp[1]) >= 0 ) goto X44;
	return(SYN_RETURN);
X44:	
	prdate();
	del = ".4$023.1....65.7";
	nsects = 0;
	goto X1712;
X66:	
	if( owner != 0 ) goto X100;
	goto X1712;
X100:	
	if( nsects++ != 0 ) goto X154;
	if( nstat != STAT_GOD ) goto X134;
	printf("   ");
X134:	
	printf("  sect  cmsgpob des eff min gold mob");
	printf("  civ mil   sh  gun  pl  ore bar prod\n");
X154:	
	if( nstat != STAT_GOD ) goto X212;
	printf("%2d ", sect.sct_owned);
X212:	
	printf("%3d,%-3d", nsct.n_x, nsct.n_y);
	printf("%c", (sect.sct_chkpt != 0) ? '*' : ' ');
	if( sect.sct_desig != S_XCHNG ) goto X354;
	sect.sct_c_use = sect.sct_m_use = sect.sct_s_use = sect.sct_g_use = sect.sct_p_use = sect.sct_o_use = sect.sct_b_use = 0;
X354:	
	printf("%c", del[sect.sct_c_use & 017]);
	printf("%c", del[sect.sct_m_use & 017]);
	printf("%c", del[sect.sct_s_use & 017]);
	printf("%c", del[sect.sct_g_use & 017]);
	printf("%c", del[sect.sct_p_use & 017]);
	printf("%c", del[sect.sct_o_use & 017]);
	printf("%c", del[sect.sct_b_use & 017]);
	printf("%c", (sect.sct_dfend != 0) ? '%' : ' ');
	printf(" %c", dchr[sect.sct_desig].d_mnem);
	printf("%4d%%%4d", sect.sct_effic, sect.sct_miner);
	printf("%5d%4d", sect.sct_gmin, sect.sct_mobil);
	dpkg = dchr[sect.sct_desig].d_pkg;
	i = 12;
	printf("%5d", ichr[i].i_pkg[dpkg] * sect.sct_civil);
	i = 13;
	printf("%4d", ichr[i].i_pkg[dpkg] * sect.sct_milit);
	i = 14;
	printf("%5d", ichr[i].i_pkg[dpkg] * sect.sct_shell);
	i = 15;
	printf("%5d", ichr[i].i_pkg[dpkg] * sect.sct_guns);
	i = 16;
	printf("%4d", ichr[i].i_pkg[dpkg] * sect.sct_plane);
	i = 17;
	printf("%5d", ichr[i].i_pkg[dpkg] * sect.sct_ore);
	i = 18;
	printf("%4d", ichr[i].i_pkg[dpkg] * sect.sct_gold);
	if( sect.sct_contr == 0 ) goto X1652;
	if( sect.sct_desig == S_MINE ) goto X1652;
	if( sect.sct_desig != S_GMINE ) goto X1702;
X1652:	
	printf("%4d\n", sect.sct_prdct);
	goto X1712;
X1702:	
	printf("  $\n");
X1712:	
	if( nxtsct(&nsct, UP_OWN) <= 0 ) goto X1742;
	goto X66;
X1742:	
	printf("    %d sector%s\n", nsects, splur(nsects));
	return(NORM_RETURN);
}
