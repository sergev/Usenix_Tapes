      subroutine locate(x1,x2,y1,y2)
	include 'gsdata.f'
c
c
c
c
c
c
c     this is the locate module
c
      xmapt1=x1
      ymapt1=y1
c
c
      call change(xmapt1,ymapt1)
c
c
      xmapt2=x2
      ymapt2=y2
c
      call change(xmapt2,ymapt2)
c
c
c
c     this is used to check to make sure that the delta
c     graphic limit values are not zero.
c     thus if they are the same the system will increment
c      the value of the max by one such that the range
c      will never be exactly zero.
c
c
c
      if (xmapt1.eq.xmapt2) xmapt2=xmapt2+1.0
      if (ymapt1.eq.ymapt2) ymapt2=ymapt2+1.0
c
c
c
c
      call clip(x1,x2,y1,y2)
c
      return
      end
