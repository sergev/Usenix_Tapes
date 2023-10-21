/*

The routines in this file provide support for the Atari ST520 or 1040
using VT52 emulation.  The I/O services are provided by routines in
"termio.c".  It compiles into nothing if not a ST520 style device.  The
bell on the ST520 is terrible, so the "beep" routine is conditionalized
on defining BEL. 

*/

#define	termdef	1			/* don't define "term" external */

#include        <stdio.h>
#include        "estruct.h"
#include	"edef.h"

#if MEGAMAX
overlay "st520"
#endif

#if     ATARI & ST520 & MEGAMAX
#include	<osbind.h>
#include	<ctype.h>

#define LINEA_INIT 0xA000
#define V_CEL_WR   -0x28
#define V_CEL_MY   -0x2a
#define V_CEL_HT   -0x2e
#define V_FNT_AD   -0x16
#define V_OFF_AD   -0x0a
#define V_DISAB    -346

#define NROW    24                      /* Screen size.                 */
#define NCOL    80                      /* Edit if you want to.         */
#define	MARGIN	8			/* size of minimim margin and	*/
#define	SCRSIZ	64			/* scroll size for extended lines */
#define	NPAUSE	25			/* # times thru update to pause */
#define BIAS    0x20                    /* Origin 0 coordinate bias.    */
#define ESC     0x1B                    /* ESC character.               */
#define BEL     0x07                    /* ascii bell character         */

extern  int     ttopen();               /* Forward references.          */
extern  int     ttgetc();
extern  int     ttputc();
extern  int     ttflush();
extern  int     ttclose();
extern  int     st520move();
extern  int     st520eeol();
extern  int     st520eeop();
extern  int     st520beep();
extern  int     st520open();
extern	int	st520close();
extern	int	st520rev();
extern	int	st520cres();
extern  int st520kopen();
extern  int st520kclose();

#if	COLOR
extern	int	st520fcol();
extern	int	st520bcol();

int		cfcolor = -1;		/* current fg (character) color */
int		cbcolor = -1;		/* current bg color */
int		oldpal[8];		/* pallette when emacs was invoked */
int		newpal[8] = {		/* default emacs pallette */
	0x000, 0x700, 0x070, 0x770, 0x007, 0x707, 0x077, 0x777};
#endif

int STncolors = 0;		/* number of colors  */
int STrez;			/* physical screen resolution */	

/*
 * Dispatch table. All the
 * hard fields just point into the
 * terminal I/O code.
 */
TERM    term    = {
        NROW-1,
        NCOL,
	MARGIN,
	MARGIN,
	SCRSIZ,
	NPAUSE,
        &st520open,
        &st520close,
	&st520kopen,
	&st520kclose,
        &ttgetc,
        &ttputc,
        &ttflush,
        &st520move,
        &st520eeol,
        &st520eeop,
        &st520beep,
        &st520rev,
        &st520cres
#if	COLOR
	, &st520fcol,
	&st520bcol
#endif
};
	struct KBDvecs {
		int (*midivec) ();
		int (*vkbderr) ();
		int (*vmiderr) ();
		int (*statvec) ();
		int (*mousevec) ();
		int (*clockvec) ();
		int (*joyvec) ();
		int (*midisys) ();
		int (*ikbdsys) ();
	};
	struct Param {
		char topmode;
		char buttons;
		char xparam;
		char yparam;
		int xmax,ymax;
		int xinitial,yinitial;
	};
	struct KBDvecs *kbdvecs;
	struct Param *paramp;
	char kbdcmds[25];

st520move(row, col)
{
        ttputc(ESC);
        ttputc('Y');
        ttputc(row+BIAS);
        ttputc(col+BIAS);
}

st520eeol()
{
        ttputc(ESC);
        ttputc('K');
}

st520eeop()
{

#if	COLOR
		st520fcol(gfcolor);
		st520bcol(gbcolor);
#endif
        ttputc(ESC);
        ttputc('J');
}

st520rev(status)	/* set the reverse video state */

int status;	/* TRUE = reverse video, FALSE = normal video */

{

	if(status) {
		ttputc(ESC);
		ttputc('p');
	}
	else {
		ttputc(ESC);
		ttputc('q');
	}
}

#if	COLOR
st520fcol(color)
int color;	
{
		if(color == cfcolor || !STncolors)
			return;
		else {

			ttputc(ESC);
			ttputc('b');
			ttputc(color & 0x0f);
			cfcolor = color;
		}
}

st520bcol(color)
int color;
{
		if(color == cbcolor || !STncolors)
			return;
		else {
			ttputc(ESC);
			ttputc('c');
			ttputc(color & 0x0f);
			cbcolor = color;
		}

}
#endif

st520beep()
{
#ifdef  BEL
        ttputc(BEL);
        ttflush();
#endif
}

st520open()
{
	int i,j,k;
	long phys, log;	/* screen bases */
	
/* IMPORTANT: it is ABSOLUTELY necessary that the default resolution be the
 *	largest possible so that display will allocate (malloc) the maximum
 *	size for the VIDEO arrray
 */
	STrez = Getrez();
	switch(STrez) {
		case 0: /* low res 25x40 16 colors */
			strcpy(sres, "LOW");
			phys = Physbase();
			log  = Logbase();
			Setscreen(log, phys, 1);
			STrez = 1;
			/* fall thru to med res */

		case 1: /* med res 25x80 4 colors */
			if (STrez == 1) strcpy(sres, "MEDIUM");
			term.t_nrow = 25 - 1;
			term.t_ncol  = 80;
			grez = 1;
#if	COLOR
			STncolors = 4;
			for(i=0;i<8;i++) {
				oldpal[i] = Setcolor(i,newpal[i]);
			}
#endif
			break;
		case 2: /* high res 25x80 no colors */
			strcpy(sres, "HIGH");
			term.t_nrow  = 40 - 1;
			term.t_ncol  = 80;
			grez = 2;
			make_8x10(); /* create a smaller font */
			set_40();    /* and go to 40 line mode */
#if	COLOR
			STncolors = 0;
#endif
			break;
	}

	revexist = TRUE;
	eolexist = TRUE;
	paramp = (struct Param *)malloc(sizeof(struct Param));
	kbdvecs = (struct KBDvecs *)Kbdvbase();
	paramp -> topmode = 0;
	paramp -> buttons = 4;
	paramp -> xparam = 8;
	paramp -> yparam = 10;
	paramp -> xmax = 79;
	paramp -> ymax = 23;
	paramp -> xinitial = 0;
	paramp -> yinitial = 0;
	Initmous(1,paramp,kbdvecs -> mousevec);

	i = 0;
	kbdcmds[i++] = 0x0a;	/*set mouse keycode mode */
	kbdcmds[i++] = 0x08;
	kbdcmds[i++] = 0x0a;
	Ikbdws(i-1,&kbdcmds[0]);
	Cursconf(1,0);
	Cursconf(3,0);
	Cconout(27);Cconout('E');
        ttopen();
}

st520close()

{
	int i,j,k;

	i = 0;
	kbdcmds[i++] = 0x80;	/*reset mouse keycode mode */
	kbdcmds[i++] = 0x01;
	Ikbdws(i-1,&kbdcmds[0]);
	if(grez == 2 && STrez == 2) /* b/w monitor in 40 row mode */
		restore();

#if		COLOR
	for(i=0;i<STncolors;i++)
		Setcolor(i,oldpal[i]);
#endif
	Cconout(27);Cconout('E');
	paramp -> buttons = 0;
	Initmous(2,paramp,kbdvecs -> mousevec);
	i = 0;
	kbdcmds[i++] = 0x80;	/*reset the keyboard*/
	kbdcmds[i++] = 0x01;
	Ikbdws(i-1,&kbdcmds[0]);
	Cursconf(1,0);
	ttclose();
}
st520kopen()
{

}
st520kclose()
{

}

st520cres(res)		/* change screen resolution */

char *res;		/* resolution to change to */

{
	register int nurez;	/* number of res to change to */
	int ierr, i, j ,k;
	long phys, log;	/* screen bases */
	char dum[80]; /* for debugging only */

	/* determine the needed resolution */
	if (strcmp(res, "LOW") == 0)
		nurez = 1;
	else if (strcmp(res, "MEDIUM") == 0)
		nurez = 2;
	else if (strcmp(res, "HIGH") == 0)
		nurez = 3;
	else
		return(FALSE);

	if(grez == nurez)
		return(TRUE);
		
	if(STrez == 2) { /* b/w monitor-only allow hi | med rez */
		switch(nurez) {
			case 2: /* high res */
				term.t_nrow  = 40 - 1;
				term.t_ncol  = 80;
				make_8x10(); /* create a smaller font */
				set_40();    /* and go to 40 line mode */
				grez = 2;
				sgarbf = TRUE;
				onlywind(1,1);
				strcpy(sres, "HIGH");
				break;
			case 1: /* med res */
				term.t_nrow  = 25 - 1;
				term.t_ncol  = 80;
				restore();
				grez = 1;
				sgarbf = TRUE;
				onlywind(1,1);
				strcpy(sres, "MEDIUM");
				break;
			default:
				mlwrite("Invalid resolution");
				return(FALSE);
				break;
		}
	}
	else { /* color monitor-only allow low | medium resolution */
		phys = Physbase();
		log  = Logbase();
		switch(nurez) {
			case 1:
				term.t_nrow  = 25 - 1;
				term.t_ncol  = 80;
				Setscreen(log, phys, 1);
				STncolors = 4;
				grez = 1;
				sgarbf = TRUE;
				onlywind(1,1);
				strcpy(sres, "LOW");
				break;
			case 0:
				term.t_nrow  = 25 - 1;
				term.t_ncol  = 40;
				Setscreen(log, phys, 0);
				STncolors = 8;
				grez = 0;
				sgarbf = TRUE;
				onlywind(1,1);
				strcpy(sres, "MEDIUM");
				break;
			default:
				mlwrite("%Invalid resolution");
				return(FALSE);
				break;
		}
	}
	return(TRUE);
}			

STcurblink(onoff)
int onoff;
{
	if(onoff)
		Cursconf(2,0);
	else
		Cursconf(3,0);
}


char parm_save[28];
long fnt_8x10[640];

make_8x10()
{
	int i,j,k;
	long savea23[2];
	
	for(i=0;i<640;i++)
		fnt_8x10[i] = 0;
		
	asm {
	movem.l	A2-A3,savea23(A6)
	
	dc.w	LINEA_INIT		;A1 -> array of font headers

	lea	parm_save(A4),A2	;A2 -> parameters savearea
	move.l	V_OFF_AD(A0),(A2)+
	move.l	V_FNT_AD(A0),(A2)+
	move.w	V_CEL_HT(A0),(A2)+
	move.w	V_CEL_MY(A0),(A2)+
	move.w	V_CEL_WR(A0),(A2)+


	move.l	04(A1),A1		; A1 -> 8x8 font header
	move.l	76(A1),A2		; A2 -> 8x8 font data
	lea	fnt_8x10+0x100(A4),A3	; A3 -> 2nd line of font buffer
	move.w	#0x200-1,D0		; D0 <- longword counter for font xfer

fnt_loop:

	move.l	(A2)+,(A3)+
	dbf	D0,fnt_loop
		
	movem.l	savea23(A6),A2-A3
	}
	
}

set_40()
{
	long	savea23[2];
	
	asm {
	
;
;  use the 8x10 character set: 40 line mode
;

	movem.l	A2-A3,savea23(A6)
	
	dc.w	LINEA_INIT

	move.l	04(A1),A1		; A1 -> 8x8 font header
	move.l	72(A1),V_OFF_AD(A0)	; v_off_ad <- 8x8  offset table addr
	lea	fnt_8x10(A4),A2
	move.l	A2,V_FNT_AD(A0)		; v_fnt_ad <- 8x10 font data addr

	move.w	#10,V_CEL_HT(A0)	; v_cel_ht <- 10   8x10 cell height
	move.w	#39,V_CEL_MY(A0)	; v_cel_my <- 39   maximum cell "Y"
	move.w	#800,V_CEL_WR(A0)	; v_cel_wr <- 800  offset to cell Y+1

	movem.l savea23,A2-A3
	}
}

set_20()
{
	long	savea23[2];

	asm {
		
;
;  use the 8x10 character set: 20 line mode
;

	movem.l	A2-A3,savea23(A6)
	
	dc.w	LINEA_INIT		; A0 -> line A variables

	move.l	04(A1),A1		; A1 -> 8x8 font header
	move.l	72(A1),V_OFF_AD(A0)	; v_off_ad <- 8x8  offset table addr
	lea	fnt_8x10(A4),A2
	move.l	A2,V_FNT_AD(A0)		; v_fnt_ad <- 8x10 font data addr

	move.w	#10,V_CEL_HT(A0)	; v_cel_ht <- 10   8x10 cell height
	move.w	#19,V_CEL_MY(A0)	; v_cel_my <- 19   maximum cell "Y"
	move.w	#1600,V_CEL_WR(A0)	; v_cel_wr <- 800  offset to cell Y+1
	
	movem.l	savea23,A2-A3
	}
}


restore()
{
	long savea23[2];
	
	asm {
	
;  return what was saved in parameter save zone	

	movem.l	A2-A3,savea23(A6)

	dc.w	LINEA_INIT		; a0 -> line A variables

	lea	parm_save(A4),A2	; a2 -> parameter save area
	move.l	(A2)+,V_OFF_AD(A0)
	move.l	(A2)+,V_FNT_AD(A0)
	move.w	(A2)+,V_CEL_HT(A0)
	move.w	(A2)+,V_CEL_MY(A0)
	move.w	(A2)+,V_CEL_WR(A0)
	
	movem.l	savea23(A6),A2-A3
	}          
}
GetCurStat(onoff)
int	onoff;
{
	long savea23[2];

	asm {
	movem.l	A2-A3,savea23(A6)

	dc.w	LINEA_INIT		; a0 -> line A variables
	move.w	V_DISAB(A0),onoff(A6)	; 0 = cursor visible
	moveq	#0,D0
	move.w	V_DISAB(A0),D0	
	movem.l	savea23(A6),A2-A3
	}          
}
#else
sthello()
{
}
#endif

