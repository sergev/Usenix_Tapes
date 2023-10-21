      subroutine putomu(x,y)
	include 'gsdata.f'
c
c   convert plotter units to machine units.
c
      x=x/xmaxpu*(xmaxmu-xminmu)+xminmu
      y=y/ymaxpu*(ymaxmu-yminmu)+yminmu
      return
      end
