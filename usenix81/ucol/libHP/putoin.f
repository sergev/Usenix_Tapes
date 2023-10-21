      subroutine putoin(x,y)
	include 'gsdata.f'
c
c     used to convert plotter units to inches.
c
      x=((x-xoffpu)*xmaxin)/(xfacts*xmaxpu)
      y=((y-yoffpu)*ymaxin)/(yfacts*ymaxpu)
      return
      end
