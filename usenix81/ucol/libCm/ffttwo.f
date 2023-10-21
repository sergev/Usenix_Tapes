      subroutine ffttwo(a,nd)                                            parker
c  Routine for finding discrete fourier transform of a complex series.
c  modified from Cooley et al IEEE Trans of Education 12 no 1 (1969)
c                          argument list
c  a  is a complex array containing the series to be transformed.  the results
c  are returned in  a  over-writing the original series.
c  abs(nd)  is the number of terms in the series.  It must be a power of 2 
c  or the routine will print a message and return without doing anything.
c  It finds, for m running from 0 through abs(nd)-1, the sum of
c  a(k+1)*exp(isg*2*pi*(0,1)*k*m/n),  with k=0 to abs(nd)-1.  Here
c  isg  is the sign of nd.
c  Note indices are displaced with respect to frequency, e.g. zero frequency is
c  held in a(1).
      dimension a(1)
      complex a,u,w,t
      data pi/3.1415926536/
      n = iabs(nd)
      isg = nd/n
      m=alog(float(n))/0.693147
      if (n.ne.2**m) go to 1000
      nv2=n/2
      nm1=n-1
      j=1
      do 7 i=1,nm1
      if (i.ge.j) go to 5
      t=a(j)
      a(j)=a(i)
      a(i)=t
5     k=nv2
6     if (k.ge.j) go to 7
      j=j-k
      k=k/2
      go to 6
7     j=j+k
      le=1
      do 20 l=1,m
      le1=le
      le=2*le
      u=(1.0,0.0)
      b=isg*pi/float(le1)
      w=cmplx(cos(b),sin(b))
      do 20 j=1,le1
      do 10 i=j,n,le
      ip=i+le1
      t=a(ip)*u
      a(ip)=a(i)-t
10    a(i)=a(i)+t
20    u=u*w
      return
1000  write(6,100) n
 100  format(5(4h ***),i6,70h is not a power of 2.  ffttwo cannot transf
     +orm a series of this length    )
      return
      end                                                               ffttwo
