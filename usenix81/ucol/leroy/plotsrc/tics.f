      subroutine tics(x1,y1,x2,y2,stic,ntic,dtic,tlen,thick,idir)
c
c    routine tics draws tic marks
c
c    inputs  - x1     = x coordinate of starting point of axis in user units
c              y1     = y coordinate of starting point af axis in user units
c              x2     = x coordinate of ending point af axis in user units
c              y2     = y coordinate of ending point af axis in user units
c              stic   = position along axis from (x1,y1) for first tic mark
c			in user units
c              ntic   = number of tic marks
c              dtic   = increment along axis for tic marks in user units
c              tlen   = length of tic marks IN INCHES
c              thick  = thickness of tic marks IN INCHES
c              idir   = flag which defines orientation of tic marks
c			(directions given for (x2,y2) directly to the
c			right of (x1,y1))
c			> 0 - tic marks point up
c			= 0 - tic marks point on both sides
c			< 0 - tic marks point down
c
      save
c
      common /pdim/ xdim,ydim,xlow,ylow,rxdim,rydim,rxlow,rylow,
     1              xbl,ybl,xbh,ybh,xbm,ybm,itran,tangle,
     2              ca,sa,cellht,cellwd
c
      common /pscl/ xmin,xmax,ymin,ymax,xscale,yscale,xrange,yrange
c
      dimension x(3),y(3)
c
c    xc = standard raster conversion factor in rasters per inch
c
      data  xc / 1638.4 /
c
      xr1 = xmap(x1)
      yr1 = ymap(y1)
      xr2 = xmap(x2)
      yr2 = ymap(y2)
      dx = x2 - x1
      dy = y2 - y1
      axis = sqrt(dx**2 + dy**2)
      rdx = xr2 - xr1
      rdy = yr2 - yr1
      rrdx = rdx/axis
      rrdy = rdy/axis
      raxis = sqrt(rdx**2 + rdy**2)
      rxdtic = xc*tlen*(-rdy)/raxis
      rydtic = xc*tlen*rdx/raxis
      if (idir .lt. 0) rxdtic = -rxdtic
      if (idir .lt. 0) rydtic = -rydtic
c
      do 100 i = 1,ntic
      rxtic = (stic + dtic*(i-1))*rrdx + xr1
      rytic = (stic + dtic*(i-1))*rrdy + yr1
      x(1) = rxtic + rxdtic
      y(1) = rytic + rydtic
      x(2) = rxtic
      y(2) = rytic
      x(3) = rxtic - rxdtic
      y(3) = rytic - rydtic
      jplot = 2
      if (idir .eq. 0) jplot = 3
      do 110  j = 1,3
      x(j) = (x(j) - rxlow)/xscale + xmin
  110 y(j) = (y(j) - rylow)/yscale + ymin
  100 call nplot(jplot,x,y,0,1,thick,' ')
c
      return
      end
