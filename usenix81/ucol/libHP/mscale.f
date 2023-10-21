      subroutine mscale (xoff,yoff)
	include 'gsdata.f'
c
c     this is used to set user units to millimeters.
c
      ntemp=minit1
      minit1=2
      call locate(0.0,xmaxpu,0.0,ymaxpu)
      xran=(xmaxmu-xminmu)*.025-xoff
      yran=(ymaxmu-yminmu)*.025-yoff
      call mapuu(-xoff,xran,-yoff,yran)
      minit1=ntemp
      return
      end
