c    The following function routine maps from plot data units to
c    standard raster units (16384 rasters per 10 inches)
c
      function xmap(x)
c
c    horizontal plot map
c
      common /pdim/ xdim,ydim,xlow,ylow,rxdim,rydim,rxlow,rylow,
     1              xbl,ybl,xbh,ybh,xbm,ybm,itran,tangle,
     2              ca,sa,cellht,cellwd
c
      common /pscl/ xmin,xmax,ymin,ymax,xscale,yscale,xrange,yrange
c
      xmap = rxlow + (x-xmin)*xscale
c
      return
      end
c
      function ymap(y)
c
c    vertical plot map
c
      common /pdim/ xdim,ydim,xlow,ylow,rxdim,rydim,rxlow,rylow,
     1              xbl,ybl,xbh,ybh,xbm,ybm,itran,tangle,
     2              ca,sa,cellht,cellwd
c
      common /pscl/ xmin,xmax,ymin,ymax,xscale,yscale,xrange,yrange
c
      ymap = rylow + (y-ymin)*yscale
c
      return
      end
c
c
