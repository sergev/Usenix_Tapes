      subroutine mutopu(x,y)
	include 'gsdata.f'
c
c    used to convert machine units to plotter units.
c
      x=(x-xminmu)*xmaxpu/(xmaxmu-xminmu)
      y=(y-yminmu)*ymaxpu/(ymaxmu-yminmu)
      return
      end
