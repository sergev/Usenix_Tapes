      subroutine ltype(n)
c
c    routine ltype sets the line type for graphing.  
c
c     n  =  0 - solid
c        =  1 - dotted
c        =  2 - dot-dashed
c        =  3 - short-dashed
c        =  4 - long-dashed
c
      save
      common /pfile/ ipunit
      character*1 f
      character*6 solid
      character*7 dotted
      character*10 ddashd
      character*12 sdashd
      character*11 ldashd
c
      data  f  / 'f' /
      data  solid  / 'solid\n' /
      data  dotted / 'dotted\n' /
      data  ddashd / 'dotdashed\n' /
      data  sdashd / 'shortdashed\n' /
      data  ldashd / 'longdashed\n' /
c
      il = n
      if (n .lt. 0 .or. n .gt. 4)  il = 0
      call rwrite(ipunit, 1, f)
      if (il .eq. 0) call rwrite(ipunit, 6, solid)
      if (il .eq. 1) call rwrite(ipunit, 7, dotted)
      if (il .eq. 2) call rwrite(ipunit, 10, ddashd)
      if (il .eq. 3) call rwrite(ipunit, 12, sdashd)
      if (il .eq. 4) call rwrite(ipunit, 11, ldashd)
      return
      end
c
c
