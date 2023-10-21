/*	libmac.h	1.0	84/05/01	*/

/*
 * C library linkage for Macintosh.
 *
 * Copyright (C) 1984, Stanford Univ. SUMEX project.
 * May be used but not sold without permission.
 */

/*
 * history
 * 05/01/84	Croft	Created.
 * 05/12/84	Croft	traps/calls now packed in 3bit nibbles.
 * 05/31/84	Croft	added VS/VPS
 */


#define	N	0	/* null */
#define	S	1	/* short */
#define	L	2	/* long */
#define	PS	3	/* pascal string */
#define	SS	4	/* short structure (e.g. Point) */
#define	TS	4	/* 4 byte type string (resource,creator,filetype) */
#define	B	5	/* boolean */
#define	VS	6	/* VAR short */
#define	VPS	7	/* VAR pascal string */

#define	RN	;	/* return null */
#define	RI	return(0); /* return integer */
#define	R(v)	return(v); /* return value */

#ifdef	CODE	/* defined if compiling code library */

#define	_sh(r,a1,a2,a3,a4,a5,a6,a7,a8,a9) \
	(a9<<27 | a8<<24 | a7<<21 | a6<<18 | a5<<15 \
	| a4<<12 | a3<<9 | a2<<6 | a1<<3 | r)

#define	_mact1(lr,t,r,a1) _mactrap((t<<16) | 0x4e71, \
	_sh(r,a1,0,0,0,0,0,0,0,0));
#define	_mact2(lr,t,r,a1,a2) _mactrap((t<<16) | 0x4e71, \
	_sh(r,a1,a2,0,0,0,0,0,0,0));
#define	_mact3(lr,t,r,a1,a2,a3) _mactrap((t<<16) | 0x4e71, \
	_sh(r,a1,a2,a3,0,0,0,0,0,0));
#define	_mact4(lr,t,r,a1,a2,a3,a4) _mactrap((t<<16) | 0x4e71, \
	_sh(r,a1,a2,a3,a4,0,0,0,0,0));
#define	_mact5(lr,t,r,a1,a2,a3,a4,a5) _mactrap((t<<16) | 0x4e71, \
	_sh(r,a1,a2,a3,a4,a5,0,0,0,0));
#define	_mact6(lr,t,r,a1,a2,a3,a4,a5,a6) _mactrap((t<<16) | 0x4e71, \
	_sh(r,a1,a2,a3,a4,a5,a6,0,0,0));
#define	_mact7(lr,t,r,a1,a2,a3,a4,a5,a6,a7) _mactrap((t<<16) | 0x4e71, \
	_sh(r,a1,a2,a3,a4,a5,a6,a7,0,0));
#define	_mact8(lr,t,r,a1,a2,a3,a4,a5,a6,a7,a8) _mactrap((t<<16) | 0x4e71, \
	_sh(r,a1,a2,a3,a4,a5,a6,a7,a8,0));
#define	_mact9(lr,t,r,a1,a2,a3,a4,a5,a6,a7,a8,a9) _mactrap((t<<16) | 0x4e71, \
	_sh(r,a1,a2,a3,a4,a5,a6,a7,a8,a9));
#define	_macc1(lr,t,r,a1) extern t(); _mactrap(t, \
	_sh(r,a1,0,0,0,0,0,0,0,0));
#define	_macc2(lr,t,r,a1,a2) extern t(); _mactrap(t, \
	_sh(r,a1,a2,0,0,0,0,0,0,0));
#define	_macc3(lr,t,r,a1,a2,a3) extern t(); _mactrap(t, \
	_sh(r,a1,a2,a3,0,0,0,0,0,0));
#define	_macc4(lr,t,r,a1,a2,a3,a4) extern t(); _mactrap(t, \
	_sh(r,a1,a2,a3,a4,0,0,0,0,0));
#define	_macc5(lr,t,r,a1,a2,a3,a4,a5) extern t(); _mactrap(t, \
	_sh(r,a1,a2,a3,a4,a5,0,0,0,0));
#define	_macc6(lr,t,r,a1,a2,a3,a4,a5,a6) extern t(); _mactrap(t, \
	_sh(r,a1,a2,a3,a4,a5,a6,0,0,0));
#define	_macc7(lr,t,r,a1,a2,a3,a4,a5,a6,a7) extern t(); _mactrap(t, \
	_sh(r,a1,a2,a3,a4,a5,a6,a7,0,0));
#define	_macc8(lr,t,r,a1,a2,a3,a4,a5,a6,a7,a8) extern t(); _mactrap(t, \
	_sh(r,a1,a2,a3,a4,a5,a6,a7,a8,0));
#define	_macc9(lr,t,r,a1,a2,a3,a4,a5,a6,a7,a8,a9) extern t(); _mactrap(t, \
	_sh(r,a1,a2,a3,a4,a5,a6,a7,a8,a9));
#define	_macs(lr)

#else		/* else compiling lint library */

#define	_mact1(lr,t,r,a1) lr
#define	_mact2(lr,t,r,a1,a2) lr
#define	_mact3(lr,t,r,a1,a2,a3) lr
#define	_mact4(lr,t,r,a1,a2,a3,a4) lr
#define	_mact5(lr,t,r,a1,a2,a3,a4,a5) lr
#define	_mact6(lr,t,r,a1,a2,a3,a4,a5,a6) lr
#define	_mact7(lr,t,r,a1,a2,a3,a4,a5,a6,a7) lr
#define	_mact8(lr,t,r,a1,a2,a3,a4,a5,a6,a7,a8) lr
#define	_mact9(lr,t,r,a1,a2,a3,a4,a5,a6,a7,a8,a9) lr
#define	_macc1(lr,t,r,a1) lr
#define	_macc2(lr,t,r,a1,a2) lr
#define	_macc3(lr,t,r,a1,a2,a3) lr
#define	_macc4(lr,t,r,a1,a2,a3,a4) lr
#define	_macc5(lr,t,r,a1,a2,a3,a4,a5) lr
#define	_macc6(lr,t,r,a1,a2,a3,a4,a5,a6) lr
#define	_macc7(lr,t,r,a1,a2,a3,a4,a5,a6,a7) lr
#define	_macc8(lr,t,r,a1,a2,a3,a4,a5,a6,a7,a8) lr
#define	_macc9(lr,t,r,a1,a2,a3,a4,a5,a6,a7,a8,a9) lr
#define	_macs(lr) lr

#endif	CODE

#define	void	int		/* otherwise we would pull in everything */
