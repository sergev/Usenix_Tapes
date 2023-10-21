      subroutine chol(n,a,x,b,iflag)
c$$$$$ calls no other routines
c  solution of positive-definite symmetric linear system by cholesky
c  decomposition.  se wilkinson + reinsch
c  solves   ax=b   for x  where
c  n  order of matrix
c  a  symmetric array.  half below diagonal need not be supplied and is
c        destroyed during solution
c  x  unknown vector
c  b  right hand vector
c  iflag  error flag( set to 1 if a  is not positive definite
c
c  pivot array  p  set for a maximum array size of 100 x 100.
c  changes for double-precision alternative flagged by $$$$$$ following comment
c  translated from algol by r.l. parker
c
c$$$$$$      double precision a,b,x,y,z,p
      dimension a(n,n),x(n),b(n),p(100)
      data iout/0/
      iflag=0
      do 1500 i=1,n
      do 1500 j=i,n
      y=a(i,j)
      if (i.eq.1) go to 1350
      do 1300 kk=2,i
      k=i-kk+1
 1300 y=y-a(j,k)*a(i,k)
 1350 if (i.ne.j) go to 1400
      if (y) 3000,3000,1370
c$$$$$$ 1370 p(i)=1.0/dsqrt(y)
 1370 p(i) = 1.0/sqrt(y)
      go to 1500
 1400 a(j,i)=y*p(i)
 1500 continue
      do 2500 i=1,n
      z=b(i)
      if (i.eq.1) go to 2500
      do 2200 kk=2,i
      k=i-kk+1
 2200 z=z-a(i,k)*x(k)
 2500 x(i)=z*p(i)
      do 2700 ii=1,n
      i=n-ii+1
      z=x(i)
      i1=i+1
      if (i.eq.n) go to 2700
      do 2600 k=i1,n
 2600 z=z-a(k,i)*x(k)
 2700 x(i)=z*p(i)
      return
c
c  error message and return
 3000 write(iout,300)
 300  format( 53h matrix in chol not positive definite.  chol fails    )
      iflag=1
      return
      end                                                               chol
