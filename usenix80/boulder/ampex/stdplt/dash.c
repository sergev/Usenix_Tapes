#include "stdplt.h"

dash(ddefp)
  register float *ddefp;
/*
 * Define dash line mode for current frame.
 * ddefp is a pointer to a 4 element vector containing
 * dash1, space1, dash2, space2 lengths for the dashed line prototype.
 * All units are in the 1. by 1. root coordinate frame.
 * A null ddefp means turn off the dashed lines (i.e. use solid lines).
 * A ddefp in the range 1 - NDASHLIN means use a predefined prototype.
 * A check on the validity of the pointer is made by
 * checking that each segment length lies between 0. and 1.
 * The prototype _dproto will contain the distances from the begining
 * of the prototype to the end of each of the 4 dash/space segments.
 */
   {
	register FRAME *dotp;
	register float *protop;	/* ptr to prototype */
	float tmp;
#	define NDASHLIN	4	/* predefined ddef[]s */
	static float preddef[] =
	  {	0.0003,0.005,0.0003,0.005,
		0.01,0.01,0.01,0.01,
		0.03,0.01,0.03,0.01,
		0.03,0.01,0.01,0.01,
	  };

	dotp= _dotp;
	if(ddefp == 0)		/* return to solid line */
	  {	dotp->_fflags =& ~_DODASH;
		return;
	  }
	if(ddefp <= NDASHLIN)	/* use predefined dashes */
		ddefp = &preddef[(((int)ddefp) - 1)*4];
	if (((int)ddefp)&1)	/* garbage ptr */
		_err("dash: bad ddef pointer %6o\n", ddefp);
	for (protop = dotp->_dproto, tmp = 0.; protop < &dotp->_dproto[4]; )
	  {	if (*ddefp < 0. || *ddefp > 1.)
			_err("dash: segment length %f not in 0. - 1. range\n",
				*ddefp);
		*protop++ = (tmp += *ddefp++ *STDSIZ);
	  }
	dotp->_fflags =| _DODASH;
	dotp->_dphase= 0.;
   }
