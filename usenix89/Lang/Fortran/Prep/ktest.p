c test program for Kutta.  Probably needs double precision.  If so
c do:
c	prep -d DOUBLE ktest

#include "premac.h"
: DIM	2 ;

	implicit real ( a-h, o-z )
	real y(DIM)
	external simple

	xi = 10
	xf = 0
	y(1) = exp(xi)
	y(2) = exp(xi)
	n = DIM
	del = .1
	accurc = 1.e-8
	imax = 10
	
	write(*,*) kutta( xi, xf, y, n, del, accurc, imax, simple ),
     *		' iterations'

	do i = 1, DIM
	   write(*,100) i, xf, y(i)-1
	end do

	stop
100	format( ' error in y(', i2, ') at x=', g12.5, ' is ', g12.5 )
	end


c functions to be integrated
	subroutine simple( x, y, f )
	implicit real ( a-h, o-z )
	real y(DIM), f(DIM)

	f(1) = y(2)
	f(2) = 2*y(2) - y(1)

	return
	end




c Runge Kutta ODE solver, real version
c
c This routine solves a system of 1st order ODE's of the form
c
c     dYn
c     --- = Fn(x,Yn)
c     dx
c
c where the functions Fn are defined by the external routine
c equa( x, Y, F ).  n must be <= DIM, defined below.
c
c variable defs
c	xi, xf		integrate from xi to xf
c	y		initial y's,  solutions also returned here
c	n		number of initial y's
c	del		initial dx
c	accurc		accuracy measure
c	imax		maximum times that del may be halved
c	equa		external functions to be integrated
c
c returns the number of iterations, -1 on error
c
c originally written ??? by ???
c converted to a readable form 2/2/87, P.R.Ove

#include "premac.h"

: INCREASING_DONE	( (xn+h>=xf) & (xf>=xi) ) ;
: DECREASING_DONE	( (xn+h<=xf) & (xf<=xi) ) ;
: DONE			( INCREASING_DONE | DECREASING_DONE ) ;
: DIM	6 ;

do limits [ n ]

	integer function kutta( xi, xf, y, n, del, accurc, imax, equa )
	implicit real (a-h,o-z)
	real	y(DIM), yi(DIM), yn(DIM),
     *		k1(DIM), k2(DIM), k3(DIM), k4(DIM), k5(DIM),
     *		f(DIM), e(DIM), f1(DIM)
	real	test, xi, xf, del, accurc, xn, h
	real	amax1, abs
	logical quit

c initialization
	if ( n > DIM ) then
	   write(*,*) 'KUTTA: too many equations: ', n, '>', DIM
	   kutta = -1
	   return
	end if

	kutta = 0
	n_halves = 0
	xn = xi
	h = del
	quit = FALSE
	yn(#) = y(#)

c fix sign of h if not sensible
	if (  ( xf > xi & h < 0 )  |  ( xf < xi & h > 0 )  ) h = -h

c check to see if finished. adjust h so that y(xf) is returned.
	begin
	   if ( DONE ) then
	      del = h
	      h = xf - xn
	      quit = TRUE
	   end if

c runge kutta calculation. calculates y(xn + h) from y(xn) and dy(xn).
	   call equa(xn, yn, f1)

	   begin

[	      k1(#) = h*f1(#)/3.
	      yi(#) = yn(#) + k1(#)	]

	      call equa(xn + h/3., yi, f)

[	      k2(#) = h*f(#)/3.
	      yi(#) = yn(#) + k1(#)/2. + k2(#)/2.	]

	      call equa(xn + h/3., yi, f)

[	      k3(#) = h*f(#)/3.
	      yi(#) = yn(#) + 3.*k1(#)/8. + 9.*k3(#)/8.	]

	      call equa(xn + h/2., yi, f)

[	      k4(#) = h*f(#)/3.
	      yi(#) = yn(#) + 3.*k1(#)/2. - 9.*k3(#)/2. + 6.*k4(#)	]

	      call equa(xn + h, yi, f)

c accuracy check. repeats with h halved if check fails.
	      test = 0.0

[	      k5(#) = h*f(#)/3.
	      e(#) = (k1(#) - 9.*k3(#)/2. + 4.*k4(#) - k5(#)/2.)/5.
	      test = amax1(test, abs(e(#)))	]

	   while ( test >= accurc )
	      n_halves = n_halves + 1
	      if( n_halves >= imax ) then
	         del = h
	         kutta = -1
	         write(*,*) 'KUTTA: step made too small:'
	         write(*,*) '       del = ', del, '    at x = ', xn
	         return
	      end if
	      h = h/2.
	      quit = FALSE
	   again

c increase h to 2h if acceptable. return if finished.
	   yn(#) = yn(#) + (k1(#) + 4.*k4(#) + k5(#))/2.
	   xn = xn + h
	   kutta = kutta + 1
	   if ( test < accurc/32. ) h = 2.*h

	until ( quit )

	y(#) = yn(#)

	return
	end
