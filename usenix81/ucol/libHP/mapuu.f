      subroutine mapuu(x1,x2,y1,y2)
	include 'gsdata.f'
c
c     this is used to set the variables user unit mapping
c     points.
c
      xminuu=x1
      yminuu=y1
c
c
c
      xmaxuu=x2
      ymaxuu=y2
c
c
c
c     this is used to check to make sure tha the delta
c     user unit limt values are not zero.
c     thus if they are the same the system will incrment
c    the value of the max by one such that the range
c    will never be exactly zero.
c
c
      if (xmaxuu.eq.xminuu) xmaxuu=xmaxuu+1.0
      if (ymaxuu.eq.yminuu) ymaxuu=ymaxuu+1.0
c
c
      call setuu
c
c
c     assign the temperary locate points to the pernament
c     locate values
c
      xmapl1=xmapt1
      xmapl2=xmapt2
      ymapl1=ymapt1
      ymapl2=ymapt2
c
c
      return
      end
