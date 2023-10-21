      subroutine putouu(x,y)
	include 'gsdata.f'
c
c    convert plotter units to user units.
c
      x=((x-xoffpu)-xmapl1)*(xmaxuu-xminuu)/(xfacts*(xmapl2-xmapl1))
     &+xminuu
      y=((y-yoffpu)-ymapl1)*(ymaxuu-yminuu)/(yfacts*(ymapl2-ymapl1))
     &+yminuu
      return
      end
