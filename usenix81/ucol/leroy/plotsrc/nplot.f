      subroutine nplot(n,x,y,igraf,iclip,thick,asymb)
c
c    routine nplot will plot a set of points specified by the arrays x
c    and y
c
c    inputs  - n      = number of points to plot
c              x      = plot data horizontal coordinate array
c              y      = plot data vertical coordinate array
c              igraf  = plot flag
c			< 0 - plot only symbols with no lines
c			= 0 - plot only lines with no symbols
c			> 0 - plot both lines and symbols
c	       iclip  = clipping flag see routine plot1
c              thick  = thickness of line in inches on the versatek
c              asymb  = plotting symbol.  note:  if the number of points,
c			n, is negative then it as assumed that asymb
c			is an array of dimension n
c
      save 
c
      common /pdim/ xdim,ydim,xlow,ylow,rxdim,rydim,rxlow,rylow,
     1              xbl,ybl,xbh,ybh,xbm,ybm,itran,tangle,
     2              ca,sa,cellht,cellwd
c
      common /pscl/ xmin,xmax,ymin,ymax,xscale,yscale,xrange,yrange
c
      common /pfile/ ipunit
c
      character*1 asymb(1)
      dimension x(1),y(1)
c
c    xc = standard raster conversion factor in rasters per inch
c
      data  xc / 1638.4 /
c
c    rmin = minimum spacing in standard rasters for line thickening
c
      data  rmin  / 10. /
c
c    xchoff,ychoff = offsets in standard rasters for symbol plotting
c
      data  xchoff  /  16. /
      data  ychoff  /  24. /
c
      nn = iabs(n)
c
c    line plot
c
      if (igraf .lt. 0)  go to 200
      rthick = thick*xc
      ilines = rthick/rmin
      if (ilines .eq. 0) ilines = 1
      rthick = (ilines-1)*rmin/2.
      ilabs = ilines
      if (ilines .lt. 0) ilabs = -thick
c
      do 100  i = 1,ilabs
      roff = rmin*(i-1) - rthick
      if (ilines .lt. 0) roff = 0.
  100 call plot1(nn,x,y,roff,iclip)
c
c    symbol plot
c
  200 if (igraf .eq. 0)  go to 900
      do 300  i = 1,nn
      j = 1
      if (n .lt. 0) j = i
      if (asymb(j) .eq. ' ')  go to 300
      xx = xmap(x(i)) - xchoff
      yy = ymap(y(i)) - ychoff
      call ltext(xx,yy,0.,asymb(j),1,iclip)
  300 continue
c
  900 return
      end
c
c
