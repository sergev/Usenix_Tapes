	/* ship types */
#define	S_PT	0
#define	S_MIN	1
#define	S_DES	2
#define	S_SUB	3
#define	S_FRE	4
#define	S_TEN	5
#define	S_BAT	6
#define	S_CAR	7
#define	TMAXNO	7

struct	shpstr {
	char    shp_own;            /* country # of owner */
	char    shp_type;           /* ship type */
	char    shp_effc;           /* 0 - 100 */
	short   shp_xp, shp_yp;     /* location in abs coords */
	char    shp_fleet;          /* group membership */
	char    shp_crew;           /* military | civvies on board */
	char    shp_shels;          /* shells on board */
	char    shp_gun;            /* etc */
	char    shp_plns;
	char    shp_or;
	char    shp_gld;
	char    shp_spric;          /* ship price, if for sale */
	char    shp_mbl;            /* mobility */
	int     shp_lstp;           /* time of last update */
};
