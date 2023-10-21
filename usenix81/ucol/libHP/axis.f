      subroutine axis(x,y,ibcd,n,size,theta,xmin,dx,size1)
c
c  this subroutine is used to draw and label
c  x and y axis using the scale factors determined
c  by the scale subroutine. the axis location may be
c  specified or determined automatically by the
c  subroutine.
c
      dimension ibcd(1)
c     dimension ixx(2)
      character*2 ixx(2)
	include 'gsdata.f'
      data ixx/ 2h-1,2h0 /
      temp1=x
      temp2=y
      temp3=size
      temp4=xmin
      temp5=dx
      temp6=size1
      in=1
      in1=1
c
c     save label origin mode, line type, units
c
      ltemp=mlbpos
      iunit=msetun
      itype=mlntyp
      xpat=xdllen
      iinit=minit1
c
c     set units to inches
c
      call setin
c
c     set line type to solid lines
c
      call daslna(1,0.0)
      if(size) 53,56,56
 53   size=abs(size)
c
c     for hp7203 version, fact is called
c
      ax=1.0
      ay=1.0
c
c     for hp7203 version, sfact is called
c
      axx=xmaxin
      ayy=ymaxin
      xxl=aint(axx*ax)
      yyl=aint(ayy*ay)
      if (size1) 50,52,52
 50   ymin=x
      dy=y
      xl=size
      yl=abs(size1)
      xmax=(xl*dx)+xmin
      ymax=(yl*dy)+ymin
      if (xmin) 80,81,81
 81   if (ymin) 82,83,83
 83   y=0.5
      go to 91
 82   if (ymax) 84,84,85
 84   y=yyl-0.5
      go to 91
 80   if (xmax) 86,86,87
 86   xmin=xmax-(xxl-1.)*dx
      go to 830
 87   xm=aint(((xxl-1.)-xl)/2.)
      xmin=xmin-xm*dx
      xt1=(xxl-1.)-abs(xmin/dx)
      xt2=abs(xmin/dx)
      in1=-1
      if (xt1-xt2) 810,820,820
 810  xnt=xt2/2.
      nt1=xnt
      go to 830
 820  xnt=xt2+(xt1/2.)
      nt1=xnt
 830  if (ymin) 88,89,89
 89   y=0.5
      go to 91
 88   if (ymax) 90,90,85
 90   y=yyl-0.5
      go to 91
 85   xm=aint(((yyl-1.)-yl)/2.)
      y=abs(ymin/dy)+xm+0.5
 91   x=0.5
      size=xxl-1.
      go to 56
 52   in=-1
      xxm=x
      xxd=y
      yl=size
      xl=size1
      ymin=xmin
      y=0.5
      ymax=(yl*dx)+ymin
      xmax=(xl*xxd)+xxm
      if(xxm) 92,93,93
 93   if (ymin) 94,95,95
 95   x=0.5
      xx1=0.5
      xx2=0.5
      go to 97
 94   if (ymax) 96,96,999
 999  xx1=0.5
      x=0.5
      go to 997
 96   xx1=0.5
      xx2=(yyl-1.)-yl+0.5
      xmin=ymax-(yyl-1.)*dx
      x=0.5
      go to 97
 92   if (xmax) 98,98,99
 99   xm=aint(((xxl-1.)-xl)/2.)
      x=abs(xxm/xxd)+xm+0.5
      xx1=0.5+xm
      go to 100
 98   x=xxl-0.5
      xx1=xxl-1.-xl+0.5
 100  if (ymin) 101,977,977
 977  xx2=0.5
      go to 97
 101  if (ymax) 102,102,997
 102  xmin=ymax-(yyl-1.)*dx
      xx2=(yyl-1.)-yl+0.5
      go to 97
 997  xm=aint(((yyl-1.)-yl)/2.)
      xmin=xmin-xm*dx
      xt1=(yyl-1.)-abs(xmin/dx)
      xt2=abs(xmin/dx)
      xx2=0.5+xm
      in1=-1
      if (xt1-xt2) 991,992,992
 991  xnt=xt2/2.
      nt1=xnt
      go to 97
 992  xnt=xt2+xt1/2.
      nt1=xnt
 97   size=yyl-1.
 56   kn=n
      a=1.0
c
c     set for annotation on clockwise or counterclockwise side o
c
      if (kn) 6,7,7
    6 a=-a
      kn=-kn
    7 ex=0.0
c
c     adjust dx into range of 100.0 to 0.01
c
      adx= abs  (dx)
      if (adx) 1,5,1
    1 if (adx-100.0) 4,2,2
    2 adx=adx/10.0
      ex=ex+1.0
      go to 1
    3 adx=adx*10.0
      ex=ex-1.0
    4 if (adx-0.01) 3,5,5
    5 xval=xmin*10.0**(-ex)
      adx= dx  *10.0**(-ex)
      sth=theta*0.0174533
      cth=cos(sth)
      sth=sin(sth)
c
c     calculate starting location for tic mark annotation
c
      dxb=-0.1
      dyb=0.16*a-0.04
      xn=x+dxb*cth-dyb*sth
      yn=y+dyb*cth+dxb*sth
      ntic=size+1.0
      if (in1) 111,112,112
 111  nt=nt1+1
      sizex=nt
      go to 113
 112  nt=ntic/2
      sizex=size*0.5
 113  do  20  i=1,ntic
c
c     plot tic mark annotation increment
c
c
c     modify label origin mode
c
      if(theta .eq. 0.0) call lorg(2)
      call number(xn,yn,.10,xval,theta,2)
      xval=xval+adx
      xn=xn+cth
      yn=yn+sth
      if (nt) 20,11,20
   11 z=kn
      if (ex)  12,13,12
   12 z=z+7.0
c
c     calculate starting location for axis title
c
   13 dxb=-.07*z+sizex
      dyb=0.325*a-0.075
      xt=x+dxb*cth-dyb*sth
      yt=y+dyb*cth+dxb*sth
c
c     plot axis title
c
c
c     modify label origin mode
c
      if(theta .eq. 0.0) call lorg(2)
      call symbol(xt,yt,0.14,ibcd,theta,kn)
c
c     test for exponent and calculate starting location for base
c
      if (ex)  14,20,14
   14 z=kn+2
      xt=xt+z*cth*0.14
      yt=yt+z*sth*0.14
c     plot base, calculate starting location for exponent and pl
      call symbol(xt,yt,0.14,ixx,theta,3)
      xt=xt+(3.0*cth-0.8*sth)*0.14
      yt=yt+(3.0*sth+0.8*cth)*0.14
      call number(xt,yt,0.10,ex,theta,-1)
   20 nt=nt-1
c
c     restore label origin mode
c
      call lorg(ltemp)
      xe=x+size*cth
      ye=y+size*sth
      call plot(xe,ye,3)
      dxb=-0.07*a*sth
      dyb=+0.07*a*cth
      a=ntic-1
c
c     calculate location of last tic mark
c
      xn=x+a*cth
      yn=y+a*sth
      do  30  i=1,ntic
c
c     plot tic marks starting with the last one
c
      call plot(xn,yn,2)
      call plot(xn+dxb,yn+dyb,2)
      call plot(xn,yn,2)
      xn=xn-cth
      yn=yn-sth
c
c     use the following if -0.0 can cause problems
c     if (ntic-1-i) 30,28,20
c  28 xn=x
c     yn=y
c
   30 continue
      if (in) 31,32,32
c
 31   call plot(xx1,xx2,-3)
c
 32   x=temp1
      y=temp2
      size=temp3
      xmin=temp4
      dx=temp5
      size1=temp6
c
c     restore line type
c
      minit1=2
      call daslna(itype,xpat)
c
c     restore line type, units
c
      msetun=iunit
      mlntyp=itype
      xdllen=xpat
      minit1=iinit
      return
      end
