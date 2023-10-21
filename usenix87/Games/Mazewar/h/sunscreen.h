/* RCS Information: $Header: sunscreen.h,v 1.1 84/08/25 17:25:00 jcoker Exp $ */

#include <pixrect/pixrect_hs.h>

extern struct pixrect	*px_screen;
extern struct pixfont	*pf_font;

#define	SCREEN_WIDTH	1024
#define SCREEN_HEIGHT	800

#define OP_WRITE	PIX_SRC
#define OP_SET		(PIX_SRC | PIX_NOT(PIX_SRC))
#define OP_CLEAR	(PIX_SRC & PIX_NOT(PIX_SRC))
#define OP_OR		(PIX_SRC | PIX_DST)
#define OP_AND		(PIX_SRC & PIX_DST)
#define OP_XOR		(PIX_SRC ^ PIX_DST)

/* clear the given pixrect */
#define prclear(pr)		pr_rop((pr), 0, 0, (pr)->pr_width, \
					(pr)->pr_height, PIX_SET,\
					(struct pixrect *)0, 0, 0)
/* copy the pixrect onto the other pixrect */
#define copyat(pr,x,y,op)	pr_rop(px_screen, (x), (y), (pr)->pr_width, \
					(pr)->pr_height, (op), (pr), 0, 0)
/* copy the pixrect onto the screen */
#define prcopyat(pr1,pr,x,y,op)	pr_rop((pr1), (x), (y), (pr)->pr_width, \
					(pr)->pr_height, (op), (pr), 0, 0)
/* paint the solid rectangle on the pixrect */
#define rect(pr,x,y,w,h,op)	pr_rop((pr), (x), (y), (w), (h), (op), \
					(struct pixrect *)0, 0, 0)
/* draw the line on the pixrect */
#define line(pr,x1,y1,x2,y2,op)	pr_vector((pr), (x1), (y1), \
					(x2), (y2), (op), 1)
/* draw a single point on the pixrect */
#define point(pr,x,y)		pr_put((pr), (x), (y), 1)

/*
 *   Window    X-begin Y-begin  Width   Height
 *
 *  View Window	  24	  24	  752	  568
 *  Stat Window	  800	  24	  200	  752
 *  Map Window	  24	  592	  320	  160
 */

#define VIEWX		24
#define VIEWY		24
#define VIEWW		752
#define VIEWH		568

#define STATX		800
#define STATY		24
#define STATW		200
#define STATH		752

#define MAPX		24
#define MAPY		616
#define MAPW		320
#define MAPH		160

#define MAXDEPTH	30
