      subroutine putogu(x,y)
	include 'gsdata.f'
c
c    used to convert plotter units to graphic units.
c
      x=((x-xoffpu)/xmaxpu)*xmaxgu/xfacts
      y=((y-yoffpu)/ymaxpu)*ymaxgu/yfacts
      return
      end
