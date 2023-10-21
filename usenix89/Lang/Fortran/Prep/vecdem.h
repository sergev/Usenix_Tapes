c macros defs for vec demo

#include "premac.h"

: XLIM		81 ;		hard dimensions of arrays are from 0 --> ?lim
: YLIM		81 ;

: SCRNX		320 ;		geodesic drawing screen dimensions
: SCRNY		200 ;
: PHOTONS	64 ;		number of photons

: SMALL		1.e-20 ;
: BIG		1.e+20 ;

: include(x)	use x ;		cray specific file include
: PERIODIC(x)	call periodic( mx, my, x ) ;

c default do limits
do_limits = [ (XLIM-1), (YLIM-1) ]
