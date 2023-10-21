      subroutine wait(n)
c
c    routine wait causes n off's to be sent to the terminal
c
      save
      common /pfile/ ipunit
      character*1 m
      integer*2 xplt,yplt
      common /obuf/ xplt,yplt
c
      data  m  / 'm' /
c
      xplt = 0
      yplt = 0
      do 100  i = 1,n
      call rwrite(ipunit, 1, m)
  100 call rwrite(ipunit, 4, xplt)
      return
      end
c
c
