      subroutine xlimit(xmin,xmax,ymin,ymax,xmarg)
c
c    This routine is use for setting the limits on the hp plotter
c
      save
      common /pfile/ ipunit
      common /obuf5/ x(5)
c
      character*1 b
c
      data  b / 'b' /
c
      x(1) = xmin
      x(2) = xmax
      x(3) = ymin
      x(4) = ymax
      x(5) = xmarg
      call rwrite(ipunit, 1, b)
      call rwrite(ipunit, 20, x)
      call hpmap(0.,16384.,0.,12288.)
      return
      end
c
c
