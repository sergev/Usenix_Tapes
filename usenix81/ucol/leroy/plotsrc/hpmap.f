      subroutine hpmap(xmin,xmax,ymin,ymax)
c
c    routine hpmap maps plot units on the hp
c
      save
      common /pfile/ ipunit
c
      character*1 g
c
      common /obuf5/ x(5)
c
      data  g / 'g' /
c
      x(1) = xmin
      x(2) = xmax
      x(3) = ymin
      x(4) = ymax
      call rwrite(ipunit, 1, g)
      call rwrite(ipunit, 16, x)
c
      return
      end
c
c
