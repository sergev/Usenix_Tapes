      subroutine hplot(x,y,npt,iline,isymb,ipen,
     + xmin,xmax,ymin,ymax,xl,yl,tl,mdevin,mdevot)
	dimension x(1),y(1)
      character*8 itim
      character*9 idat
      character*15 iar
      character*20 jar
      character*1 tl(80),it(80),yl(80),xl(80)
      logical xseq,xlim,ylim
      data intype /-1/
      call plots(intype,mdevin,mdevot)
      intype=1
      call cntr(tl,it,n)
      if(n  .eq.  0)  go to 1
      call lorg(4)
      call csizea(3.,.60,0)
      call symbol(70.,0,3.,it,0.,n)
    1 call cntr(yl,it,n)
      if(n  .eq.  0)  go to 2
      call lorg(6)
      call csizea(1.5,.5,15.5)
      call symbol(0.,50.,1.5,it,90.,n)
    2 call cntr(xl,it,n)
      if(n  .eq.  0)  go to 3
      call lorg(4)
      call symbol(70.,6.,1.5,it,0.,n)
    3 continue
      if(npt)  100,101,102
 101	continue
	call plotof
	print 103
  103 format(' ABORT--NUMBER OF POINTS =0')
      return
  100 npt=-npt
      xseq=.true.
      xlim=.false.
      xst=xmin
      xinc=xmax
      xmin1=xst
      xmax1=xst +(npt-1.)*xinc
      go to 6
  102 xseq=.false.
      if(xmin  .eq.  0  .and.  xmax  .eq.  0)  go to 4
    9 xmin1=xmin
      xmax1=xmax
      xlim=.true.
      go to 6
    4 xmin1=x(1)
	xmax1=xmin1
      do 5 i=2,npt
      if(x(i)  .gt.  xmax1)  xmax1=x(i)
      if(x(i)  .lt.  xmin1)  xmin1=x(i)
    5 continue
      xlim=.false.
    6 if(ymin  .eq.  0  .and.  ymax  .eq.  0)  go to 10
    7 ymin1=ymin
      ymax1=ymax
      ylim=.true.
      go to 8
 10	continue
	ymin1=y(1)
	ymax1=y(1)
      do 11 i=2,npt
      if(y(i)  .gt.  ymax1)  ymax1=y(i)
      if(y(i)  .lt.  ymin1)  ymin1=y(i)
   11 continue
      ylim=.false.
    8 call locate(10.,140.,15.,95.)
      if(xmax1  .gt.  xmin1)  go to 104
	call plotof
      print 105,xmax1,xmin1
  105 format(' XMAX='e15.8', XMIN1='e15.8' NO PLOT')
      return
  104 if(ymax1  .gt.  ymin1)  go to 106
	call plotof
      print 107,ymax1,ymin1
  107 format(' YMAX='e15.8', YMIN='e15.8' NO PLOT')
      return
  106 continue
      call rndlmt(xmin1,xmax1,xoff,xtic,xlim)
      call rndlmt(ymin1,ymax1,yoff,ytic,ylim)
      call mapuu(xmin1,xmax1,ymin1,ymax1)
      call setgu
      if(xoff  .eq.  0)  go to 60
      write(iar,"(f14.2)") xoff
      if(abs(xoff).gt.1.e10  .or.  abs(xoff).lt..05)write(iar,"(e14.7)") xoff
      iar(15:15)='+'
      call lorg(1)
      call symbol(10.,8.5,1.3,iar,0.,15)
   60 if(yoff  .eq.  0) go to 70
      write(iar,"(f14.2)") yoff
      if(abs(yoff).gt.1.e10.or.abs(yoff).lt..05)write(iar,"(e14.7)") yoff
      iar(15:15)='+'
      call lorg(1)
      call symbol(0.,98.,1.3,iar,0.,15)
   70 call date(idat)
      call time(itim)
      jar=' '//idat//' '//itim//' '
      call symbol(145.,35.,1.3,jar,90.,20)
      call csizea(1.5,.5,0)
      call setuu
      call penspd(6)
      call laxes(xtic,ytic,xmin1,ymin1,-2,2,2.)
      call axes(xtic,ytic,xmax1,ymax1,2,2,2.)
      call penspd(36)
      ztic=ytic/10.
      if(mod(ipen,5)  .ne.  1)  call newpen(ipen)
   50 continue
      z=xtic/5.
      if(iline  .ne.  0)  call daslna(iline,z)
      call penup
      if(xseq)  xxx=xst-xoff
      if(.not.  xseq)  xxx=x(1)-xoff
      call plot(xxx,y(1)-yoff,0)
      if(isymb  .ne.  0)  go to 20
	go to 21
   20 continue
      call symbol(xxx,y(1)-y0ff,ztic,isymb,0.,-1)
      call plot(xxx,y(1)-yoff,3)
   21 continue
      ip=2
      if(iline  .eq.  0)  ip=3
      do 12 i=2,npt
      yyy=y(i)-yoff
      if(xseq)  xxx=xst+(i-1.)*xinc
      if(.not. xseq ) xxx=x(i)-xoff
      call plot(xxx,yyy,ip)
      if(isymb  .ne.  0)   go to 23
	go to 12
   23 continue
      call symbol(xxx,yyy,ztic,isymb,0.,-1)
      call plot(xxx,yyy,0)
   12 continue
      call newpen(0)
      call plotof
      return
      entry  adplot
      call ploton
      call newpen(ipen)
      go to 50
      end
