      subroutine movev(x,y,itran)
c
c    routine movev moves the cursor to position x,y and then initiates
c    the graph mode.
c
      save
      common /pfile/ ipunit
      character*1 m
      integer*2 xplt,yplt
      common /obuf/ xplt,yplt
c
      data  m    / 'm' /
c
      xplt = x
      yplt = y
      if (itran .eq. 0)  go to 10
      xplt = 16384. - y
      yplt = x
   10 continue
      call rwrite(ipunit, 1, m)
      call rwrite(ipunit, 4, xplt)
      return
      end
c
c
