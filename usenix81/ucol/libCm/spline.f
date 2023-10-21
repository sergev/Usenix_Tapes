      subroutine splneq(nn,u,s)
c$$$$$ calls no other routines
c  finds coeffs for a spline interpolation of equally spaced data
c  based on @spline interpolation ona  digital computer@ by r.f.thompson
c  nn  number of data points (may be negative - see d1,d2 below)
c  u  array of function values to be interpolated, assumed to samples at equal i
c     intervals of a twice differentiable function.
c  s  array to hold computed values of 2nd derivative of spline fit at sample
c     points.  these values are required by evaleq  to interpolate
c  if the user wishes to force specific values on the derivatives at the end
c  points, he should put h*du(1)/dx  nad  h*du(n)/dx  in  s(1),s(2), then call
c  splneq  with nn=-number of terms in series. h = sample spacing in x.
c  normally the derivatives are found by fitting a parabola through the
c  1st and last 3 points.
c  if the number of terms is between 1 and 3, straight-line interpolation is don
      dimension u(1),s(1),a(13)
c
      n=iabs(nn)
      if (n.le.3) go to 5000
      d1=-0.5*u(3)  +2.0*u(2)  -1.5*u(1)
      dn= 0.5*u(n-2)-2.0*u(n-1)+1.5*u(n)
      if (nn.gt.0) go to 1000
      d1=s(1)
      dn=s(2)
 1000 a(1)=2.0
      a(2)=3.5
      s(1)=u(2)-u(1)-d1
      s(2)=u(1)-2.0*u(2)+u(3)-0.5*s(1)
      n1=n-1
      do 3000 i=3,n1
      if (i.gt.13) go to 3000
      k=i
      a(k)=4.-1.0/a(k-1)
 3000 s(i)=u(i-1)-2.*u(i)+u(i+1)-s(i-1)/a(k-1)
      s(n)=u(n1)-u(n)+dn-s(n1)/a(k)
      s(n)=6.0*s(n)/(2.0-1.0/a(k))
      n2=n-2
c  compute 2nd derivatives by back-substitution
c  the array  a  tends to a constant (2+sqrt(3)) so only 13 elements are needed
      do 4000 j=1,n2
      i=n-j
      k=min0(i,k)
 4000 s(i)=(6.0*s(i)-s(i+1))/a(k)
      s(1)=3.0*s(1)-0.5*s(2)
      return
c  series too short for cubic spline.  fit straight lines.
 5000 do 5500 i=1,n
 5500 s(i)=0.0
      return
      end
      function evaleq(y,nn,u,s)
c$$$$$ calls no other routines
c  performs spline interpolation of equally spaced data.
c  based on @spline interpolation on a digital computer@( by r.f.thompson.
c  evaluates a spline interpolate in a set of equally spaced samples.
c  the routine  splneq  should be called first, to establish the array  s .
c  y  the  coordinate at which interpolate is required, with y=1 for 1st
c     sample point, y=2 for the 2nd, etc.  if actual spacing is  h  and  x1 is
c     the 1st sample coordinate use  y = 1.0 + (x-x1)/h
c  nn  number of samples of function in original set.
c  u  array containing function samples.
c  s  array of normalized 2nd derivatives, computed by  splneq.  the derivatives
c     have been multiplied by h**2, where h is the sample spacing.
c  if  y  is out of the range (1,nn), the 1st or last sample value is used.
      dimension u(1),s(1)
c
      if (y.le.1.0) go to 1500
      if (y.ge.float(nn)) go to 2000
      k1=y
      k=k1+1
      dk=k-y
      dk1=y-k1
      ff1=s(k1)*dk*dk*dk
      ff2=s(k)*dk1*dk1*dk1
      evaleq=(dk*(6.0*u(k1)-s(k1)) + dk1*(u(k)*6.0-s(k)) + ff1 +ff2)/6.0
      return
c  out of range.  supply constant values
 1500 evaleq=u(1)
      return
 2000 evaleq=u(nn)
      return
      end
      subroutine spline(nn,x,u,s,a)
c$$$$ calls no other routines
c  finds array  s  for spline interpolator  eval.
c  nn  number of data points supplied (may be negative, see below)
c  x  array containing x-coordinates where function is sampled.  xx(1),xx(2),...
c     must be a strictly increasing sequence.
c  u  array containing sample values that are to be interpolated.
c  s  output array of 2nd derivative at sample points.
c  a  working space array of dimension at least  nn.
c  if the user wishes to force the derivatives at the ends of the series to
c  assume specified values, he should put du(1)/dx and du(n)/dx in s1,s2
c  and call the routine with nn=-number of terms in series.  normally a parabola
c  is fitted through the 1st and last 3 points to find the slopes.
c  if less than 4 points are supplied, straight lines are fitted.
      dimension x(1),u(1),s(1),a(1)
c
      q(u1,x1,u2,x2)=(u1/x1**2-u2/x2**2)/(1.0/x1-1.0/x2)
c
      n=iabs(nn)
      if (n.le.3) go to 5000
      q1=q(u(2)-u(1),x(2)-x(1),u(3)-u(1),x(3)-x(1))
      qn=q(u(n-1)-u(n),x(n-1)-x(n),u(n-2)-u(n),x(n-2)-x(n))
      if (nn.gt.0) go to 1000
      q1=s(1)
      qn=s(2)
 1000 s(1)=6.0*((u(2)-u(1))/(x(2)-x(1)) - q1)
      n1= n - 1
      do 2000 i=2,n1
      s(i)= (u(i-1)/(x(i)-x(i-1)) - u(i)*(1.0/(x(i)-x(i-1))+
     + 1.0/(x(i+1)-x(i))) + u(i+1)/(x(i+1)-x(i)))*6.0
 2000 continue
      s(n)=6.0*(qn + (u(n1)-u(n))/(x(n)-x(n1)))
      a(1)=2.0*(x(2)-x(1))
      a(2)=1.5*(x(2)-x(1)) + 2.0*(x(3)-x(2))
      s(2)=s(2) - 0.5*s(1)
      do 3000 i=3,n1
      c=(x(i)-x(i-1))/a(i-1)
      a(i)=2.0*(x(i+1)-x(i-1)) - c*(x(i)-x(i-1))
      s(i)=s(i) - c*s(i-1)
 3000 continue
      c=(x(n)-x(n1))/a(n1)
      a(n)=(2.0-c)*(x(n)-x(n1))
      s(n)=s(n) - c*s(n1)
c  back substitiute
      s(n)= s(n)/a(n)
      do 4000 j=1,n1
      i=n-j
      s(i) =(s(i) - (x(i+1)-x(i))*s(i+1))/a(i)
 4000 continue
      return
c  series too short for cubic spline - use straight lines.
 5000 do 5500 i=1,n
 5500 s(i)=0.0
      return
      end
      function eval(y,nn,x,u,s)
c$$$$ calls no other routines
c  performs cubic spline interpolation of a function sampled at unequally
c  spaced intervals.  the routine spline  should be called to set up the array s
c  y  the coordinate at which function value is desired.
c  nn  number of samples of original function.
c  x  array containing sample coordinates. the sequence x(1),x(2).....x(nn)
c     must be strictly increasing.
c  u  array containing samples of function at the coordinates x(1),x(2)...
c  s  array containing the 2nd derivatives at the sample points.  found by the
c     routine  spline, which must be called once before beginning interpolation.
c  if  y  falls outside range(x(1),x(nn))  the value at the nearest endpoint
c  of the series is used.
      dimension x(1),u(1),s(1)
      common /startx/ istart
      data istart/1/
c
      nn=iabs(nn)
      if (y.lt.x(1))  go to 3000
      if (y.gt.x(nn)) go to 3100
c  locate interval (x(k1),x(k))  containing y
      if (y-x(istart)) 1200,1000,1000
c  scan up the x arrat
 1000 do 1100 k=istart,nn
      if (x(k).gt.y) go to 1150
 1100 continue
 1150 k1=k-1
      go to 1500
c  scan downwards in x array
 1200 do 1300 k=1,istart
      k1=istart-k
      if (x(k1).le.y) go to 1350
 1300 continue
 1350 k=k1+1
 1500 istart=k1
c  evaluate interpolate
      dy=x(k)-y
      dy1=y-x(k1)
      dk=x(k)-x(k1)
      deli=1./(6.0*dk)
      ff1=s(k1)*dy*dy*dy
      ff2=s(k)*dy1*dy1*dy1
      f1=(ff1+ff2)*deli
      f2=dy1*((u(k)/dk)-(s(k)*dk)/6.)
      f3=dy*((u(k1)/dk)-(s(k1)*dk)/6.0)
      eval=f1+f2+f3
      return
c  out of range.  substitute end values.
 3000 eval=u(1)
      return
 3100 eval=u(nn)
      return
      end
