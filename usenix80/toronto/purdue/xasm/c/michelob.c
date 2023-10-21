#include "/v/wa1yyn/c/plot/plot.h"
#define ECHO 010
#define RAW 040
#define M 2
#define D 3
#define C 4
#define REFX 5160
#define REFY 3880
	char plotob[600];
	int pstat[20];
	int field;		/* current field	*/
	int place[3];		/* [0] = x, [1] = y	*/
	int lastep[3];		/* Last end point of a line */
	char crossterm;		/* croshair terminate char	*/
	char rotdir;		/* rotation direction	*/
	char rotscale;		/* scale factor		*/
	char command[200];	/* current command	*/
	char boxname[200];	/* current box name	*/
	int aposit;		/* a/n cursor position	*/
	int apositx;		/* a/n x cursor positon	*/
	int ttybuf[3],savebuf[3]; /* half/full dup tty buf*/

	char *boxkind[]{	/* kinds of boxes	*/
		"#42fE2z",	/* dummy string */
		"square",
		"nose",
		"and2",
		"nand2",
		"and3",
		"nand3",
		"and4",
		"nand4",
		"buf",
		"inv",
		"tbuf",
		"tinv",
		"or2",
		"nor2",
		"xor2",
		"ff",
		"trail",
		"lead",
		"ffp",
		"hyst",
		"arrow",
		"feather",
		"bi",
		"d",
		"res",
		"cap",
		"var",
		"bat",
		"npn",
		"pnp",
		"t",
		"mu",
		"ohm",
		"sigma",
		"int",
		"four",
		"diode",
		"zen",
		"ground",
		"earth",
		"photon",
		"fuse",
		"light",
		"coax",
		"varactor",
		"triac",
		"coil",
		"relay",
		"ind",
		"slug",
		"wiper",
		"ff7473",
		"ff7474",
		"ff7476",
		"ant",
		"crystal",
		"bus",
		"mud",
		"dot",
		"ic6",
		"ic8",
		"ic14",
		"ic16",
		"ic18",
		"ic24",
		"ic28",
		"ic40",
		-1
		};


    int boxdata[]{
/*	command		x	y
Valid commands are:
	0	End of list
	1	End of this symbol
	2	'M'ove
	3	'D'raw
	4	put char in y.  (one char. only)
							*/
	D,	0,	320,		/* square	*/
	D,	320,	0,
	D,	0,	-320,
	D,	-320,	0,
	1,	0,	0,
	M,	0,	-30,		/* nose		*/
	D,	0,	60,
	D,	50,	50,
	D,	60,	0,
	D,	50,	-50,
	D,	0,	-60,
	D,	-50,	-50,
	D,	-60,	0,
	D,	-50,	50,
	M,	0,	30,
	1,	0,	0,
	D,	0,	400,		/* and2	*/
	D,	470,	0,
	D,	60,	-30,
	D,	50,	-60,
	D,	40,	-80,
	D,	0,	-60,
	D,	-40,	-80,
	D,	-50,	-60,
	D,	-60,	-30,
	D,	-470,	0,
	M,	0,	80,
	D,	-120,	0,
	M,	0,	240,
	D,	120,	0,
	M,	620,	-120,
	D,	140,	0,
	M,	-140,	0,
	M,	-620,	-200,
	1,	0,	0,
						/* nand2	*/
	D,	0,	400,
	D,	470,	0,
	D,	60,	-30,
	D,	50,	-60,
	D,	40,	-80,
	D,	0,	-60,
	D,	-40,	-80,
	D,	-50,	-60,
	D,	-60,	-30,
	D,	-470,	0,
	M,	0,	80,
	D,	-120,	0,
	M,	0,	240,
	D,	120,	0,
	M,	620,	-120,
	D,	26,	28,
	D,	26,	0,
	D,	26,	-28,
	D,	100,	0,
	M,	-100,	0,
	D,	-27,	-28,
	D,	-26,	0,
	D,	-26,	28,
	1,	0,	0,
					/* and3	*/
	D,	0,	560,
	D,	470,	0,
	D,	90,	-40,
	D,	80,	-80,
	D,	40,	-120,
	D,	0,	-80,
	D,	-40,	-120,
	D,	-80,	-80,
	D,	-90,	-40,
	D,	-470,	0,
	M,	0,	80,
	D,	-120,	0,
	M,	0,	200,
	D,	120,	0,
	M,	0,	200,
	D,	-120,	0,
	M,	800,	-200,
	D,	160,	0,
	M,	-160,	0,
	M,	-680,	-280,
	1,	0,	0,
					/* nand3	*/
	D,	0,	560,
	D,	470,	0,
	D,	90,	-40,
	D,	80,	-80,
	D,	40,	-120,
	D,	0,	-80,
	D,	-40,	-120,
	D,	-80,	-80,
	D,	-90,	-40,
	D,	-470,	0,
	M,	0,	80,
	D,	-120,	0,
	M,	0,	200,
	D,	120,	0,
	M,	0,	200,
	D,	-120,	0,
	M,	800,	-200,
	D,	26,	28,
	D,	27,	0,
	D,	27,	-28,
	D,	80,	0,
	M,	-80,	0,
	D,	-27,	-28,
	D,	-27,	0,
	D,	-26,	28,
	M,	-680,	-280,
	1,	0,	0,
					/* and4	*/
	M,	0,	-60,
	D,	0,	840,
	D,	450,	0,
	D,	130,	-80,
	D,	110,	-130,
	D,	70,	-170,
	D,	0,	-80,
	D,	-70,	-170,
	D,	-110,	-130,
	D,	-130,	-80,
	D,	-450,	0,
	M,	0,	60,
	D,	-120,	0,
	M,	0,	240,
	D,	120,	0,
	M,	0,	240,
	D,	-120,	0,
	M,	0,	240,
	D,	120,	0,
	M,	760,	-360,
	D,	160,	0,
	M,	-160,	0,
	M,	-760,	-420,
	1,	0,	0,
					/* nand4	*/
	M,	0,	-60,
	D,	0,	840,
	D,	450,	0,
	D,	130,	-80,
	D,	110,	-130,
	D,	70,	-170,
	D,	0,	-80,
	D,	-70,	-170,
	D,	-110,	-130,
	D,	-130,	-80,
	D,	-450,	0,
	M,	0,	60,
	D,	-120,	0,
	M,	0,	240,
	D,	120,	0,
	M,	0,	240,
	D,	-120,	0,
	M,	0,	240,
	D,	120,	0,
	M,	760,	-360,
	D,	26,	28,
	D,	27,	0,
	D,	27,	-28,
	D,	80,	0,
	M,	-80,	0,
	D,	-27,	-28,
	D,	-27,	0,
	D,	-26,	28,
	M,	-760,	-420,
	1,	0,	0,

	M,	120,	-180,
	D,	0,	360,		/* buf	*/
	D,	360,	-180,
	D,	-360,	-180,
	M,	0,	180,
	D,	-120,		0,
	M,	480,	0,
	D,	120,	0,
	M,	-600,	0,
	1,	0,	0,
	M,	120,	-180,
	D,	0,	360,		/* inv	*/
	D,	360,	-180,
	D,	-360,	-180,
	M,	0,	180,
	D,	-120,		0,
	M,	480,	0,
	D,	26,	25,
	D,	27,	0,
	D,	27,	-25,
	D,	80,	0,
	M,	-80,	0,
	D,	-27,	-25,
	D,	-27,	0,
	D,	-26,	25,
	M,	-480,	0,
	1,	0,	0,
	M,	120,	-180,
	D,	0,	360,		/* tbuf	*/
	D,	360,	-180,
	D,	-360,	-180,
	M,	0,	180,
	D,	-120,		0,
	M,	480,	0,
	D,	120,	0,
	M,	-280,	-80,
	D,	0,	-120,
	M,	-320,	200,
	1,	0,	0,
	M,	120,	-180,
	D,	0,	360,		/* tinv	*/
	D,	360,	-180,
	D,	-360,	-180,
	M,	0,	180,
	D,	-120,	0,
	M,	480,	0,
	D,	26,	25,
	D,	27,	0,
	D,	27,	-25,
	D,	80,	0,
	M,	-80,	0,
	D,	-26,	-25,
	D,	-27,	0,
	D,	-27,	25,
	M,	-160,	-80,
	D,	0,	-120,
	M,	-320,	200,
	1,	0,	0,
	M,	120,	-80,
	D,	40,	80,		/* or2	*/
	D,	-160,	0,
	M,	160,	0,
	D,	20,	120,
	D,	-20,	120,
	D,	-160,	0,
	M,	160,	0,
	D,	-40,	80,
	D,	450,	0,
	D,	80,	-40,
	D,	60,	-70,
	D,	50,	-90,
	D,	160,	0,
	M,	-160,	0,
	D,	-50,	-90,
	D,	-60,	-70,
	D,	-80,	-40,
	D,	-450,	0,
	M,	-120,	80,
	1,	0,	0,
	M,	120,	-80,
	D,	40,	80,		/* nor2	*/
	D,	-160,	0,
	M,	160,	0,
	D,	20,	120,
	D,	-20,	120,
	D,	-160,	0,
	M,	160,	0,
	D,	-40,	80,
	D,	450,	0,
	D,	80,	-40,
	D,	60,	-70,
	D,	50,	-90,
	D,	26,	25,
	D,	27,	0,
	D,	27,	-25,
	D,	80,	0,
	M,	-80,	0,
	D,	-27,	-25,
	D,	-27,	0,
	D,	-26,	25,
	D,	-50,	-90,
	D,	-60,	-70,
	D,	-80,	-40,
	D,	-450,	0,
	M,	-120,	80,
	1,	0,	0,
	M,	120,	-80,
	D,	40,	80,		/* xor2	*/
	D,	-160,	0,
	M,	160,	0,
	D,	20,	120,
	D,	-20,	120,
	D,	-160,	0,
	M,	160,	0,
	D,	-40,	80,
	M,	50,	0,
	D,	420,	0,
	D,	80,	-40,
	D,	60,	-70,
	D,	50,	-90,
	D,	170,	0,
	M,	-170,	0,
	D,	-50,	-90,
	D,	-60,	-70,
	D,	-80,	-40,
	D,	-420,	0,
	D,	40,	80,
	D,	20,	120,
	D,	-20,	120,
	D,	-40,	80,
	M,	-270,	-280,
	1,	0,	0,
	M,	120,	-80,
	D,	0,	960,		/* FF	*/
	D,	640,	0,
	D,	0,	-960,
	D,	-640,	0,
	M,	0,	160,
	D,	-120,	0,
	M,	0,	320,
	D,	120,	0,
	M,	0,	320,
	D,	-120,	0,
	M,	760,	0,
	D,	120,	0,
	M,	0,	-640,
	D,	-120,	0,
	M,	-320,	-160,
	D,	0,	-120,
	M,	-320,	120,
	M,	60,	120,
	C,	0,	'K',
	M,	-77,	320,
	C,	0,	'C',
	C,	0,	'K',
	M,	-154,	320,
	C,	0,	'J',
	M,	183,	70,
	C,	0,	' ',	/* if have preset */
	M,	163,	-70,
	C,	0,	'Q',
	M,	-77,	-640,
	C,	0,	'Q',
	M,	-77,	110,
	D,	45,	0,
	M,	-308,	-200,
	C,	0,	'C',
	1,	0,	0,
	M,	0,	120,		/* tail	*/
	D,	60,	0,
	D,	0,	-120,
	D,	-30,	40,
	M,	30,	-40,
	D,	30,	40,
	M,	-30,	-40,
	D,	60,	0,
	M,	-120,	0,
	1,	0,	0,
	D,	60,	0,	/* lead		*/
	D,	0,	120,
	D,	-30,	-40,
	M,	30,	40,
	D,	30,	-40,
	M,	-30,	40,
	D,	60,	0,
	M,	-120,	-120,
	1,	0,	0,
	D,	0,	960,		/* FFP	*/
	D,	640,	0,
	D,	0,	-960,
	D,	-640,	0,
	M,	0,	160,
	D,	-120,	0,
	M,	0,	320,
	D,	120,	0,
	M,	0,	320,
	D,	-120,	0,
	M,	760,	0,
	D,	120,	0,
	M,	0,	-640,
	D,	-120,	0,
	M,	-320,	-160,
	D,	0,	-120,
	M,	-320,	120,
	M,	320,	960,
	D,	0,	120,
	M,	-320,	-1080,
	M,	50,	120,
	C,	0,	'K',
	M,	-77,	320,
	C,	0,	'C',
	C,	0,	'K',
	M,	-154,	320,
	C,	0,	'J',
	M,	183,	70,
	C,	0,	'P',
	M,	163,	-70,
	C,	0,	'Q',
	M,	-77,	-640,
	C,	0,	'Q',
	M,	-77,	110,
	D,	45,	0,
	M,	-328,	-200,
	C,	0,	'C',
	1,	0,	0,
	M,	0,	60,	/* hyst	*/
	D,	100,	0,
	D,	30,	-10,
	D,	30,	-40,
	D,	30,	-10,
	D,	50,	0,
	M,	-50,	0,
	D,	-50,	0,
	D,	-30,	10,
	D,	-30,	40,
	D,	-30,	10,
	M,	-50,	-60,
	1,	0,	0,
	M,	-20,	-35,
	D,	20,	20,	/* arrow	*/
	D,	0,	30,
	D,	-20,	20,
	D,	70,	-35,
	D,	-70,	-35,
	M,	20,	35,
	1,	0,	0,
	M,	-40,	-30,
	D,	40,	20,	/* feather	*/
	D,	0,	20,
	D,	-40,	20,
	M,	40,	-30,
	1,	0,	0,
	M,	0,	10,	/* bi		*/
	D,	10,	10,
	D,	60,	0,
	D,	10,	-10,
	D,	0,	-20,
	D,	-10,	-10,
	D,	-60,	0,
	D,	-10,	10,
	D,	0,	20,
	D,	80,	-20,
	M,	-80,	10,
	1,	0,	0,
	D,	0,	960,		/* D	*/
	D,	640,	0,
	D,	0,	-960,
	D,	-640,	0,
	M,	0,	160,
	D,	-120,	0,
	M,	0,	320,
	D,	120,	0,
	M,	0,	320,
	M,	-120,	0,
	M,	760,	0,
	D,	120,	0,
	M,	0,	-640,
	D,	-120,	0,
	M,	-320,	-160,
	D,	0,	-120,
	M,	-320,	120,
	M,	320,	960,
	D,	0,	120,
	M,	-320,	-1080,
	M,	50,	120,
	C,	0,	'D',
	M,	-77,	320,
	C,	0,	'C',
	C,	0,	'K',
	M,	-154,	320,
	C,	0,	' ',
	M,	183,	70,
	C,	0,	'P',
	M,	163,	-70,
	C,	0,	'Q',
	M,	-77,	-640,
	C,	0,	'Q',
	M,	-77,	110,
	D,	45,	0,
	M,	-328,	-200,
	C,	0,	'C',
	1,	0,	0,
	D,	70,	0,	/* res	*/
	D,	25,	60,
	D,	50,	-120,
	D,	50,	120,
	D,	50,	-120,
	D,	50,	120,
	D,	50,	-120,
	D,	25,	60,
	D,	70,	0,
	M,	-440,	0,
	1,	0,	0,
	D,	110,	0,	/* cap	*/
	M,	0,	-80,
	D,	0,	160,
	M,	60,	0,
	D,	0,	-160,
	M,	0,	80,
	D,	110,	0,
	M,	-290,	0,
	1,	0,	0,
	M,	-160,	-160,
	D,	320,	320,	/* var	*/
	D,	-40,	0,
	M,	40,	0,
	D,	0,	-40,
	M,	-160,	-120,
	1,	0,	0,
	D,	120,	0,	/* bat	*/
	M,	0,	-60,
	D,	0,	120,
	M,	40,	40,
	D,	0,	-200,
	M,	0,	100,
	D,	120,	0,
	M,	-280,	0,
	1,	0,	0,
	D,	160,	0,	/* npn	*/
	M,	0,	-80,
	D,	0,	160,
	M,	0,	-60,
	D,	160,	140,
	M,	-160,	-140,
	M,	0,	-40,
	D,	80,	-70,
	D,	0,	40,
	M,	0,	-40,
	D,	-40,	0,
	M,	40,	0,
	D,	80,	-70,
	M,	-320,	160,
	1,	0,	0,
	D,	160,	0,	/* pnp	*/
	M,	0,	-80,
	D,	0,	160,
	M,	0,	-60,
	D,	160,	140,
	M,	-160,	-140,
	M,	0,	-40,
	D,	80,	-70,
	D,	0,	-40,
	M,	0,	40,
	D,	40,	0,
	M,	-40,	0,
	D,	80,	-70,
	M,	-320,	160,
	1,	0,	0,
	D,	160,	0,	/* t	*/
	D,	40,	40,
	D,	0,	40,
	D,	-40,	40,
	D,	-40,	0,
	M,	40,	0,
	D,	40,	40,
	D,	0,	40,
	D,	-40,	40,
	D,	-40,	0,
	M,	40,	0,
	D,	40,	40,
	D,	0,	40,
	D,	-40,	40,
	D,	-40,	0,
	M,	40,	0,
	D,	40,	40,
	D,	0,	40,
	D,	-40,	40,
	D,	-160,	0,
	M,	230,	0,
	D,	0,	-480,
	M,	30,	0,
	D,	0,	480,
	M,	30,	0,
	D,	0,	-480,
	M,	230,	0,
	D,	-160,	0,
	D,	-40,	40,
	D,	0,	40,
	D,	40,	40,
	D,	40,	0,
	M,	-40,	0,
	D,	-40,	40,
	D,	0,	40,
	D,	40,	40,
	D,	40,	0,
	M,	-40,	0,
	D,	-40,	40,
	D,	0,	40,
	D,	40,	40,
	D,	40,	0,
	M,	-40,	0,
	D,	-40,	40,
	D,	0,	40,
	D,	40,	40,
	D,	160,	0,
	M,	-480,	-480,
	1,	0,	0,
	M,	0,	-40,
	D,	10,	40,	/* mu	*/
	D,	0,	40,
	M,	0,	-40,
	D,	10,	-10,
	D,	30,	0,
	D,	10,	10,
	D,	0,	40,
	M,	0,	-40,
	D,	10,	-20,
	M,	-70,	20,
	1,	0,	0,
	D,	40,	0,	/* ohm	*/
	D,	0,	10,
	D,	-20,	20,
	D,	0,	20,
	D,	30,	20,
	D,	20,	0,
	D,	20,	-20,
	D,	0,	-20,
	D,	-20,	-20,
	D,	0,	-10,
	D,	40,	0,
	M,	-110,	0,
	1,	0,	0,
	M,	100,	10,	/* sigma	*/
	D,	-10,	-10,
	D,	-90,	0,
	D,	0,	20,
	D,	30,	50,
	D,	-30,	50,
	D,	0,	20,
	D,	90,	0,
	D,	10,	-10,
	M,	-90,	-90,
	1,	0,	0,
	D,	10,	-10,	/* int	*/
	D,	10,	0,
	D,	10,	10,
	D,	20,	100,
	D,	10,	10,
	D,	10,	0,
	D,	10,	-10,
	M,	-80,	-80,
	1,	0,	0,
	M,	-20,	80,
	D,	-10,	-10,	/* four	*/
	D,	-10,	0,
	D,	-10,	10,
	D,	10,	10,
	D,	10,	0,
	D,	10,	0,
	D,	30,	-30,
	D,	-20,	-80,
	D,	10,	50,
	D,	50,	0,
	M,	-50,	0,
	M,	10,	30,
	D,	50,	0,
	D,	20,	20,
	M,	-80,	-80,
	1,	0,	0,
	D,	120,	0,	/* diode	*/
	M,	0,	-100,
	D,	0,	200,
	D,	80,	-100,
	D,	-80,	-100,
	M,	80,	0,
	D,	0,	200,
	M,	0,	-100,
	D,	120,	0,
	M,	-320,	0,
	1,	0,	0,
	D,	120,	0,	/* zenier	*/
	M,	0,	-100,
	D,	0,	200,
	D,	80,	-100,
	D,	-80,	-100,
	M,	80,	0,
	D,	-40,	-40,
	M,	40,	40,
	D,	0,	200,
	D,	40,	40,
	M,	-40,	-40,
	M,	0,	-100,
	D,	120,	0,
	M,	-320,	0,
	1,	0,	0,
	D,	0,	-120,	/* ground	*/
	M,	-100,	0,
	D,	200,	0,
	M,	-25,	-40,
	D,	-150,	0,
	M,	25,	-40,
	D,	100,	0,
	M,	-25,	-40,
	D,	-50,	0,
	M,	25,	240,
	1,	0,	0,
	D,	0,	-120,	/* earth	*/
	M,	-100,	0,
	D,	-50,	-40,
	M,	50,	40,
	D,	100,	0,
	D,	-50,	-40,
	M,	50,	40,
	D,	100,	0,
	D,	-50,	-40,
	M,	-60,	180,
	1,	0,	0,
	D,	10,	30,	/* photon	*/
	D,	20,	20,
	D,	30,	10,
	D,	20,	20,
	D,	30,	0,
	D,	20,	20,
	D,	10,	30,
	D,	20,	20,
	D,	30,	10,
	D,	-50,	0,
	M,	50,	0,
	D,	0,	-50,
	M,	220,	170,
	1,	0,	0,
	D,	85,	0,	/* fuse	*/
	D,	10,	40,
	D,	20,	20,
	D,	15,	0,
	D,	20,	-20,
	D,	10,	-40,
	D,	10,	-40,
	D,	20,	-20,
	D,	15,	0,
	D,	20,	20,
	D,	10,	40,
	D,	85,	0,
	M,	-320,	0,
	1,	0,	0,
	D,	0,	220,	/* light	*/
	D,	-60,	60,
	D,	0,	80,
	D,	60,	60,
	D,	100,	0,
	D,	60,	-60,
	D,	0,	-80,
	D,	-60,	-60,
	D,	0,	-220,
	D,	-30,	0,
	D,	-10,	-20,
	D,	-20,	0,
	D,	-10,	20,
	D,	-30,	0,
	D,	100,	30,
	M,	-100,	0,
	D,	100,	30,
	M,	-100,	0,
	D,	100,	30,
	M,	-100,	0,
	D,	100,	30,
	D,	-100,	0,
	M,	30,	0,
	D,	0,	160,
	D,	-40,	40,
	D,	20,	20,
	D,	10,	-10,
	D,	20,	20,
	D,	20,	-20,
	D,	20,	20,
	D,	10,	-10,
	D,	20 ,	-20,
	D,	-40,	-40,
	D,	0,	-160,
	M,	-70,	-120,
/* rays of light */
	M,	-80,	250,
	D,	-60,	-60,
	M,	60,	60,
	M,	-30,	70,
	D,	-60,	0,
	M,	60,	0,
	M,	30,	70,
	D,	-60,	60,
	M,	60,	-60,
	M,	130,	60,
	D,	0,	60,
	M,	0,	-60,
	M,	130,	-60,
	D,	60,	60,
	M,	-60,	-60,
	M,	30,	-70,
	D,	60,	0,
	M,	-60,	0,
	M,	-30,	-70,
	D,	60,	-60,
	M,	-60,	60,
	1,	0,	0,	/* coax	*/
	M,	0,	50,
	D,	40,	0,
	M,	20,	0,
	D,	40,	0,
	M,	20,	0,
	D,	40,	0,
	M,	0,	-100,
	D,	-40,	0,
	M,	-20,	0,
	D,	-20,	0,
	D,	0,	-60,
	M,	-30,	0,
	D,	60,	0,
	M,	-15,	-30,
	D,	-30,	0,
	M,	15,	90,
	D,	-20,	0,
	M,	-20,	0,
	D,	-40,	0,
	M,	0,	50,
	1,	0,	0,
	D,	0,	50,	/* var	*/
	D,	100,	0,
	M,	-100,	-50,
	1,	0,	0,
	M,	-85,	150,
	D,	170,	0,	/* triac	*/
	M,	0,	60,
	D,	-170,	0,
	M,	85,	0,
	D,	0,	150,
	M,	0,	-210,
	D,	0,	-150,
	M,	-65,	150,
	D,	30,	60,
	D,	30,	-60,
	M,	40,	0,
	D,	-30,	60,
	M,	60,	0,
	D,	-30,	-60,
	M,	-35,	-150,
	1,	0,	0,
	D,	160,	0,	/* coil	*/
	D,	40,	40,
	D,	0,	40,
	D,	-40,	40,
	D,	-50,	0,
	M,	50,	0,
	D,	40,	40,
	D,	0,	40,
	D,	-40,	40,
	D,	-50,	0,
	M,	50,	0,
	D,	40,	40,
	D,	0,	40,
	D,	-40,	40,
	D,	-50,	0,
	M,	50,	0,
	D,	40,	40,
	D,	0,	40,
	D,	-40,	40,
	D,	-160,	0,
	M,	0,	-480,
	1,	0,	0,
	D,	160,	0,	/* relay	*/
	D,	40,	40,
	D,	0,	40,
	D,	-40,	40,
	D,	-50,	0,
	M,	50,	0,
	D,	40,	40,
	D,	0,	40,
	D,	-40,	40,
	D,	-50,	0,
	M,	50,	0,
	D,	40,	40,
	D,	0,	40,
	D,	-40,	40,
	D,	-50,	0,
	M,	50,	0,
	D,	40,	40,
	D,	0,	40,
	D,	-40,	40,
	D,	-170,	0,
	M,	240,	0,
	D,	0,	-480,
	M,	30,	0,
	D,	0,	480,
	M,	30,	0,
	D,	0,	-480,
	M,	-300,	0,
	1,	0,	0,
	D,	0,	100,	/* ind	*/
	D,	50,	0,
	D,	40,	40,
	D,	0,	40,
	D,	-40,	40,
	D,	-50,	0,
	M,	50,	0,
	D,	40,	40,
	D,	0,	40,
	D,	-40,	40,
	D,	-50,	0,
	M,	50,	0,
	D,	40,	40,
	D,	0,	40,
	D,	-40,	40,
	D,	-50,	0,
	M,	50,	0,
	D,	40,	40,
	D,	0,	40,
	D,	-40,	40,
	D,	-50,	0,
	D,	0,	100,
	M,	0,	-680,
	1,	0,	0,
	M,	0,	30,	/* slug	*/
	D,	0,	60,
	M,	0,	60,
	D,	0,	60,
	M,	0,	60,
	D,	0,	60,
	M,	0,	60,
	D,	0,	60,
	M,	0,	60,
	D,	0,	60,
	M,	80,	0,
	D,	0,	-60,
	M,	0,	-60,
	D,	0,	-60,
	M,	0,	-60,
	D,	0,	-60,
	M,	0,	-60,
	D,	0,	-60,
	M,	0,	-60,
	D,	0,	-60,
	M,	-80,	-30,
	1,	0,	0,
	M,	20,	0,
	D,	40,	40,	/*	wiper	*/
	M,	-40,	-40,
	D,	40,	-40,
	M,	-40,	40,
	D,	150,	0,
	M,	-150,	0,
	1,	0,	0,
	M,	120,	-80,
	D,	0,	960,		/* 7473	*/
	D,	640,	0,
	D,	0,	-960,
	D,	-640,	0,
	M,	0,	180,
	D,	-160,	0,
	M,	0,	320,
	D,	80,	0,
	D,	27,	28,
	D,	27,	0,
	D,	26,	-28,
	D,	-26,	-28,
	D,	-27,	0,
	D,	-27,	28,
	M,	80,	0,
	M,	0,	300,
	D,	-120,	0,
	M,	760,	0,
	D,	120,	0,
	M,	0,	-640,
	D,	-120,	0,
	M,	-320,	-160,
	D,	28,	-26,
	D,	0,	-27,
	D,	-28,	-27,
	D,	-28,	27,
	D,	0,	27,
	D,	28,	26,
	M,	0,	-80,
	D,	0,	-80,
	M,	-320,	120,
	M,	60,	160,
	C,	0,	'K',
	M,	-77,	320,
	C,	0,	'C',
	C,	0,	'K',
	M,	-154,	320,
	C,	0,	'J',
	M,	183,	70,
	C,	0,	' ',	/* if have preset */
	M,	163,	-70,
	C,	0,	'Q',
	M,	-77,	-640,
	C,	0,	'Q',
	M,	-77,	110,
	D,	45,	0,
	M,	-308,	-200,
	C,	0,	'C',
	1,	0,	0,
	D,	0,	960,		/* 7474	*/
	D,	640,	0,
	D,	0,	-960,
	D,	-640,	0,
	M,	0,	160,
	D,	-120,	0,
	M,	0,	320,
	D,	120,	0,
	M,	0,	320,
	M,	-120,	0,
	M,	760,	0,
	D,	120,	0,
	M,	0,	-640,
	D,	-120,	0,
	M,	-320,	-160,
	D,	28,	-26,
	D,	0,	-27,
	D,	-28,	-27,
	D,	-28,	27,
	D,	0,	27,
	D,	28,	26,
	M,	0,	-80,
	D,	0,	-80,
	M,	-320,	120,
	M,	320,	1000,
	D,	-28,	26,
	D,	0,	27,
	D,	26,	27,
	D,	26,	-27,
	D,	0,	-27,
	D,	-28,	-26,
	M,	0,	80,
	D,	0,	80,
	M,	-320,	-1160,
	M,	50,	120,
	C,	0,	'D',
	M,	-77,	320,
	C,	0,	'C',
	C,	0,	'K',
	M,	-154,	320,
	C,	0,	' ',
	M,	183,	70,
	C,	0,	'P',
	M,	163,	-70,
	C,	0,	'Q',
	M,	-77,	-640,
	C,	0,	'Q',
	M,	-77,	110,
	D,	45,	0,
	M,	-328,	-200,
	C,	0,	'C',
	1,	0,	0,
	D,	0,	960,		/* 7476	*/
	D,	640,	0,
	D,	0,	-960,
	D,	-640,	0,
	M,	0,	160,
	D,	-120,	0,
	M,	0,	320,
	D,	80,	0,
	D,	27,	28,
	D,	27,	0,
	D,	26,	-28,
	D,	-26,	-28,
	D,	-27,	0,
	D,	-27,	28,
	M,	80,	0,
	M,	0,	320,
	D,	-120,	0,
	M,	760,	0,
	D,	120,	0,
	M,	0,	-640,
	D,	-120,	0,
	M,	-320,	-160,
	D,	28,	-26,
	D,	0,	-27,
	D,	-28,	-27,
	D,	-28,	27,
	D,	0,	27,
	D,	28,	26,
	M,	-80,	0,
	D,	-80,	0,
	M,	-320,	120,
	M,	320,	960,
	D,	-28,	26,
	D,	0,	27,
	D,	28,	27,
	D,	28,	-27,
	D,	0,	-27,
	D,	-28,	-26,
	M,	0,	80,
	D,	0,	80,
	M,	-320,	-1080,
	M,	50,	120,
	C,	0,	'K',
	M,	-77,	320,
	C,	0,	'C',
	C,	0,	'K',
	M,	-154,	320,
	C,	0,	'J',
	M,	183,	70,
	C,	0,	'P',
	M,	163,	-70,
	C,	0,	'Q',
	M,	-77,	-640,
	C,	0,	'Q',
	M,	-77,	110,
	D,	45,	0,
	M,	-328,	-200,
	C,	0,	'C',
	1,	0,	0,
	D,	0,	100,	/* ant	*/
	D,	-100,	80,
	D,	200,	0,
	D,	-100,	-80,
	D,	0,	80,
	M,	0,	-180,
	1,	0,	0,
	D,	80,	0,	/* crystal	*/
	M,	0,	-20,
	D,	0,	40,
	M,	40,	40,
	D,	0,	-120,
	D,	40,	0,
	D,	0,	120,
	D,	-40,	0,
	M,	80,	-40,
	D,	0,	-40,
	M,	0,	20,
	D,	80,	0,
	M,	-280,	0,
	1,	0,	0,
	M,	0,	-40,		/* bus		*/
	D,	240,	0,
	D,	0,	-40,
	D,	160,	80,
	D,	-160,	80,
	D,	0,	-40,
	D,	-240,	0,
	M,	0,	-40,
	1,	0,	0,
	M,	-10,	0,		/* mud		*/
	D,	10,	10,
	D,	10,	-10,
	D,	-10,	-10,
	D,	-10,	10,
	M,	10,	0,
	1,	0,	0,
	M,	0,	-13,	/* dot	*/
	D,	0,	26,
	D,	27,	27,
	D,	26,	0,
	D,	27,	-27,
	D,	0,	-26,
	D,	-27,	-27,
	D,	-26,	0,
	D,	-27,	27,
	M,	0,	13,
	1,	0,	0,
	M,	10,	0,	/*	ic6	*/
	D,	0,	20,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	20,
	D,	35,	0,
	D,	10,	-10,
	D,	10,	0,
	D,	10,	10,
	D,	35,	0,
	D,	0,	-20,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-20,
	D,	-100,	0,
	M,	-10,	0,
	1,	0,	0,
	M,	10,	0,	/*	ic8	*/
	D,	0,	20,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	20,
	D,	35,	0,
	D,	10,	-10,
	D,	10,	0,
	D,	10,	10,
	D,	35,	0,
	D,	0,	-20,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-20,
	D,	-100,	0,
	M,	-10,	0,
	1,	0,	0,
	M,	10,	0,	/*	ic14	*/
	D,	0,	20,
/* pup					*/
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	20,
	D,	35,	0,
	D,	10,	-10,
	D,	10,	0,
	D,	10,	10,
	D,	35,	0,
	D,	0,	-20,
/* pud					*/
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-20,
	D,	-100,	0,
	M,	-10,	0,
	1,	0,	0,
	M,	10,	0,	/*	ic16	*/
	D,	0,	20,
/* pup					*/
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	20,
	D,	35,	0,
	D,	10,	-10,
	D,	10,	0,
	D,	10,	10,
	D,	35,	0,
	D,	0,	-20,
/* pud					*/
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-20,
	D,	-100,	0,
	M,	-10,	0,
	1,	0,	0,
	M,	10,	0,	/*	ic18	*/
	D,	0,	20,
/* pup					*/
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	20,
	D,	35,	0,
	D,	10,	-10,
	D,	10,	0,
	D,	10,	10,
	D,	35,	0,
	D,	0,	-20,
/* pud					*/
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-20,
	D,	-100,	0,
	M,	-10,	0,
	1,	0,	0,
	M,	10,	0,	/*	ic24	*/
	D,	0,	20,
/* pup					*/
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	20,
	D,	95,	0,
	D,	10,	-10,
	D,	10,	0,
	D,	10,	10,
	D,	95,	0,
	D,	0,	-20,
/* pud					*/
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-20,
	D,	-220,	0,
	M,	-10,	0,
	1,	0,	0,
	M,	10,	0,	/*	ic28	*/
	D,	0,	20,
/* pup					*/
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	20,
	D,	95,	0,
	D,	10,	-10,
	D,	10,	0,
	D,	10,	10,
	D,	95,	0,
	D,	0,	-20,
/* pud					*/
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-20,
	D,	-220,	0,
	M,	-10,	0,
	1,	0,	0,
	M,	10,	0,	/*	ic40	*/
	D,	0,	20,
/* pup					*/
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	30,
	D,	-10,	0,
	D,	0,	-20,
	D,	10,	0,
	D,	0,	30,
	D,	0,	20,
	D,	95,	0,
	D,	10,	-10,
	D,	10,	0,
	D,	10,	10,
	D,	95,	0,
	D,	0,	-20,
/* pud					*/
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-30,
	D,	10,	0,
	D,	0,	20,
	D,	-10,	0,
	D,	0,	-30,
	D,	0,	-20,
	D,	-220,	0,
	M,	-10,	0,
	1,	0,	0,
	0,	0,	0,
	0,	0,	0,
	0,	0,	0,		/* end of list	*/
	};


psetup()
{

	pstat[HPBS] = plotob;
	pstat[DEBUG] = 1;
	plotsel(pstat,TEK);
	plotinit(pstat);
	pstat[DEBUG] = 1;
	lastep[0] = lastep[1] = 330;
	aposit = 7000;
	apositx = 160;
	crossterm = '\n';
	rotdir = '0';
	rotscale = '1';

	gtty(0,savebuf);	/* save original status	*/
	gtty(0,ttybuf);	/* play buffer		*/
	ttybuf[2] =& ~ECHO;	/* Echo off		*/
/*	ttybuf[2] =| RAW;	/* Raw mode on		*/

	plotpen(pstat,BLACK);
	place[0]=1000;place[1]=1000;

}
	extern	*zero,*dot,*dol;
getit()
{
	register char c;
	register char *p,*k;
again:	if(crossterm == ' '){
		c = 'p';
		goto wire;
		}
	c = getchar();
	if(c == 'a'){
		if(zero == dol){
			printf("No line to adjust\n");
			goto again;
			}
		p = getline(zero[*dot - 1]);
		k = command;
		while(*k++ = *p++);
		alcom();
		zero[*dot - 1] = putline(command);
		goto again;
		}
wire:	getcom(c);
	if(crossterm == 'n'){
		foobar();
		goto again;
		}
	plotcom();
	foobar();
}

getcom(cm)
	char cm;
{
	register x,y;			/* gets a command line	*/
	register char c;		/* and puts it in 'command' */
	char *p,*k;
	int from[3],to[3];


	if(cm == 'r'){
		ruler();
		foobar();
		getcom(getchar());
		return;
		}
	if(cm == 'g'){			/* make work grid	*/
		while(getchar() != '\n');
		gridit();
		foobar();
		getcom(getchar());
		return;
		}
	p = command;
	*p++ = cm;
	*p = '\0';

    switch(cm){
	case 'w':			/* draw a 'W'ire	*/
		eatnl();
		getcross(from);
		hicke(from);
morelin:	getcross(to);
		if(crossterm == 'a'){
			place[0] = from[0];	place[1] = from[1];
			align();
			from[0] = place[0];	from[1] = place[1];
			goto morelin;
			}
		lastep[0] = to[0]; lastep[1] = to[1];
		hicke(to);
		make(from,p);
		seke(&p);
		*p++ = ':';
		make(to,p);
		seke(&p);
		*p++ = '\n';
		break;

	case 'z':			/* draw a 'Z'arrallagram	*/
		eatnl();
		getcross(from);
		hicke(from);
moreli:	getcross(to);
		if(crossterm == 'a'){
			place[0] = from[0];	place[1] = from[1];
			align();
			from[0] = place[0];	from[1] = place[1];
			goto moreli;
			}
		lastep[0] = to[0]; lastep[1] = to[1];
		make(from,p);
		seke(&p);
		*p++ = ':';
		make(to,p);
		seke(&p);
		*p++ = '\n';
		break;
	case 'c':			/* put a charactor	*/
		c = getchar();
		while(getchar() != '\n');
		getcross(to);
		make(to,p);
		seke(&p);
		*p++ = '-';
		*p++ = c;
		*p++ = '\n';
		break;

	case 'b':			/* draw a 'B'ox		*/
		c = getchar();
		k = boxname;
		if((c < '0') || (c > '7')){
			rotdir = '0';
			*k++ = c;
			goto boxno;
			}
		   else {
			rotdir = c;
			}
		c = getchar();
		if((c < '0') || (c > ':')){
			rotscale = '1';
			*k++ = c;
			goto boxno;
			}
		    else {
			rotscale = c;
			}
boxno:		while((*k++ = getchar()) != '\n');
		--k;	*k = '\0';
		putchar(ESC);	putchar(SUB);
		restart();
		move(pstat,REFX,REFY);
		putbox(boxname);
		putchar('\n');
		getrc(to);
		make(to,p);
		seke(&p);
		*p++ = '-';
		*p++ = rotdir;
		*p++ = rotscale;
		k = boxname;
		while(*p++ = *k++);
		p--;	*p++ = '\n';
		break;

	case 's':			/* 'S'tring plot	*/
		c = getchar();
		x = c;
		k = boxname;
		if((c >= '0') && (c <= '4')){
			}
		    else {
			*k++ = c;	c = x = '1';
			}
		while((*k++ = getchar()) != '\n');
		--k;
		*k = '\0';
		restart();
		move(pstat,5130,3860);
		setchr(pstat,x);
		printf("%s\n",boxname);
		getrc(to);
		make(to,p);
		seke(&p);
		*p++ = '-';	*p++ = c;
		k = boxname;
		while(*p++ = *k++);
		p--;	*p++ = '\n';
		break;

	case 'p':			/* 'P'ath 		*/
		if(crossterm != ' ')eatnl();
		from[0] = lastep[0];
		from[1] = lastep[1];
		command[0] = 'w';
		goto morelin;
		break;

	case '.':			/* get out of input mode */
		command[0] = '\0';
		crossterm = '\n';
		eatnl();
		break;

	case 'v':			/* set vector type */
		if((c = getchar()) == '\n')*p++ = '`';
		else *p++ = c;
		if(c != '\n')eatnl();
		*p++ = '\n';
		break;

	case 't':			/* pen select	*/
		if((c = getchar()) == '\n')*p++ = '1';
		else *p++ = c;
		if(c != '\n')eatnl();
		*p++ = '\n';
		break;

	case '*':			/* comment	*/
		while((*p++ = getchar()) != '\n');
		break;

	}
	*p = '\0';
	goalpha(pstat);
	rotdir = '0';
	rotscale = '1';
	if(command[0] != 'w')crossterm = '\n';
}

plotcom()				/* plots 'command'	*/
{
	register char *p,c;
	register x,y;
	int x2,y2,savdeb;
	char *glub,temp[200],*k,**ss;
	char temp2[20];

	glub = command;	k = temp;
	while((*glub) && (*glub != '\n'))*k++ = *glub++;
	*k = '\0';

	glub = temp;
	c = *glub++;
	ss = &glub;

	savdeb = pstat[DEBUG];
	pstat[DEBUG] = 0;

    switch(c){

	case 'w':			/* draw 'W'ire		*/
		setup(ss);
		sscan(temp2,ss);
		x = basin(temp2,10);
		sscan(temp2,ss);
		y = basin(temp2,10);
		draw(pstat,x,y);
		break;

	case 'z':			/* draw a 'Z'arralagram	*/
		sscan(temp2,ss);
		x = basin(temp2,10);
		sscan(temp2,ss);
		y = basin(temp2,10);
		sscan(temp2,ss);
		x2 = basin(temp2,10);
		sscan(temp2,ss);
		y2 = basin(temp2,10);
		move(pstat,x,y);
		draw(pstat,x,y2);
		draw(pstat,x2,y2);
		draw(pstat,x2,y);
		draw(pstat,x,y);
		break;

	case 'c':			/* plot 'C'haractor	*/
		setup(ss);
		(*ss)++;
		plotc(pstat,**ss);
		break;

	case 'b':			/* plot 'B'ox		*/
		setup(ss);
		p = boxname;
		k = (*ss)++;
		k++;
		rotdir = *k++;
		rotscale = *k++;
		while(*p++ = *k++);
		putbox(boxname);
		rotdir = '0';
		rotscale = '1';
		break;

	case 's':			/* plot 'S'tring	*/
		setup(ss);
		(*ss)++;
		c = *(*ss)++;
		setchr(pstat,c);
		plots(pstat,*ss);
		setchr(pstat,'1');
		break;

	case 'v':				/* set vector type */
		setvec(pstat,**ss);
		break;

	case 't':			/* pen select	*/
		plotpen(pstat,**ss);
		break;

	case '\n':			/* blank command	*/
		break;

	case '*':			/* comment	*/
		break;

	}
	pstat[DEBUG] = savdeb;
	goalpha(pstat);
}

putbox(name)
	char *name;
{
	register i,*ii;
	int denote,x,y;

	denote = pstat[DEBUG];
	pstat[DEBUG] = 0;

	i = llu(name,boxkind);
	if(i == -1){
		putchar('\n');
		reclear();
		foobar();
		goalpha(pstat);
		pstat[DEBUG] = denote;
		printf("Unknown symbol- '%s'.\n",name);
		return;
		}

	i--;
	ii = boxdata;
	while(i && *ii){
		while((*ii != 1) && *ii)ii =+ 3;
		ii =+ 3;
		i--;
		}
	if(*ii == 0){
		goalpha(pstat);
		pstat[DEBUG] = denote;
		printf("Symbol not in table- '%s'.\n",name);
		return;
		}
	while((*ii != 1) && *ii){
	    switch(*ii++){
		case 2:					/* move	*/
			x = *ii++;
			y = *ii++;
			rotate(&x,&y);
			mover(pstat,x,y);
			break;
		case 3:					/* draw	*/
			x = *ii++;	y = *ii++;
			rotate(&x,&y);
			drawr(pstat,x,y);
			break;
		case 4:
			x = *ii++;	y = *ii++;
			goalpha(pstat);
			plotc(pstat,y);
			break;
		}
	}
	pstat[DEBUG] = denote;
}


restart()
{
	putchar(ESC);
	printf("=1");
}
reend()
{	putchar('\n');
}
reclear()
{
	putchar(ESC);	printf("=0");
	setchr(pstat,'1');
}
restore()
{
	putchar(ESC);	printf("=3");
	getchar(0);		/* flush line terminator	*/
}
setup(ss)
	int ss;
{
	register x,y;
	char temp[20];

	sscan(temp,ss);
	x = basin(temp,10);
	sscan(temp,ss);
	y = basin(temp,10);
	move(pstat,x,y);
}
make(loc,p)				/* put x,y locations into  */
	int *loc;	char *p;	/* into string		*/
{
	basesp(loc[0],p,10);
	seke(&p);
	*p++ = ',';
	basesp(loc[1],p,10);
}
seke(p)					/* seek 'p' to end of string */
	int **p;
{
	char *q;

	q = *p;			/* have *char	*/
	while(*q != '\0')q++;		/* find end of string	*/
	*p = q;			/* store it in pointer	*/
}
hicke(loc)				/* put a hicke at current */
	int loc[2];			/* location on paper	*/
{
	move(pstat,loc[0] - 30,loc[1]);
	drawr(pstat,30,30);
	drawr(pstat,30,-30);
	drawr(pstat,-30,-30);
	drawr(pstat,-30,30);
	mover(pstat,30,0);
}

getcross(place)				/* read in the crosshairs */
	int *place;
{
	register x,i;
	register char c;

	stty(0,ttybuf);	/* go no echo	*/
	putchar(ESC);	putchar(SUB);
	crossterm = getchar();
	place[0] = getcrxx();
	place[1] = getcrxx();
	getchar();		/* eat up \n terminator	*/
	stty(0,savebuf);	/* go full dup	*/
	putchar(7);
}
getcrxx()
{
/* Note that the resolution of the crosshairs is 512 lines/page.
  This is to aid with allignment. 				*/
	register x,temp;

	x = ((getchar() & 037) << 5);
	temp = getchar() & 036;
	if(temp & 2)temp =+ 2;
	x =+ temp;
	return(x * 10);
}
getrc(place)				/* read in crosshairs */
	int *place;			/* with refresh mem.  */
{					/* running	    */
	int i;
	register char c;
	char pos;		/* user must prompt with n */
	stty(0,ttybuf);	/* go no echo	*/
	crossterm = '\n';
/* Wait for request for crosshair postion read		*/
	while((c = getchar()) != '\n'){
		if(c == 'n')crossterm = c;
		}
	reclear();
	putchar(ESC);	putchar(SUB);
	for(i = 0;i != 19;i++)putchar(1);
	putchar(ESC);	putchar(ENQ);	/* request cursor position */
	place[0] = getcrjs();
	place[1] = getcrjs();
	place[0] =- (REFX/2);
	place[1] =- (REFY/2);
	place[0] =* 2;
	place[1] =* 2;
	place[1] =/ 10;		place[0] =/ 10;
	place[1] =& 037776;	place[0] =& 037776;
	if(place[0] & 2)place[0] =+ 2;
	if(place[1] & 2)place[1] =+ 2;
	place[0] =* 10;		place[1] =* 10;
	place[0] =+ 80; place[1] =+ 40;	/* funny hardware corections */
	if((pos = getchar()) != '\n'){reclear();printf("I am out of sync'%c'.\n",pos);}
	stty(0,savebuf);	/* go with echo	*/
	reclear();
}
getcrjs()
{
/* Note that the resolution of the joystick is 256 lines/page.
  This is to aid with allignment and correct for joystick drift. */
	register x,temp;

	x = ((getchar() & 037) << 5);
	temp = getchar() & 037;
	x =+ temp;
	return(x * 10);
}
foobar()
{
	move(pstat,apositx,aposit =- 130);
	if(aposit < 0){
		aposit = 7900;
		apositx =+ 2500;
		}
}
rotate(x,y)
	int *x,*y;
{
	register xt,yt,fac;
	char temp;

	temp = rotdir;
	if(rotdir > '3'){
		*y = -*y;
		temp =- 4;
		}
    switch(temp){
	case '0':					/* no rotate	*/
		xt = *x;	yt = *y;
		break;
	case '1':					/* -90 deg.	*/
		xt = *y;
		yt = -*x;
		break;
	case '2':					/* 180 deg.	*/
		xt = -*x;
		yt = *y;
		break;
	case '3':					/* +90 deg.	*/
		xt = -*y;
		yt = *x;
		break;
	}
	if(rotscale == '0'){
		*x = xt / 2;
		*y = yt / 2;
		}
	    else {
		fac = rotscale - '0';
		*x = xt * fac;
		*y = yt * fac;
		}
}
ruler()
{
	register n,x,y;
	int x1,y1;
	int to[3],linit;
	char c,temp[22];

	linit = 0;
	c = getchar();
	if(c == 'l'){
		linit = 1;
		c = getchar();
		}
	if(c == 'i'){
		linit = 2;
		c = getchar();
		}
	if(c == 'r')goto rulread;
	while(getchar() != '\n');
	n = c - '0';
/* n has # of points	*/
	if(n < 2)return;
	n--;
	getcross(to);
	move(pstat,to[0],to[1]);
	hickx();
	x1 = to[0];	y1 = to[1];
	getcross(to);
	x = to[0] - x1;	y = to[1] - y1;
	move(pstat,x1,y1);
	if(linit == 2){
		x = x/n;	y = y/n;
		}
	while(n--){
		if(linit == 1)
			drawr(pstat,x,y);
		    else
			mover(pstat,x,y);
		hickx();
		}
	move(pstat,x1,y1);
	crossterm = '\n';
	return;
rulread:
	scanin(temp);	x1 = basin(temp,10);
	scanin(temp);	y1 = basin(temp,10);
	getcross(to);
	move(pstat,to[0] + x1,to[1] + y1);
	hickx();
	crossterm = '\n';
}
eatnl()
{
	while(getchar() != '\n');
}
hickx()
{
	drawr(pstat,20,20);
	drawr(pstat,-40,-40);
	mover(pstat,0,40);
	drawr(pstat,40,-40);
	mover(pstat,-20,20);
}
scanin(world)
	char *world;
{
	register char *p,c;
	register i;
	p = world;
	*p = '\0';

	while((island(c = getchar())!= 1)&&(c!= '\n')&&(c != '\0')&&(c != ';'));

	if(c == '\n') return(-1);
	if(c == '\0') return(-2);
	if(c == ';')return(c);
	*p++ = c;
	i = 1;
	while(island(*p++ = getchar()))
			if(++i > 20) --p;
		if(*(--p) == '\0') return(-2);
		if(*p == '\n'){
			*p = '\0';
			return(-1);
			}
	else {i = *p;	*p = '\0';}
	return(i);
}
sscan(world,string)
	char *world,**string;
{
	register char *p,**c;
	register i;
	p = world;
	*p = '\0';
	c = string;

	while((island(**c)!= 1)&&(**c!= '\n')&&(**c != '\0')&&(**c != ';'))(*c)++;

	if(**c == '\n') return(-1);
	if(**c == '\0') return(-2);
	if(**c == ';')return(**c);
	*p++ = *(*c)++;
	i = 1;
	while(island(*p++ = *(*c)++))
			if(++i > 20) --p;
	(*c)--;
		if(*(--p) == '\0') return(-2);
		if(*p == '\n'){
			*p = '\0';
			return(-1);
			}
	else {i = *p;	*p = '\0';}
	return(i);
}
basin(string,base)
	char *string;
	int base;
{
	char s[21];
	register number;
	register p;
	int i;
	register exponent;
	for(i=0;(s[i]= string[i])!='\0';i++);
	for(p=0;s[p]!='\0';p++);
	for(i=0;(s[i]!='\0');i++){
		if((s[i] == 'o') || (s[i] == 'O'))s[i] = '0';
		if((s[i] >= 'A') && (s[i] <= 'Z'))s[i] =+ 32;
		if(s[i] > '9')s[i]=
			(s[i]-('a'-':'));
		}
	number=0;exponent=1;p=p-1;
		while(p!=(-1)){
			number=number+(((s[p--])-48)*exponent);
			exponent=exponent*base;}
	return(number);
}
llu(world,keylist)		/*   List Look Up	*/
	char *world,*keylist[];
{
	register char *p,*m;
	register i;

	i = 0;				/*	point to first keylist			*/
	while(keylist[i] != -1){	/*	and if not all done with them:	*/
		p = world;		/*	point to input string		*/
		m = keylist[i++];	/*	point to current keylist string	*/

		while((*p++ == *m++)&&(*(p-1)));	/* eat up equal char.	*/

		if((*--p == '\0')&&(*--m == '\0')) return(--i);
	}
	return(-1);
}
basesp(n,t,base)
	int n,base;
	char *t;
{
int p[10];
register i;
register char *j;
j = t;
p[0]= 0;
for(i=0;n != 0;i++){
	p[i] = lrem(0,n,base);
	n = ldiv(0,n,base);
	}
if(i!=0)--i;	/* remove this line if allways want leading 0 */
for(;i >= 0;i--)*j++ = (((p[i] > 9)?(p[i]+87):(p[i]+'0')));
*j = '\0';
}
island(ch)
	char ch;
{
	if(((ch >= 'A')&&(ch <= 'Z'))||((ch >= 'a')&&(ch <= 'z'))) return (1);
	if((ch >= '0')&&(ch <= '9')) return(1);
	if(ch == '_')return(1);
	return(0);
}
islet(ch)
	char ch;
{
	if(((ch >= 'A')&&(ch <= 'Z'))||((ch >= 'a')&&(ch <= 'z'))) return (1);
	if(ch == '_')return(1);
	if((ch >= '0')&&(ch <= '9')) return(0);
	if((ch >= '!')&&(ch <= '/')) return(2);
	if((ch >= ':')&&(ch <= '?')) return(2);
	if((ch == '`')||(ch == '@')) return(2);
	if((ch >= '[')&&(ch <= '_')) return(2);
	if((ch >= '{')&&(ch <= '~')) return(2);
	return(3);
}
/* align and x,y co-ordinate.  CR terminates the allign-
   ment process, ' ' continues it, and 'v'  views  the
   current position.  Crosshairs are positioned in the
   direction of point movment.  Only x OR y can be changed
   (there is no 45 degree movement allowed).		*/
align()
{					/* place passes args */
	register dx,dy,i;
	int tx,ty,x,y,to[3];

	crossterm = ' ';
	x = place[0];	y = place[1];
	move(pstat,x-60,y);
	drawr(pstat,-60,0);
	mover(pstat,120,60);
	drawr(pstat,0,60);
	mover(pstat,60,-120);
	drawr(pstat,60,0);
	mover(pstat,-120,-60);
	drawr(pstat,0,-60);
	mover(pstat,0,120);
    while(1){
	getcross(to);
	if(crossterm == '\n'){
		place[0] = x;	place[1] = y;
		return;
		}
	if(crossterm == 'v'){
		goto alignv;
		}
	dx = to[0] - x;
	dy = to[1] - y;
	tx = dx;	if(tx < 0)tx = -tx;
	ty = dy;	if(ty < 0)ty = -ty;
	if(tx > ty){
		if(dx > 0)x =+ 10;
		    else x=- 10;
		}
	    else{
		if(dy > 0)y=+ 10;
		    else y=- 10;
		}
alignv:	point(pstat,x,y);
	}
}
/*	Read x & y from a command string pointed to by
	**ss type.  Return x & y in 'place' (global).
	*ss is modifed and left at end of the command. */
getxy(ss)
	char **ss;
{
	char temp[20];
	sscan(temp,ss);	/* read x co-ord	*/
	place[0] = basin(temp,10);
	sscan(temp,ss);	/* read y co-ord	*/
	place[1] = basin(temp,10);
}
/*	Take 'command' and align the last data point
	in it, leaving a new command in 'command'.	*/
alcom()
{
	register char *p,*k,c;
	char *glub,**ss,*n,ncomm[200];

	glub = command;	n = ncomm;
	c = *glub++;	*n++ = c;
	ss = &glub;
	if(c == 'w'){
		while((*n++ = *(*ss)++) != ':');
		}
	getxy(ss);
	align();
	make(place,n);
	seke(&n);
	if(c == 'w'){
		*n++ = '\n';
		*n++ = '\0';
		goto alcomf;
		}
	*n++ = '-';
	while(*n++ = *(*ss)++);
alcomf:	p = command;	k = ncomm;
	while(*p++ = *k++);
}
gridit(){
	register x,y,n;
	int nn,sx,sy;
	int to[3];

	getcross(to);
	x = to[0]-160;	y = to[1]-160;

	sx = x;
	for(n = 0;n != 9;n++){
		x = sx;
		for(nn = 0;nn != 9;nn++){
			point(pstat,x,y);
			x =+ 40;
			}
		y =+ 40;
		}
}
