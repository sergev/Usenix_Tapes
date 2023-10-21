#include "stdplt.h"

prframe(dotp)
  register FRAME *dotp;
/*
 * dump debugging info to stderr.
 */
  {
	register float *protop;
	char buf[256];

	fprintf(stderr, "-------------------------------\n");
	fprintf(stderr, "\nframe %s:", _path(dotp, buf));
	fprintf(stderr, "\nflags=%6o\n'..' = '%s', '.' = '%s'\n",
		dotp->_fflags, dotp->_parent->_fname, dotp->_fname);
	fprintf(stderr, "(x,y) = (%f,%f), (xc,yc) = (%f,%f)\n",
		dotp->_x, dotp->_y, dotp->_xcat*PELSIZ, dotp->_ycat*PELSIZ);
	fprintf(stderr, "(ixc, iyc) = (%d,%d)\n", _x, _y);
	fprintf(stderr, "(xo,yo) = (%f,%f), (xoc, yoc) = (%f,%f)\n",
		dotp->_xo, dotp->_yo, dotp->_xocat*PELSIZ, dotp->_yocat*PELSIZ);
	fprintf(stderr, "t  = %f  %f\n     %f  %f\n",
		dotp->_t[0][0], dotp->_t[0][1],
		dotp->_t[1][0], dotp->_t[1][1]);
	fprintf(stderr, "tc = %f  %f\n     %f  %f\n",
		dotp->_tcat[0][0]*PELSIZ, dotp->_tcat[0][1]*PELSIZ,
		dotp->_tcat[1][0]*PELSIZ, dotp->_tcat[1][1]*PELSIZ);
	fprintf(stderr, "sx = %f, sxcat = %f\n",
		dotp->_sx, dotp->_sxcat*PELSIZ);
	fprintf(stderr, "sy = %f, sycat = %f\n",
		dotp->_sy, dotp->_sycat*PELSIZ);
	fprintf(stderr, "rot = %f degrees, rotcat = %f degrees\n",
		dotp->_rot/DEG, dotp->_rotcat/DEG);
	fprintf(stderr, "fat = %d, font = %o\n",
		dotp->_fat, dotp->_font);
	fprintf(stderr, "cw = %f, crot = %f degrees\n",
		dotp->_cw*PELSIZ, dotp->_crot/DEG);
	if(dotp->_fflags&_DODASH)
	   {
		protop= dotp->_dproto;
		fprintf(stderr,"dashes= %f %f %f %f\n",
			protop[0]*PELSIZ,
			(protop[1]-protop[0])*PELSIZ,
			(protop[2]-protop[1])*PELSIZ,
			(protop[3]-protop[2])*PELSIZ);
	   }
  }
