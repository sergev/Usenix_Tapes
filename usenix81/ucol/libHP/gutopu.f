      subroutine gutopu(x,y)
	include 'gsdata.f'
c
c      this is used to convert graphic units to plotter units
c
      x=x*xmaxpu/xmaxgu*xfacts+xoffpu
      y=y*ymaxpu/ymaxgu*yfacts+yoffpu
      return
      end
