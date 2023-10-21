      subroutine drawv(x,y,itran)
c
c    routine drawv will draw a straight line from the existing position
c    to position x,y.
c
      save
      common /pfile/ ipunit
      character*1 n
      integer*2 xplt,yplt
      common /obuf/ xplt,yplt
c
      data  n / 'n' /
c
      xplt = x
      yplt = y
      if (itran .eq. 0)  go to 10
      xplt = 16384. - y
      yplt = x
   10 continue
      call rwrite(ipunit, 1, n)
      call rwrite(ipunit, 4, xplt)
      return
      end
c
c
