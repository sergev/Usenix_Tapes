/*
 * Standard Plot Library header file.
 *
 *	John Nickolls, Stanford University
 */
#ifndef FILE
#	include <stdio.h>
#endif

#define FRAME struct _frame
#define STDSIZ	16383.			/* size of standard plot in "" coords */
#define PELSIZ	6.10388817676860e-5	/* 1/STDSIZ */
#define _FNMLEN	14			/* # of characters in frame name */
#define _CMD	0200			/* distinguishes vplot cmds from ascii */
#define PI	3.14159265358979323844	/* pi */
#define DEG	0.01745329251994329576	/* pi/180 */

struct _frame
  {	int _fflags;			/* flags for "." */
	struct _frame *_parent;		/* ptr to ".." */
	struct _frame *_sibling;	/* ptr to sibling chain */
	struct _frame *_child;		/* ptr to child chain */
	char _fname[_FNMLEN+1];		/* name for "." */
	float _x, _y;			/* "."s pen location in "." coords */
	float _xo, _yo;			/* "."s origin in ".." coords */
	float _xcat, _ycat;		/* "."s pen location in "" coords */
	float _xocat, _yocat;		/* "."s origin in "" coords */
	float _t[2][2];			/* transform from "." to ".." */
	float _tcat[2][2];		/* transform from "." to "" */
	float _sx;			/* x-scale between "." and ".." */
	float _sxcat;			/* x-scale between "." and "" */
	float _sy;			/* y-scale between "." and ".." */
	float _sycat;			/* y-scale between "." and "" */
	float _rot;			/* rotation between "." and ".." */
	float _rotcat;			/* rotation between "." and "" */
	float _dproto[4];		/* prototype dashed line */
	float _dphase;			/* distance along prototype */
	char _fat;			/* extra line fatness in "." */
	char _font;			/* character font in "." */
	float _cw;			/* character width in "" coords */
	float _crot;			/* rotation between chars and "." */
	float _wind[4];			/* window limits (x1,x2,y1,y2) in "." coords */
  };

/*
 * _fflags:
 */
#define _DOCAT 01			/* frame needs concatenation update */
#define _DOTXT 02			/* text size and angle need to be output */
#define _DODASH 04			/* do a dashed line */
#define	_TXTMODE 010			/* last action was textf() call */
#define _DOWIND	020			/* do window */

/*
 * global vars
 */
FRAME _root;				/* root frame "/" */
FRAME *_dotp;				/* ptr to current frame "." */
FILE *stdplt;				/* standard plot file descriptor */
unsigned _x, _y;			/* current actual pen location in "" */
char _fat;				/* current fatness */
float _tmp;				/* used by _T() */
float _rwind[4];			/* global window limits in "" coords */

/*
 * function type declarations
 */
FRAME *frame(), *_find();
char *_fmatch(), *_path(), *_cfname();
double cwidth(), xdist(), ydist();
double sin(), cos();

/*
 * in-line functions
 */
#define _T(tp,x,y)	(_tmp = *tp++ *(x), _tmp + *tp++ *(y))

#define xpos		_dotp->_x
#define ypos		_dotp->_y
#define charw		(_dotp->_cw/_dotp->_sxcat)
#define charh		(_dotp->_cw*1.5/_dotp->_sycat)
#define charrot		_dotp->_crot

#define move(x,y)	_move((double) (x), (double) (y))
#define rmove(dx,dy)	_move(xpos + (dx), ypos + (dy))
#define pmove(r,t)	move((r)*cos((double)t), (r)*sin((double)t))
#define rpmove(dr,t)	rmove((dr)*cos((double)t), (dr)*sin((double)t))
#define draw(x,y)	_draw((double) (x), (double) (y))
#define rdraw(dx,dy)	_draw(xpos + (dx), ypos + (dy))
#define pdraw(r,t)	draw((r)*cos((double)t), (r)*sin((double)t))
#define rpdraw(dr,t)	rdraw((dr)*cos((double)t), (dr)*sin((double)t))
#define plot(x,y,code)	( (code)? draw((x),(y)): move((x),(y)) )
#define rplot(x,y,code)	( (code)? rdraw((x),(y)): rmove((x),(y)) )
#define point(x,y)	{ move((x),(y)); draw(xpos, ypos); }
#define vector(x1,y1,x2,y2) { move((x1),(y1)); draw((x2),(y2)); }
#define origin(xo,yo)	_origin((double) (xo), (double) (yo))
#define rotate(t)	_rotate((double) (t))
#define scale(sx,sy)	_scale((double) (sx), (double) (sy))
#define pathname(buf)	_path(_dotp, (buf))
#define mapsq(xl,xh,yl,yh) {origin((xl),(yl)); scale((double)(xh)-(double)(xl),(double)(yh)-(double)(yl));}
#define window(x1,x2,y1,y2) _window((double)(x1),(double)(x2),(double)(y1),(double)(y2),_dotp)
#define rmwindow() _rmwindow(_dotp)
#define rwindow(x1,x2,y1,y2) _window((double)(x1),(double)(x2),(double)(y1),(double)(y2),NULL)
