      subroutine selpen(ipen)
c
c    This routine is used to select a new pen on the hp
c
      save
      common /pfile/ ipunit
      integer*2 ipn
      character*1 h
c
      data  h / 'h' /
c
      ipn = ipen
      call rwrite(ipunit,1,h)
      call rwrite(ipunit, 2, ipn)
      return
      end
c
c
