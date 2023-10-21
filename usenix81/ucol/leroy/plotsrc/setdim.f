      subroutine setdim(xdim,ydim,xlow,ylow)
c
c    routine setdim sets the physical dimensions of the plot and must be
c    called prior to a call to setscl.
c
c    inputs  - xdim   = horizontal dimension in inches
c 	       ydim   = vertical dimension in inches
c	       xlow   = horizontal location of lower left hand corner of plot
c			in inches
c	       ylow   = vertical location of lower left hand corner of plot
c			in inches
c
      save
c
      common /pdim/ xxdim,yydim,xxlow,yylow,rxdim,rydim,rxlow,rylow,
     1              xbl,ybl,xbh,ybh,xbm,ybm,itran,tangle,
     2              ca,sa,cellht,cellwd
c
c    xc = standard raster conversion factor in rasters per inch
c
      data  xc / 1638.4 /
c
      xxdim = xdim
      yydim = ydim
      xxlow = xlow
      yylow = ylow
c
      rxdim = xdim*xc
      rydim = ydim*xc
      rxlow = xlow*xc
      rylow = ylow*xc
c
      xbm = 16384.
      ybm = 12288.
      if (itran .ne. 0) xbm = 12288.
      if (itran .ne. 0) ybm = 16384.
      xbl = rxlow
      if (xbl .lt. 0.) xbl = 0.
      ybl = rylow
      if (ybl .lt. 0.) ybl = 0.
      xbh = rxlow + rxdim
      if (xbh .gt. xbm) xbh = xbm
      ybh = rylow + rydim
      if (ybh .gt. ybm) ybh = ybm
c
      return
      end
c
c
