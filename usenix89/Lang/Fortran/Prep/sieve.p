c Sieve benchmark in fortran.  Translated directly from a c
c version (?) without much thought, to illustrate a few prep
c facilities.  Not written for speed, but it should work.

#include "premac.h"
: S		8190 ;
: do_while(l)	begin
		while (l) ;

do limits [ (0, S) ]

	integer f(0:S)
	integer i, p, k, c, n

	do n = 1, 10
	   c = 0
	   f(#) = 1

	   do i = 0, S
	      if ( f(i) != 0 ) then
	         p = 2*i + 3
	         k = i + p

c	this loop will be faster as a normal do loop on a vector machine
	         do_while ( k <= S )
	            f(k) = 0
	            k = k + p
	         again

	         c = c + 1
	      end if
	   end do

	end do

	write(*,*) c, ' primes'

	stop
	end
