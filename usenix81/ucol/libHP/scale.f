      subroutine scale(y,yl,np,l)
c
c
c  this subroutine is used to scale data values
c  in an array and provide parameters used by the
c  axis subroutine.
c
      dimension vint(11),y(1)
	include 'gsdata.f'
      vint(1)=1.
      vint(2)=2.
      vint(3)=4.
      vint(4)=5.
      vint(5)=8.
      vint(6)=10.
      del=.00002
      vint(7)=20.
      vint(8)=40.
      vint(9)=50.
      vint(10)=80.
      vint(11)=100.
      fn=yl
 53   k=iabs(l)
      n=np*k
      y0=y(1)
      yn=y0
      do 25 i=1,n,k
      ys=y(i)
      if (y0-ys) 22,22,21
 21   y0=ys
      go to 25
 22   if (ys-yn) 25,25,24
 24   yn=ys
 25   continue
      xmin=y0
      xmax=yn
 54   ax=-10.
      ay=-10.
      if (fn) 50,51,52
c
c     for hp7203 version, sfact is called
c
 50   if(ax .ge. 0) go to 500
      ax=xmaxin
      ay=ymaxin
 500  fn=aint(ax-1.)
      go to 52
c
c     for hp7203 version, sfact is called
c
 51   if(ax .ge. 0) go to 510
      ax=xmaxin
      ay=ymaxin
 510  fn=(aint(ay-1.))
 52    a=(xmax-xmin)/fn
      al=alog(a)*0.43429448
      nal=al
      if (a-1.) 11,12,12
 11   nal=nal-1
 12   b=a/(10.**nal)
      do 20 i=1,6
      if (b-(vint(i) +del)) 31,20,20
 20   continue
      i=6
 31   dist=vint(i)*10.**nal
      fm1=xmin/dist
      m1=fm1
      if (fm1) 32,33,33
 32   m1=m1-1
 33   xm1=abs(float(m1)+1.-fm1)
      if (xm1-del) 34,35,35
 34   m1=m1+1
 35   xminp=dist*float(m1)
      fm2=xmax/dist
      m2=fm2+1.
      if (fm2-(-1.)) 36,37,37
 36   m2=m2-1
 37   xm1= abs(fm2+1.-float(m2))
      if(xm1-del) 38,39,39
 38   m2=m2-1
 39   xmaxp=dist*float(m2)
      npp=m2-m1
      nn=fn
      if (npp-nn) 40,40,42
 42   i=i+1
      go to 31
 40   if (xminp-xmin) 14,15,15
 15   xminp=xmin
 14   if (xmaxp-xmax) 16,17,17
 16   xmaxp=xmax
 17   n=n+1
      y(n)=xminp
      n=n+1
      y(n)=dist
      if (yl) 98,98,99
 98    n=n+1
      yyl=npp
      y(n)=yyl
      go to 99
 99   return
      end
