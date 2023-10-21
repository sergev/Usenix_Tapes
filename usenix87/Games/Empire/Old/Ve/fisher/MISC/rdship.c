/* Copyright (c) 1985 by Thomas S. Fisher - Westminster, CO 80030 */

#include	<stdio.h>
#include	"emp.h"
#include	"ship.h"

FILE	*fopen(), *fdshp, *fdnshp;
struct	shpstr	Ship;

char	*Stype[] =	{
			"pt boat", "minesweep", "destroyer", "submarine",
			"freighter", "tender", "battleship", "carrier" };

main(argc, argv)
int	argc;
char	**argv;
{

	short	n, shipnum;
	short	mbl;
	char	*getenv(), *foestr;
	int	c;
	short	errflg;
	short	cflg, cno;
	short	relocflg, newx, newy;
	char	*fname;
	extern	char	*optarg;

	newx = newy = cflg = 0;
	fname = NULL;
	while( (c = getopt(argc, argv, "c:x:y:f:")) != EOF ) {
		switch( c ) {
		case 'c':
			cno = atoi(optarg);
			cflg++;
			break;
		case 'x':
			newx = atoi(optarg);
			break;
		case 'y':
			newy = atoi(optarg);
			break;
		case 'f':
			fname = optarg;
			break;
		case '?':
			errflg++;
			break;
		}
	}
	if( errflg ) {
		printf("Usage: %s [-c country_no] [-x newx] [-y newy]\n",
			argv[0]);
		exit(1);
	}
	if( newx || newy ) {
		relocflg++;
		if( (fdnshp = fopen("newship","w")) == NULL ) {
			fprintf(stderr,"Can't create newship file\n");
			exit(2);
		}
	}
	if( fname == NULL ) fname = "empship";
	if( (fdshp = fopen(fname, "r")) == NULL ) {
		fprintf(stderr, "Can't access %s file\n", fname);
		exit(3);
	}
	if( cno == 0 ) {
		foestr = getenv("FOES");
		if( foestr == NULL ) {
			cno = 0;
		} else {
			cno = atoi(foestr);
	}
	}
	shipnum = -1;
	while( (n=fread(&Ship, sizeof(struct shpstr), 1, fdshp)) == 1 ) { 
		shipnum++;
		if( !relocflg && Ship.shp_own != cno ) continue;
		if( Ship.shp_effc < 20 ) continue;
		if( relocflg ) {
			Ship.shp_xp -= newx;
			Ship.shp_yp -= newy;
		}
		Ship.shp_xp = Ship.shp_xp % XYMAX;
		if( Ship.shp_xp > XYMAX/2 ) Ship.shp_xp -= XYMAX;
		Ship.shp_yp = Ship.shp_yp % XYMAX;
		if( Ship.shp_yp > XYMAX/2 ) Ship.shp_yp -= XYMAX;
		if( relocflg ) {
			fwrite(&Ship, sizeof(struct shpstr), 1, fdnshp);
		} else {
			if( (mbl = Ship.shp_mbl) > 127 ) mbl -= 256;
        		printf("%4d %-11s%3d,%-3d%6d%%%4d%4d%4d%4d%4d%4d%4d\n",
				shipnum,
				Stype[(int)Ship.shp_type],
				Ship.shp_xp,	Ship.shp_yp,
				(int)Ship.shp_effc,
				(int)Ship.shp_crew,
				(int)Ship.shp_shels, (int)Ship.shp_gun,
				(int)Ship.shp_plns, (int)Ship.shp_or,
				(int)Ship.shp_gld,  mbl);
		}
	}
	if( relocflg ) fclose(fdnshp);
}
