/* :ts=8 bk=0
 *
 * robotroff.h		Various defines.
 *
 * Leo L. Schwab			8703.16
 */

#define	PORTNAME		"Robotroff Control Port"
#define	CMDSIG			(1L << cmdport -> mp_SigBit)
#define	TIMESIG			(1L << timeport -> mp_SigBit)
#define	DEFAULTHIGHWATER	120
#define DEFAULTLOWATER		30

#define	HULKHEIGHT		16
#define GRUNTHEIGHT		13
#define	NFORCEHEIGHT		13
#define	MINX			-20
#define	MINY			-20

#define	COLLISION		0x0200

#define	collision()		(cstm -> clxdat & COLLISION)
#define getcbase(sn)		(((sn) >> 1 << 2) + 16)
#define getr(c)			(((c) & 0x0f00) >> 8)
#define getg(c)			(((c) & 0x00f0) >> 4)
#define getb(c)			((c) & 0x000f)
#define	abs(n)			((n) < 0 ? -(n) : (n))
#define	sgn(n)			((n) < 0 ? -1 : (n) > 0)

struct control {	/*  Control block to alter operating parameters  */
	struct Message msg;
	UWORD actions;
	int highwater, lowater, newpri;
};
#define	SETHIGH			1
#define	SETLOW			(1<<1)
#define	SETPRI			(1<<2)
#define	DOSOMETHING		(1<<3)
#define	SUICIDE			(1<<4)

extern void	*OpenLibrary(), *OpenWindow(), *ViewPortAddress(),
		*CreateStdIO(), *CreateExtIO(), *CreatePort(), *FindPort(),
		*GetMsg(), *FindTask();
extern long	Wait(), GetSprite(), VBeamPos(), OpenDevice();
extern short	rnd();
