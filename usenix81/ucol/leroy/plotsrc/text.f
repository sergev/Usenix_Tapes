      subroutine text(x,y,angle,iref,string,ntimes)
c
c    routine text causes text to be plotted
c
c    inputs  - x      = reference x-coordinate in user units
c              y      = reference y-coordinate in user units
c              angle  = polar orientation of character string about point (x,y)
c			measured in degrees counterclockwise about the x-axis
c              iref   = flag which defines the reference point (x,y) relative to
c			the character string
c			= 0 - (x,y) is at lower left hand corner of string
c			= 1 - (x,y) is at center left hand edge of string
c			= 2 - (x,y) is at top left hand corner of string
c			= 3 - (x,y) is at bottom center of string
c			= 4 - (x,y) is at center of string
c			= 5 - (x,y) is at top center of string
c			= 6 - (x,y) is at lower right hand corner of string
c			= 7 - (x,y) is at center of right hand edge of string
c			= 8 - (x,y) is at upper right hand edge of string
c	       string = character string
c              ntimes = number of times to redraw same string to make darker
c			on hp plotter
c
      save
      character*(*) string
      character*1 q
      integer*2 textrf
c
      common /pdim/ xdim,ydim,xlow,ylow,rxdim,rydim,rxlow,rylow,
     1              xbl,ybl,xbh,ybh,xbm,ybm,itran,tangle,
     2              ca,sa,cellht,cellwd
      common /pfile/ ipunit
c
      data rad / .0174533  /
      data  q / 'q' /
c
      j = len(string)
      do 10  i = 1,j
c
c    the following added to prevent 0 with a slash on the hp
c
      if (string(i:i) .eq. '0') string(i:i) = 'O'
      if (string(i:i) .eq. ' ') go to 10
      if (string(i:i) .eq. '\n')  go to 10
      if (string(i:i) .eq. '\0')  go to 10
      lst = i
   10 continue
      call rwrite(ipunit, 1, q)
      textrf = iref
      call rwrite(ipunit, 2, textrf)
      xx = xmap(x)
      yy = ymap(y)
      do 100  i = 1,ntimes
  100 call ltext(xx,yy,angle,string,lst,1)
c
      return
      end
c
c
