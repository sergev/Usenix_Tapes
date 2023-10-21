#include "stdplt.h"

/*
 * Standard Plot Library Data.
 */
FRAME _root =
  {	_DOTXT,				/* _fflags */
	&_root, 0, 0, "/",		/* _parent, _sibling, _child, _fname */
	0., 0.,				/* _x, _y */
	0., 0.,				/* _xo, _yo */
	0., 0.,				/* _xcat, _ycat */
	0., 0.,				/* _xocat, _yocat */
	{STDSIZ, 0.,			/* _t[2][2] */
	 0., STDSIZ},
	{STDSIZ, 0.,			/* _tcat[2][2] */
	 0., STDSIZ},
	STDSIZ,				/* _sx */
	STDSIZ,				/* _sxcat */
	STDSIZ,				/* _sy */
	STDSIZ,				/* _sycat */
	0.,				/* _rot */
	0.,				/* _rotcat */
	{0., 0., 0., 0.},		/* _dinfo[4] */
	0.,				/* _dval */
	0,				/* _fat */
	0,				/* _font */
	252.,				/* _cw (.0154*STDSIZ) */
	0.,				/* _crot */
	{0., 0., 0., 0.},		/* _wind[4] */
  };

FRAME *_dotp = &_root;
FILE *stdplt = stdout;
unsigned _x = 0, _y = 0;
char _fat = 0;
float _rwind[] = {0.,65532.,0.,65532.};   /* 4.*STDSIZ */
int _frlen = sizeof (FRAME);		/* for debugging */
