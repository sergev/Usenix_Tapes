#define D 3
#define M 2
#define C 4
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
	M,	50,	160,
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
	D,	80,	0,
	D,	0,	-40,
	D,	160,	80,
	D,	-160,	80,
	D,	0,	-40,
	D,	-80,	0,
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
	D,	40,	0,	/* hop	*/
	D,	10,	30,
	D,	10,	10,
	D,	40,	0,
	D,	10,	-10,
	D,	10,	-30,
	D,	40,	0,
	M,	-160,	0,
	1,	0,	0,
	D,	0,	1360,		/* title	*/
	D,	2800,	0,
	D,	0,	-1360,
	D,	-2800,	0,
	M,	0,	360,
	D,	1400,	0,
	M,	0,	-360,
	D,	0,	720,
	M,	-1400,	0,
	D,	2800,	0,
	M,	0,	20,
	D,	-2800,	0,
	M,	2800,	-260,
	D,	-1380,	0,
	M,	0,	-240,
	D,	1380,	0,
	M,	-1380,	-240,
	D,	0,	720,
	M,	-1420,	-720,
	M,	2100,	0,
	D,	0,	480,
	M,	-2100,	-480,
	1,	0,	0,
	D,	10,	40,		/* circle	*/
	D,	30,	30,
	D,	40,	10,
	D,	40,	-10,
	D,	30,	-30,
	D,	10,	-40,
	D,	-10,	-40,
	D,	-30,	-30,
	D,	-40,	-10,
	D,	-40,	10,
	D,	-30,	30,
	D,	-10,	40,
	1,	0,	0,
	D,	80,	80,		/* diamond */
	D,	80,	-80,
	D,	-80,	-80,
	D,	-80,	80,
	1,	0,	0,
	D,	0,	320,		/* fet */
	M,	0,	-80,
	D,	160,	0,
	D,	0,	160,
	M,	0,	-480,
	D,	0,	160,
	D,	-160,	0,
	M,	0,	-80,
	1,	0,	0,
	D,	40,	0,		/* fetenh */
	D,	0,	240,
	M,	80,	40,
	D,	0,	-80,
	M,	0,	40,
	D,	160,	0,
	D,	0,	160,
	M,	0,	-280,
	D,	0,	-280,
	M,	0,	160,
	D,	-160,	0,
	D,	0,	40,
	D,	0,	-80,
	M,	0,	120,
	D,	0,	80,
	D,	0,	-40,
	D,	160,	0,
	M,	-280,	-120,
	1,	0,	0,
	0,	0,	0,
	0,	0,	0,
	0,	0,	0,		/* end of list	*/
	};

