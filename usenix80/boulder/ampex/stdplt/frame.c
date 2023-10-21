#include "stdplt.h"

FRAME *frame(afname)
  char *afname;
/*
 * Change to named frame, creating it if necessary.
 * New frames are identical to their parent.
 * Frame name syntax is identical to UNIX filename syntax.
 * A pointer to the new current frame is returned, and is also
 * put in _dotp.  The pen is moved to the current pen location
 * of the new current frame.
 */
  {
	register FRAME *dotp;
	char *fname;

	fname = afname;
	dotp = _find(&fname);	/* get to furthest existing place */
	while (*fname)		/* have to make a frame */
	  {	FRAME *olddotp;

		olddotp = dotp;
		if ((dotp = calloc(1, sizeof (FRAME))) == 0)
			_err("frame: out of space\n");
		  {	register int *ddp, *dp;

			for (dp = dotp, ddp = olddotp; dp < dotp + 1; )
				*dp++ = *ddp++;
		  }
		  {	register FRAME *dotdotp;
			register char *np;

			dotdotp = olddotp;
			dotp->_fflags =| _DOTXT;
			dotp->_fflags =& ~_DOWIND;
			dotp->_parent = dotdotp;
			dotp->_sibling = dotdotp->_child;
			dotp->_child = 0;
			dotdotp->_child = dotp;
			for (np = dotp->_fname; *fname && *fname != '/'; )
			  {	if (np < dotp->_fname + _FNMLEN)
					*np++ = *fname++;
				else
					_err("frame: more than %d characters in part of %s\n",
						_FNMLEN, _cfname(afname));
			  }
			*np = '\0';
			if (*fname)
				fname++;	/* skip over next '/' */

			dotp->_xo = dotp->_yo = 0.;
			dotp->_t[0][0] = dotp->_t[1][1] = 1.0;
			dotp->_t[0][1] = dotp->_t[1][0] = 0.;
			dotp->_sx = dotp->_sy = 1.;
			dotp->_rot = 0.;
		  }
	  }
	return(_dotp = dotp);
  }
