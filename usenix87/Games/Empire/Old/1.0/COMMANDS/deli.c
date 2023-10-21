#define D_UPDATE
#define D_SECTDES
#define D_SCTSTR
#define D_DCHRSTR
#define D_ICHRSTR
#define D_NSCSTR
#define D_FILES
#include	"empdef.h"

deli()
{
	register char	*cp, c, *delp;
	int	i;
	short	dx, dy;
	unsigned int	ptyp;
	int	sthresh, thresh, threshty;
	int	units;
	struct	ichrstr	*ip;
	struct	nstr	nsct;
	char	 *getstar(), *getstri(), *cname();

	cp = getstar(argp[1], "item? ");
	for( ptyp=12; (c = ichr[ptyp].i_mnem) != '\0'; ptyp++ ) {
		if( c == *cp ) break;
	}
	if( c == '\0' ) {
		printf("'%s'? The union won't let us deliver them!", cp);
		return(SYN_RETURN);
	}
	ip = &ichr[ptyp];
	delp = (int)ip->i_del + (char *)&sect.sct_owned;
	if( argp[3] != 0 && argp[3][0] != '\0' ) {
		switch( argp[3][0] ) {
		case '-':
			threshty = '-';
			break;
		case '+':
			threshty = '+';
			sthresh = atopi(argp[3] + 1);
			break;
		case '(':
			threshty = '(';
			sthresh = atopi(argp[3] + 1);
			break;
		default:
			printf("`%s' is not a valid arg; see `info deliver'.\n", argp[3]);
			return(SYN_RETURN);
		}
	} else {
		threshty = ' ';
		sthresh = 0;
	}
	if( snxtsct(&nsct, argp[2]) == -1 ) return(SYN_RETURN);
	if( threshty == '-' ) {
		while( nxtsct(&nsct, UP_OWN) > 0 ) {
			if( owner == 0 || *delp == 0 ) continue;
			i = dchr[sect.sct_desig].d_pkg;
			units = ip->i_pkg[i];
			if( sect.sct_contr != 0 &&
			    ptyp == (int)dchr[sect.sct_desig].d_ptyp ) {
				printf("%s in %d,%d being sold to %s\n", ip->i_name, nsct.n_x, nsct.n_y, cname(0));
				continue;
			}
			if( sect.sct_desig == S_XCHNG ) {
				if( ptyp >= 14 ) {
					printf("%s for sale in exchange at %d,%d.\n", ip->i_name, nsct.n_x, nsct.n_y);
					continue;
				}
			}
			dx = (*delp)<<12;
			dx = (dx>>14) + nsct.n_x;
			dy = (*delp)<<14;
			dy = (dy>>14) + nsct.n_y;
			printf("Deliver %s from %s @ %d,%d to %d,%d", ip->i_name, dchr[sect.sct_desig].d_name, nsct.n_x, nsct.n_y, dx, dy);
			if( ((char)*delp & 0360) != 0 ) {
				printf(" (thresh %d)\n", ((*delp>>1) & 0170) * units);
			} else {
				printf("\n");
			}
		}
	} else {
		while( nxtsct(&nsct, UP_OWN) > 0 ) {
			if( owner == 0 ) continue;
			i = dchr[sect.sct_desig].d_pkg;
			units = ip->i_pkg[i];
			if( sect.sct_contr != 0 &&
			    ptyp == (int)dchr[sect.sct_desig].d_ptyp ) {
				printf("%s in %d,%d currently being sold to %s\n", ip->i_name, nsct.n_x, nsct.n_y, cname(0));
				sprintf(fmtbuf,"Do you want to terminate that contract?");
				cp = getstri(fmtbuf);
				if( *cp != 'y' ) continue;
				sect.sct_contr = 0;
				putsect(nsct.n_x, nsct.n_y);
			}
			if( sect.sct_desig == S_XCHNG &&
			    ptyp >= 14 ) {
				printf("%s for sale in exchange at %d,%d.\n", ip->i_name, nsct.n_x, nsct.n_y);
				continue;
			}
X1720:	
			thresh = sthresh;
			if( threshty != '+' ) {
				sprintf(fmtbuf,"Deliver %s from %s @ %d,%d to ", ip->i_name, dchr[sect.sct_desig].d_name, nsct.n_x, nsct.n_y);
				cp = getstri(fmtbuf);
				if( *cp == '\0' ||
				    getsno(cp, "enter destination x, y") < 0 ) continue;
				if( (sx-nsct.n_x)*(sx-nsct.n_x)+(sy-nsct.n_y)*(sy-nsct.n_y) > 2 ) {
					printf("Too far for automated delivery\n");
					getsect(nsct.n_x, nsct.n_y, UP_NONE);
					goto X1720;
				}
				dx = sx;
				dy = sy;
				getsect(nsct.n_x, nsct.n_y, UP_OWN);
				while( *cp != '\0' ) {
					if( *cp++ == '(' ) {
						thresh = atopi(cp);
						break;
					}
				}
			} else {
				dx = (*delp)<<12;
				dx = (dx>>14) + nsct.n_x;
				dy = (*delp)<<14;
				dy = (dy>>14) + nsct.n_y;
			}
			i = ((thresh / units)<<1) & 0360;
			*delp = (((dx - nsct.n_x)<<2) & 014) + ((dy - nsct.n_y) & 03) + i;
			putsect(nsct.n_x, nsct.n_y);
		}
	}
	return(NORM_RETURN);
}
