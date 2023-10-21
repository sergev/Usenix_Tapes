      subroutine ur2(m,n,a,x,y,c,ifl)
c
c
c   subroutine ur2 is the counterpart of subroutine ur for systems that
c   are underdetermined (m.lss.n).  ur2 does a u*r (unitary times right
c   triangular) factorization of the transpose of an arbitrary matrix a
c   (overwriting a) then solves the system a*x=y.  the pieces of the
c   routine, urfactor and ursolve, are independent and may be seperated
c   if desired.  the advantage would be that once a is factored the
c   system a*x=y may be solved any number of times very inexpensively.
c   ur2 finds the shortest solution x of the system.  arrays in the
c   calling sequence are dimensioned - a(m,n), x(n), y(m), and c(n).
c   c is used for scratch space in ur2.  ifl is an error flag set by ur2.
c   ifl=1 indicates correct inversion, ifl=2 indicates matrix a was not
c   of maximal rank.  ur2 calls no other routines.
c    - rpb
      dimension a(m,n),x(n),y(m),c(n)
c   error off if the system is not underdetermined.
      if(m-n)19,19,18
 19   ifl=1
c
c   ***** urfactor begins here *****
c
c   unitary matrix u=h1*h2*...*hn where the hi's are householder's
c   elementary hermitian matrices.  note that neither u nor the hi's
c   are ever explicitly computed.  the hi's may be reconstructed from
c   the right triangular part of a and the vector c.
      do 1 k=1,m
      cc=0.
c   compute normalizations a(k,k)(=con) for the elementary hermitians
c   and r(k,k)(=cc).
      do 2 i=k,n
      cc=cc+a(k,i)*a(k,i)
 2    continue
      cc=sqrt(cc)
      con=a(k,k)
      if(con)3,3,4
 4    cc=-cc
 3    con=con-cc
      a(k,k)=con
      if(k-m)6,16,16
 6    k1=k+1
c   compute the right triangular matrix r.
      do 5 j=k1,m
      b=0.
      if(cc)8,7,8
 8    do 9 i=k,n
      b=b+a(k,i)*a(j,i)
 9    continue
      b=b/abs(cc*con)
 7    do 10 i=k,n
      a(j,i)=a(j,i)-b*a(k,i)
 10   continue
 5    continue
 16   c(k)=cc
 1    continue
c
c   ***** ursolve begins here ***
c
      if(c(1))17,18,17
c   solve l*z=y where l is r transpose.
 17   x(1)=y(1)/c(1)
      do 15 k=2,m
c   error off if one of the c@s is zero
      if(c(k))12,18,12
 12   k1=k-1
      b=0.
      do 21 j=1,k1
      b=b+a(k,j)*x(j)
 21   continue
      x(k)=(y(k)-b)/c(k)
 15   continue
c   zero out the rest of z.
      k=m+1
      if(k-n)22,22,23
 22   do 20 j=k,n
      x(j)=0.
 20   continue
c   compute x=u*z (xn=hn*z,xn-1=hn-1*xn,...,x=x1=h1*x2).  note that this
c   algorithm is always faster than explicitly computing either u or the
c   hi@s.
 23   do 11 j=1,m
      k=k-1
      b=0.
      do 13 i=k,n
      b=b+a(k,i)*x(i)
 13   continue
      b=b/abs(c(k)*a(k,k))
      do 14 i=k,n
      x(i)=x(i)-b*a(k,i)
 14   continue
 11   continue
      return
c   error exit.
 18   ifl=2
      return
      end
