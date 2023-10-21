      subroutine clear
c
c     routine clear causes the screen to clear.
c
c
      save
      common /pfile/ ipunit
      character*1 e
c
      data  e  / 'e' /
c
      call rwrite(ipunit, 1, e)
      call wait(200)
      return
      end
c
c
