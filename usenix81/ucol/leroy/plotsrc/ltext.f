      subroutine ltext(x,y,angle,string,lst,iclip)
c
c    routine ltext causes text to be printed on the plot
c
c    inputs  - x      = x coordinate in rasters of first letter
c              y      = y coordinate in rasters of first letter
c              angle  = angle of text labelling
c              string = character string for label
c	       lst    = length of string
c              iclip  = clip flag see routine plot1
c
      save
c
      common /pfile/ ipunit
c
      common /pdim/ xdim,ydim,xlow,ylow,rxdim,rydim,rxlow,rylow,
     1              xbl,ybl,xbh,ybh,xbm,ybm,itran,tangle,
     2              ca,sa,cellht,cellwd
c
      character*(*) string
      character*1 t,nl
c
      data  t / 't' /
      data  nl / '\n' /
c
      if (iclip .ne. 0)  go to 10
      if (x .lt. xbl) return
      if (x .gt. xbh) return
      if (y .lt. ybl) return
      if (y .gt. ybh) return
      go to 20
   10 if (x .lt. 0.) return
      if (y .lt. 0.) return
      if (x .gt. xbm) return
      if (y .gt. ybm) return
   20 call movev(x,y,itran)
      an = angle + tangle
      call chrdir(an)
      call rwrite(ipunit, 1, t)
      call rwrite(ipunit, lst, string)
      call rwrite(ipunit, 1, nl)
c
      return
      end
c
c
