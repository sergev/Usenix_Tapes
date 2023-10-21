#define D_UPDATE
#define D_NATSTAT
#define D_SECTDES
#define D_NEWSVERBS
#define D_NATSTR
#define D_SCTSTR
#define D_DCHRSTR
#define D_ICHRSTR
#define D_NSCSTR
#define D_FILES
#include	"empdef.h"

desi()
{
	register	i, j;
	char	*cp, *delp, *getstar(), *cname(), *splur();
	char	goods[7];
	int	newdes, opkg, npkg, newcap;
	int	ii, jj;
	struct	ichrstr	*ip;
	struct	nstr	nsct;

	newcap = 0;
	if( snxtsct(&nsct, argp[1]) <  0 ) return(SYN_RETURN);
	while( nxtsct(&nsct, UP_NONE) > 0 ) {
		if( owner == 0 ) continue;
		if( nstat != STAT_GOD ) {
			if( sect.sct_desig == S_MOUNT ) continue;
			if( sect.sct_desig == S_BSPAN ) continue;
		}
		sprintf(fmtbuf,"%d,%d %d%% %s  desig? ", nsct.n_x, nsct.n_y, sect.sct_effic, dchr[sect.sct_desig].d_name);
		cp = fmtbuf;
		cp = getstar(argp[2], cp);
		if( *cp == '\0' ) continue;
		for( newdes = 0; newdes <= S_XCHNG; newdes++ ) {
			if( *cp == dchr[newdes].d_mnem ) break;
		}
		if( newdes > S_XCHNG || newdes == S_BSPAN ) {
			printf("See \"info sector-types\"");
			continue;
		}
		if( nstat != STAT_GOD ) {
			if( newdes == S_WATER || newdes == S_MOUNT ||
			    newdes == S_SANCT ) {
				printf("Only %s can make a %s!\n", cname(0), dchr[newdes].d_name);
				continue;
			}
		}
		j = 0;
		opkg = dchr[sect.sct_desig].d_pkg;
		npkg = dchr[newdes].d_pkg;
		if( npkg != opkg ) {
			for( i = 0; i <= 6; i++ ) {
				cp = i + (char *)&sect.sct_owned + 12;
				ip = &ichr[i + 12];
				j = (*cp * ip->i_pkg[opkg]) / ip->i_pkg[npkg]; 
				if( j < 0 || j > 127 ) {
					printf("Redesignation of %d,%d would cause loss of %s!\n", nsct.n_x, nsct.n_y, ip->i_name);
					break;
				} else {
					goods[i] = j;
				}
			}
		}
		if( j < 0  || j > 127 ) continue;
		if( newdes == S_CAPIT ) {
			getnat(cnum);
			if( nat.nat_stat == STAT_NOCAP ) {
				nat.nat_stat = STAT_NORMAL;
			}
			if( nsct.n_x == 0 && nsct.n_y == 0 && 
			    sect.sct_desig == S_CAPIT ) {
				printf("0,0 is already your capital.\n");
				putnat(cnum);
				continue;
			}
			nat.nat_xcap += nsct.n_x;
			nat.nat_ycap += nsct.n_y;
			for( i=0; i<4; i++ ) {
				nat.nat_b[i].b_xl -= nsct.n_x;
				nat.nat_b[i].b_yl -= nsct.n_y;
				nat.nat_b[i].b_xh -= nsct.n_x;
				nat.nat_b[i].b_yh -= nsct.n_y;
			}
			printf("Designation of new capital requires");
			printf(" revision of sector numbers.\n");
			printf("Capitol at %d,%d (old system)", nsct.n_x, nsct.n_y);
			printf(" will be at 0,0 (new system).\n");
			printf("You may log on again in a few minutes.\n");
			sigsave();
			newcap = 1;
			putnat(cnum);
		}
		if( newdes == sect.sct_desig) continue;
		if( sect.sct_desig == S_GMINE && sect.sct_ore != 0 ) {
			printf("Alchemists have turned %d", sect.sct_ore);
			printf(" ton%s of gold ore into ordinary ore in %d,%d!\n", splur(sect.sct_ore), nsct.n_x, nsct.n_y);
		}
		if( newdes == S_GMINE && sect.sct_ore != 0 ) {
			printf("Alchemists lost %d", sect.sct_ore);
			printf(" ton%s of ore in %d,%d trying to make gold.\n", splur(sect.sct_ore), nsct.n_x, nsct.n_y);
			sect.sct_ore = 0;
		}
		if( sect.sct_desig == S_SANCT ) nreport(cnum, N_BROKE_SANCT, 0);
		if( nsct.n_x == 0 && nsct.n_y == 0 && newdes != S_CAPIT ) {
			getnat(cnum);
			nat.nat_stat = STAT_NOCAP;
			putnat(cnum);
			printf("You are now without a capitol.\n");
		}
		if( sect.sct_desig == S_XCHNG ||
		    newdes == S_XCHNG ) {
			sect.sct_s_use = sect.sct_g_use = sect.sct_p_use = 0;
			sect.sct_o_use = sect.sct_b_use = 0;
		}
		if( newdes == S_HARBR ) {
			for( i = -1; i <= 1; i++ ) {
				for( j = -1; j <= 1; j++ ) {
					getsect(nsct.n_x + i, nsct.n_y + j, UP_NONE);
					if( sect.sct_desig == S_WATER ) goto X2162;
					if( sect.sct_desig == S_BSPAN ) goto X2162;
				}
			}
			printf("Sector %d,%d does not border on water.\n", nsct.n_x, nsct.n_y);
			continue;
X2162:	
			getsect(nsct.n_x, nsct.n_y, UP_NONE);
		}
		if( sect.sct_desig == S_BHEAD ) {
			for( i = -1; i <= 1; i++ ) {
				for( j = -1; j <= 1; j++ ) {
					getsect(nsct.n_x + i, nsct.n_y + j, UP_NONE);
					if( sect.sct_desig != S_BSPAN ) continue;
					for( ii = -1; ii <= 1; ii++ ) {
						for( jj = -1; jj <= 1; jj++ ) {
							getsect(nsct.n_x + i + ii, nsct.n_y + j + jj, UP_NONE);
							if( sect.sct_desig == S_BHEAD &&
							    (i + ii != 0 ||
							    j + jj != 0) ) goto X2534;
						}
					}
					printf("Crumble... SCREEEECH!  Splash!\n");
					getsect(nsct.n_x + i, nsct.n_y + j, UP_NONE);
					sect.sct_desig = S_WATER;
					sect.sct_owned = 0;
					putsect(nsct.n_x + i, nsct.n_y + j);
X2534:	
					;
				}
			}
			getsect(nsct.n_x, nsct.n_y, UP_NONE);
		}
		ip = &ichr[(int)dchr[sect.sct_desig].d_ptyp];
		delp = (int)ip->i_del + (char *)&sect.sct_owned;
		if( ip->i_del != 0 && *delp == 2 ) *delp = 0;
		sect.sct_desig = newdes;
		sect.sct_effic = sect.sct_prdct = sect.sct_contr = 0;
		if( npkg != opkg ) {
			for( i = 0; i <= 6; i++ ) {
				cp = i + (char *)&sect.sct_owned + 12;
				*cp = goods[i];
			}
		}
		putsect(nsct.n_x, nsct.n_y);
	}
	if( newcap != 0 ) bye();
	return(NORM_RETURN);
}
