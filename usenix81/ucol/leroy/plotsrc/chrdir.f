      subroutine chrdir(angle)
c
c    This routine is used to set the direction of labelling on the hp
c
      save
      common /pfile/ ipunit
      character*1 k
c
      data  k / 'k' /
c
      call rwrite(ipunit, 1, k)
      call rwrite(ipunit, 4, angle)
      return
      end
c
c
