      subroutine setscl(xmin,xmax,ymin,ymax)
c
c    routine setscl sets the scale factors for the plot.  It must be called
c    after the call to setdim
c
c    inputs  - xmin   = value of plot data corresponding to the left hand edge
c			of the plot
c	       xmax   = value of plot data corresponding to the right hand edge
c			of the plot
c	       ymin   = value of plot data corresponding to the bottom edge
c			of the plot
c	       ymax   = value of plot data corresponding to the top edge
c			of the plot
c
      save
c
      common /pdim/ xdim,ydim,xlow,ylow,rxdim,rydim,rxlow,rylow,
     1              xbl,ybl,xbh,ybh,xbm,ybm,itran,tangle,
     2              ca,sa,cellht,cellwd
c
      common /pscl/ xxmin,xxmax,yymin,yymax,xscale,yscale,xrange,yrange
c
      xxmin = xmin
      yymin = ymin
      xxmax = xmax
      yymax = ymax
c
      xrange = xmax - xmin
      if (xrange .eq. 0.)  go to 900
      yrange = ymax - ymin
      if (yrange .eq. 0.)  go to 910
      xscale = rxdim/xrange
      yscale = rydim/yrange
c
      return
c
c    error abort
c
c   900 print *,' setscl: horizontal plot range set to zero - run aborted'
900	continue
      stop
c
c   910 print *,' setscl: vertical plot range set to zero - run aborted'
910	continue
      stop
c
      end
c
c
