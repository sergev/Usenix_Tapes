      subroutine chrsiz(height,ratio,slant)
c
c    This routine is used to set character size and slant on the hp
c
      save
      common /pfile/ ipunit
      common /pdim/ xdim,ydim,xlow,ylow,rxdim,rydim,rxlow,rylow,
     1              xbl,ybl,xbh,ybh,xbm,ybm,itran,tangle,
     2              ca,sa,cellht,cellwd
c
      common /pscl/ xmin,xmax,ymin,ymax,xscale,yscale,xrange,yrange
c
      common /obuf5/ x(5)
      character*1 j
c
      data  j / 'j' /
	data xc /1638.4/
c
      x(1) = height*xc
      x(2) = ratio
      x(3) = slant
      cellht = x(1)
      cellwd = x(2)*x(1)*1.5
      call rwrite(ipunit, 1, j)
      call rwrite(ipunit, 12, x)
      return
      end
c
c
