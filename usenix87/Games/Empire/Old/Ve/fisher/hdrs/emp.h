/* Copyright (c) 1985 by Thomas S. Fisher - Westminster, CO 80030 */

/*	Define BSD if you're building this on a 4.2 BSD system */
/*	Otherwise (for SysV), leave it undefined               */
/*	#define	BSD	*/

#define	MAXSCN	20
#define	XYMAX	64
#define	ERROR	-1
#define	OK	1

/*
 *  Sector Record
 */

#define	CIV	0
#define	MIL	1
#define	SHL	2
#define	GUN	3
#define	PLN	4
#define	ORE	5
#define	BAR	6

struct  sector {
	int	s_secno;
	char	s_des;
	char	s_flag;
	int	s_coun;
	int	s_date;
	char	s_ckpt;
	char	s_def;
	int	s_eff;
	int	s_min;
	int	s_gold;
	int	s_mob;
	int	s_rsrc[7];
	int	s_prod;
	char	s_del[7];
	char	s_rsrvd[9];
	} ;

/*
 *
 */
