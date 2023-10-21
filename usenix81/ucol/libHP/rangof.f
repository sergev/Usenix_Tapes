      subroutine rangof(x1,x2,y1,y2,tpts)
	include 'gsdata.f'
c
c
c     to output the minimum and maximum user unit values
c     passed to plot since rangon was last called.
c
c
      x1=xminrg
      x2=xmaxrg
      y1=yminrg
      y2=ymaxrg
      tpts=yrange
      yrange=-1.0
      return
      end
