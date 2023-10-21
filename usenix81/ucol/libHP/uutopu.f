      subroutine uutopu(x,y)
	include 'gsdata.f'
c
c     convert user units to plotter units.
c
      x=(((x-xminuu)*(xmapl2-xmapl1))/(xmaxuu-xminuu))
     &*xfacts+xmapl1+xoffpu
      y=(((y-yminuu)*(ymapl2-ymapl1))/(ymaxuu-yminuu))
     &*yfacts+ymapl1+yoffpu
      return
      end
