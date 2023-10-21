431c
	fout = creat("c.c", 0644);
.
387c
	fout = creat("l.s", 0644);
.
278a
/*
 * DV11 & modem
 * LK11A
 * DWUN
 * DMC11
 */

	"dz",
	0,	308,	CHAR+INTR,
	"\tdzin; br5+%d.\n\tdzou; br5+%d.\n",
	".globl\t_dzrint\ndzin:\tjsr\tr0,call; _dzrint\n",
	".globl\t_dzxint\ndzou:\tjsr\tr0,call; _dzxint\n",
	"",
	"\t&dzopen,   &dzclose,  &dzread,   &dzwrite,  &dzsgtty,",

/*
 * DWR70
 * LPP11
 * VMV21
 * VMV31
 * VTV01
 * KMC11
 * RL11/RLV11
 * RX02
 * TS11
 * LPA11K
 * IP11/IP300
 */

	0
};

.
276,277c
	"dup",
	0,	308,	CHAR+INTR,
	"\tduin; br6+%d.\n\tduou; br6+%d.\n",
	".globl\t_durint\nduin:\tjsr\tr0,call; _durint\n",
	".globl\t_duxint\nduou:\tjsr\tr0,call; _duxint\n",
	"",
	"\t&duopen,   &duclose,  &duread,   &duwrite,  &nodev,",
.
272,273c
 * LPS
 * DQ11
 * KW11W
 * DU11
.
242,247c
 * DR11-A
 * DR11-C
 * PA611 (reader)
 * PA611 (punch)
 * LPD11
 * DT11
 * DX11
.
225,227c
	"dn",		/* one vector per 4 devices */
	0,	308,	CHAR+INTR,	/* modified from 304 */
	"\tdnou; br4+%d.\n\tdnou; br4+%d.\n",
.
196a
/* Floating Vector Devices: */

.
194c
 * 264 RX11
 * 270 LP/LS/LV11 #3
 * 274 LP/LS/LV11 #4
.
173c
 * 234 UDC11, ICS/ICR11, IP11/IP300
 * 240 PIRQ
 * 244 FPP/FIS exception
 * 250 KT error
.
129c
 * 210 RC, RK611/RK711, LP20 #2
.
101c
 * 150 RH11/RH70 (alt RM02/03)
 * 154 SQUID
 * 160-164 unused
 * 170 LP/LS/LV11 #1
 * 174 LP/LS/LV11 #2
.
93,95c
 * 120 XY11
.
86c
	"\ttrap; br7+10.\t\t/ memory parity\n",
.
83a
/*
 * 104 KW11P
 * 110 unused
 */

.
36a
	"dz",		/* DZ is major device # 16 */
	"dup",		/* DUP is major device # 17 */
.
0a
#
/* mkconf.c - makes configuration files c.c and l.s from parts list
 *
 *	modified 10-Jun-1980 by D A Gwyn:
 *	1) DUP11, DZ11 support added;
 *	2) DN11 vector size changed;
 *	3) memory parity now trap #10.
 */

.
w
q
