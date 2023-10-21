      subroutine pspeed(ispeed)
c
c    This routine is used to set the hp pen speed
c
      save
      common /pfile/ ipunit
      integer*2 ispd
      character*1 o
c
      data  o / 'o' /
c
      ispd = ispeed
      call rwrite(ipunit, 1, o)
      call rwrite(ipunit, 2, ispd)
      return
      end
c
c
