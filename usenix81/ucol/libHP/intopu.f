      subroutine intopu(x,y)
	include 'gsdata.f'
c
c     this is used to convert inches to plotter units.
c
      x=x*xmaxpu/xmaxin*xfacts+xoffpu
      y=y*ymaxpu/ymaxin*yfacts+yoffpu
      return
      end
